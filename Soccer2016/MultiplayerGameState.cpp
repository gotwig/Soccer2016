#include <hppFiles/MultiplayerGameState.hpp>
#include <hppFiles/MusicPlayer.hpp>
#include <hppFiles/Foreach.hpp>
#include <hppFiles/Utility.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>


sf::IpAddress getAddressFromFile()
{
	{ // Try to open existing file (RAII block)
		std::ifstream inputFile("ip.txt");
		std::string ipAddress;
		if (inputFile >> ipAddress)
			return ipAddress;
	}

	// If open/read failed, create new file
	std::ofstream outputFile("ip.txt");
	std::string localAddress = "127.0.0.1";
	outputFile << localAddress;
	return localAddress;
}

MultiplayerGameState::MultiplayerGameState(StateStack& stack, Context context, bool isHost)
	: State(stack, context)
	, mWorld(*context.window, *context.fonts, *context.sounds, true)
	, mWindow(*context.window)
	, mTextureHolder(*context.textures)
	, mConnected(false)
	, mGameServer(nullptr)
	, mActiveState(true)
	, mHasFocus(true)
	, mHost(isHost)
	, mGameStarted(false)
	, mClientTimeout(sf::seconds(2.f))
	, mTimeSinceLastPacket(sf::seconds(0.f))
{
	mBroadcastText.setFont(context.fonts->get(Fonts::Main));
	mBroadcastText.setPosition(1024.f / 2, 100.f);

	mPlayerInvitationText.setFont(context.fonts->get(Fonts::Main));
	mPlayerInvitationText.setCharacterSize(20);
	mPlayerInvitationText.setColor(sf::Color::White);
	mPlayerInvitationText.setString("Press Enter to spawn player 2");
	mPlayerInvitationText.setPosition(1000 - mPlayerInvitationText.getLocalBounds().width, 760 - mPlayerInvitationText.getLocalBounds().height);

	// We reuse this text for "Attempt to connect" and "Failed to connect" messages
	mFailedConnectionText.setFont(context.fonts->get(Fonts::Main));
	mFailedConnectionText.setString("Attempting to connect...");
	mFailedConnectionText.setCharacterSize(35);
	mFailedConnectionText.setColor(sf::Color::White);
	//centerOrigin(mFailedConnectionText);
	mFailedConnectionText.setPosition(mWindow.getSize().x / 2.f, mWindow.getSize().y / 2.f);

	// Render a "establishing connection" frame for user feedback
	mWindow.clear(sf::Color::Black);
	mWindow.draw(mFailedConnectionText);
	mWindow.display();
	mFailedConnectionText.setString("Could not connect to the remote server!");
	centerOrigin(mFailedConnectionText);

	sf::IpAddress ip;
	if (isHost)
	{
		mWorld.isHost = true;
		mGameServer.reset(new GameServer(sf::Vector2f(mWindow.getSize())));
		ip = "127.0.0.1";
	}
	else
	{
		mWorld.isHost = false;
		ip = getAddressFromFile();
	}

	if (mSocket.connect(ip, ServerPort, sf::seconds(5.f)) == sf::TcpSocket::Done)
		mConnected = true;
	else
		mFailedConnectionClock.restart();

	mSocket.setBlocking(false);

	// Play game theme
	context.music->play(Music::MissionTheme);
}

void MultiplayerGameState::draw()
{
	if (mConnected)
	{
		mWorld.draw();

		// Broadcast messages in default view
		mWindow.setView(mWindow.getDefaultView());

		if (!mBroadcasts.empty())
			mWindow.draw(mBroadcastText);

		if (mLocalPlayerIdentifiers.size() < 2 && mPlayerInvitationTime < sf::seconds(0.5f))
			mWindow.draw(mPlayerInvitationText);
	}
	else
	{
		mWindow.draw(mFailedConnectionText);
	}
}

void MultiplayerGameState::onActivate()
{
	mActiveState = true;
}

void MultiplayerGameState::onDestroy()
{
	if (!mHost && mConnected)
	{
		// Inform server this client is dying
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::Quit);
		mSocket.send(packet);
	}
}

bool MultiplayerGameState::update(sf::Time dt)
{
	// Connected to server: Handle all the network logic
	if (mConnected)
	{
		mWorld.update(dt);

		// Remove players whose soccerpawns were destroyed
		bool foundLocalPlayer = false;
		for (auto itr = mPlayers.begin(); itr != mPlayers.end(); )
		{
			// Check if there are no more local players for remote clients
			if (std::find(mLocalPlayerIdentifiers.begin(), mLocalPlayerIdentifiers.end(), itr->first) != mLocalPlayerIdentifiers.end())
			{
				foundLocalPlayer = true;
			}

			if (!mWorld.getPlayerPawn(itr->first))
			{
				itr = mPlayers.erase(itr);

				// No more players left: Mission failed
				if (mPlayers.empty())
					requestStackPush(States::GameOver);
			}
			else
			{
				++itr;
			}
		}

		if (!foundLocalPlayer && mGameStarted)
		{
			requestStackPush(States::GameOver);
		}

		// Only handle the realtime input if the window has focus and the game is unpaused
		if (mActiveState && mHasFocus)
		{
			CommandQueue& commands = mWorld.getCommandQueue();
			FOREACH(auto& pair, mPlayers)
				pair.second->handleRealtimeInput(commands);
		}

		// Always handle the network input
		CommandQueue& commands = mWorld.getCommandQueue();
		FOREACH(auto& pair, mPlayers)
			pair.second->handleRealtimeNetworkInput(commands);

		// Handle messages from server that may have arrived
		sf::Packet packet;
		if (mSocket.receive(packet) == sf::Socket::Done)
		{
			mTimeSinceLastPacket = sf::seconds(0.f);
			sf::Int32 packetType;
			packet >> packetType;
			handlePacket(packetType, packet);
		}
		else
		{
			// Check for timeout with the server
			if (mTimeSinceLastPacket > mClientTimeout)
			{
				mConnected = false;

				mFailedConnectionText.setString("Lost connection to server");
				centerOrigin(mFailedConnectionText);

				mFailedConnectionClock.restart();
			}
		}

		updateBroadcastMessage(dt);

		// Time counter for blinking 2nd player text
		mPlayerInvitationTime += dt;
		if (mPlayerInvitationTime > sf::seconds(1.f))
			mPlayerInvitationTime = sf::Time::Zero;

		// Events occurring in the game
		GameActions::Action gameAction;
		while (mWorld.pollGameAction(gameAction))
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::GameEvent);
			packet << static_cast<sf::Int32>(gameAction.type);
			packet << gameAction.position.x;
			packet << gameAction.position.y;

			mSocket.send(packet);
		}

		// Regular position updates
		// mTickClock.getElapsedTime() > sf::seconds(1.f / 20.f) 
		if (true)
		{
			sf::Packet positionUpdatePacket;
			positionUpdatePacket << static_cast<sf::Int32>(Client::PositionUpdate);
			positionUpdatePacket << static_cast<sf::Int32>(mLocalPlayerIdentifiers.size());

			FOREACH(sf::Int32 identifier, mLocalPlayerIdentifiers)
			{
				if (SoccerPawn* soccerpawn = mWorld.getPlayerPawn(identifier))
					positionUpdatePacket << identifier << soccerpawn->getPosition().x << soccerpawn->getPosition().y << mWorld.soccerBallG->getPosition().x << mWorld.soccerBallG->getPosition().y;
			}

			mSocket.send(positionUpdatePacket);
			//mTickClock.restart();
		}

		mTimeSinceLastPacket += dt;
	}

	// Failed to connect and waited for more than 5 seconds: Back to menu
	else if (mFailedConnectionClock.getElapsedTime() >= sf::seconds(5.f))
	{
		requestStateClear();
		requestStackPush(States::Menu);
	}

	return true;
}

void MultiplayerGameState::disableAllRealtimeActions()
{
	mActiveState = false;

	FOREACH(sf::Int32 identifier, mLocalPlayerIdentifiers)
		mPlayers[identifier]->disableAllRealtimeActions();
}

bool MultiplayerGameState::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();

	// Forward event to all players
	FOREACH(auto& pair, mPlayers)
		pair.second->handleEvent(event, commands);

	if (event.type == sf::Event::KeyPressed)
	{
		// Enter pressed, add second player co-op (only if we are one player)
		if (event.key.code == sf::Keyboard::Return && mLocalPlayerIdentifiers.size() == 1)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::RequestCoopPartner);

			mSocket.send(packet);
		}

		// Escape pressed, trigger the pause screen
		else if (event.key.code == sf::Keyboard::Escape)
		{
			disableAllRealtimeActions();
			requestStackPush(States::NetworkPause);
		}
	}
	else if (event.type == sf::Event::GainedFocus)
	{
		mHasFocus = true;
	}
	else if (event.type == sf::Event::LostFocus)
	{
		mHasFocus = false;
	}

	return true;
}

void MultiplayerGameState::updateBroadcastMessage(sf::Time elapsedTime)
{
	if (mBroadcasts.empty())
		return;

	// Update broadcast timer
	mBroadcastElapsedTime += elapsedTime;
	if (mBroadcastElapsedTime > sf::seconds(2.5f))
	{
		// If message has expired, remove it
		mBroadcasts.erase(mBroadcasts.begin());

		// Continue to display next broadcast message
		if (!mBroadcasts.empty())
		{
			mBroadcastText.setString(mBroadcasts.front());
			centerOrigin(mBroadcastText);
			mBroadcastElapsedTime = sf::Time::Zero;
		}
	}
}

void MultiplayerGameState::handlePacket(sf::Int32 packetType, sf::Packet& packet)
{
	switch (packetType)
	{
		// Send message to all clients
	case Server::BroadcastMessage:
	{
		std::string message;
		packet >> message;
		mBroadcasts.push_back(message);

		// Just added first message, display immediately
		if (mBroadcasts.size() == 1)
		{
			mBroadcastText.setString(mBroadcasts.front());
			centerOrigin(mBroadcastText);
			mBroadcastElapsedTime = sf::Time::Zero;
		}
	} break;

	// Sent by the server to order to spawn player 1 player on connect
	case Server::SpawnSelf:
	{
		sf::Int32 soccerpawnIdentifier;
		sf::Vector2f soccerpawnPosition;
		packet >> soccerpawnIdentifier >> soccerpawnPosition.x >> soccerpawnPosition.y;

		SoccerPawn* soccerpawn = mWorld.addPlayerPawn(soccerpawnIdentifier);
		//soccerpawn->setPosition(soccerpawnPosition);
		//printf("%f", soccerpawnPosition.x); // RIGHT HERE WAS THE ERROR
		mPlayers[soccerpawnIdentifier].reset(new Player(&mSocket, soccerpawnIdentifier, getContext().keys1));
		mLocalPlayerIdentifiers.push_back(soccerpawnIdentifier);
		
		mWorld.spawnCenterBall(mHost);

		mGameStarted = true;
	} break;

	// 
	case Server::PlayerConnect:
	{
		sf::Int32 soccerpawnIdentifier;
		sf::Vector2f soccerpawnPosition;
		packet >> soccerpawnIdentifier >> soccerpawnPosition.x >> soccerpawnPosition.y;

		SoccerPawn* soccerpawn = mWorld.addPlayerPawn(soccerpawnIdentifier);
		//soccerpawn->setPosition(soccerpawnPosition);
		mPlayers[soccerpawnIdentifier].reset(new Player(&mSocket, soccerpawnIdentifier, nullptr));
	} break;

	// 
	case Server::PlayerDisconnect:
	{
		sf::Int32 soccerpawnIdentifier;
		packet >> soccerpawnIdentifier;

		//mWorld.remove(soccerpawnIdentifier);
		mPlayers.erase(soccerpawnIdentifier);
	} break;

	// 
	case Server::InitialState:
	{
		sf::Int32 soccerpawnCount;
		float worldHeight, currentScroll;
		packet >> worldHeight >> currentScroll;

		mWorld.setWorldHeight(worldHeight);
		mWorld.setCurrentBattleFieldPosition(currentScroll);

		packet >> soccerpawnCount;
		for (sf::Int32 i = 0; i < soccerpawnCount; ++i)
		{
			sf::Int32 soccerpawnIdentifier;
			sf::Int32 hitpoints;
			sf::Vector2f soccerpawnPosition;
			packet >> soccerpawnIdentifier >> soccerpawnPosition.x >> soccerpawnPosition.y >> hitpoints;

			SoccerPawn* soccerpawn = mWorld.addPlayerPawn(soccerpawnIdentifier);
			//soccerpawn->setPosition(soccerpawnPosition);
			printf("%f", soccerpawnPosition.x);
			soccerpawn->setHitpoints(hitpoints);


			mPlayers[soccerpawnIdentifier].reset(new Player(&mSocket, soccerpawnIdentifier, nullptr));
		}
	} break;

	//
	case Server::AcceptCoopPartner:
	{
		sf::Int32 soccerpawnIdentifier;
		packet >> soccerpawnIdentifier;

		mWorld.addPlayerPawn(soccerpawnIdentifier);
		mPlayers[soccerpawnIdentifier].reset(new Player(&mSocket, soccerpawnIdentifier, getContext().keys2));
		mLocalPlayerIdentifiers.push_back(soccerpawnIdentifier);
	} break;

	// Player event occurs
	case Server::PlayerEvent:
	{
		sf::Int32 soccerpawnIdentifier;
		sf::Int32 action;
		packet >> soccerpawnIdentifier >> action;

		auto itr = mPlayers.find(soccerpawnIdentifier);
		if (itr != mPlayers.end())
			itr->second->handleNetworkEvent(static_cast<Player::Action>(action), mWorld.getCommandQueue());
	} break;

	// Player's movement or fire keyboard state changes
	case Server::PlayerRealtimeChange:
	{
		sf::Int32 soccerpawnIdentifier;
		sf::Int32 action;
		bool actionEnabled;
		packet >> soccerpawnIdentifier >> action >> actionEnabled;

		auto itr = mPlayers.find(soccerpawnIdentifier);
		if (itr != mPlayers.end())
			itr->second->handleNetworkRealtimeChange(static_cast<Player::Action>(action), actionEnabled);
	} break;

	// Mission successfully completed
	case Server::MissionSuccess:
	{
		requestStackPush(States::MissionSuccess);
	} break;




	//
	case Server::UpdateClientState:
	{
		sf::Vector2f soccerBallPosition;
		sf::Vector2f soccerBallVelocity;

		sf::Int32 soccerpawnCount;
		packet >> soccerBallPosition.x >> soccerBallPosition.y >> soccerBallVelocity.x >> soccerBallVelocity.y >> soccerpawnCount;

		float currentViewPosition = mWorld.getViewBounds().top + mWorld.getViewBounds().height;

		// Set the world's scroll compensation according to whether the view is behind or too advanced
		//mWorld.setWorldScrollCompensation(currentViewPosition / currentWorldPosition);
		//mWorld.soccerBallG

		if (!mHost) {
			sf::Vector2f interpolatedPosition2 = mWorld.soccerBallG->getPosition() + (soccerBallPosition - mWorld.soccerBallG->getPosition()) * 0.4f;
			mWorld.soccerBallG->setPosition(interpolatedPosition2);

			sf::Vector2f interpolatedVelocity = mWorld.soccerBallG->getVelocity() + (soccerBallVelocity - mWorld.soccerBallG->getVelocity()) * 0.1f;
			mWorld.soccerBallG->setVelocity(soccerBallVelocity);
		}

		for (sf::Int32 i = 0; i < soccerpawnCount; ++i)
		{
			sf::Vector2f soccerpawnPosition;
			sf::Int32 soccerpawnIdentifier;
			packet >> soccerpawnIdentifier >> soccerpawnPosition.x >> soccerpawnPosition.y;

			SoccerPawn* soccerpawn = mWorld.getPlayerPawn(soccerpawnIdentifier);
			
			bool isLocalPlayer = std::find(mLocalPlayerIdentifiers.begin(), mLocalPlayerIdentifiers.end(), soccerpawnIdentifier) != mLocalPlayerIdentifiers.end();

			if (soccerpawn && !isLocalPlayer)
			{
				sf::Vector2f interpolatedPosition = soccerpawn->getPosition() + (soccerpawnPosition - soccerpawn->getPosition()) * 0.1f;
				soccerpawn->setPosition(interpolatedPosition);

				//printf("%f", interpolatedPosition.x);
			}
		}


	} break;
	}
}