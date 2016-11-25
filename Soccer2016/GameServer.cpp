#include <hppFiles/GameServer.hpp>
#include <hppFiles/NetworkProtocol.hpp>
#include <hppFiles/Foreach.hpp>
#include <hppFiles/Utility.hpp>

#include <hppFiles/SoccerPawn.hpp>

#include <SFML/Network/Packet.hpp>

GameServer::RemotePeer::RemotePeer() 
: ready(false)
, timedOut(false)
{
	socket.setBlocking(false);
}

GameServer::GameServer(sf::Vector2f soccerfieldSize)
: mThread(&GameServer::executionThread, this)
, mListeningState(false)
, mClientTimeoutTime(sf::seconds(3.f))
, mMaxConnectedPlayers(10)
, mConnectedPlayers(0)
, mWorldHeight(5000.f)
, mSoccerFieldRect(0.f, mWorldHeight - soccerfieldSize.y, soccerfieldSize.x, soccerfieldSize.y)
, mPlayerPawnCount(0)
, mPeers(1)
, mPlayerPawnIdentifierCounter(1)
, mWaitingThreadEnd(false)
, mLastSpawnTime(sf::Time::Zero)
, mTimeForNextSpawn(sf::seconds(5.f))
{
	mListenerSocket.setBlocking(false);
	mPeers[0].reset(new RemotePeer());
	mThread.launch();
}

GameServer::~GameServer()
{
	mWaitingThreadEnd = true;
	mThread.wait();
}

void GameServer::notifyPlayerRealtimeChange(sf::Int32 PlayerPawnIdentifier, sf::Int32 action, bool actionEnabled)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PlayerRealtimeChange);
			packet << PlayerPawnIdentifier;
			packet << action;
			packet << actionEnabled;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyPlayerEvent(sf::Int32 PlayerPawnIdentifier, sf::Int32 action)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PlayerEvent);
			packet << PlayerPawnIdentifier;
			packet << action;

			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::notifyPlayerSpawn(sf::Int32 PlayerPawnIdentifier)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PlayerConnect);
			packet << PlayerPawnIdentifier << mPlayerPawnInfo[PlayerPawnIdentifier].position.x << mPlayerPawnInfo[PlayerPawnIdentifier].position.y;
			mPeers[i]->socket.send(packet);
		}
	}
}

void GameServer::setListening(bool enable)
{
	// Check if it isn't already listening
	if (enable)
	{	
		if (!mListeningState)
			mListeningState = (mListenerSocket.listen(ServerPort) == sf::TcpListener::Done);
	}
	else
	{
		mListenerSocket.close();
		mListeningState = false;
	}
}

void GameServer::executionThread()
{
	setListening(true);

	sf::Time stepInterval = sf::seconds(1.f / 60.f);
	sf::Time stepTime = sf::Time::Zero;
	sf::Time tickInterval = sf::seconds(1.f / 20.f);
	sf::Time tickTime = sf::Time::Zero;
	sf::Clock stepClock, tickClock;

	while (!mWaitingThreadEnd)
	{	
		handleIncomingPackets();
		handleIncomingConnections();

		stepTime += stepClock.getElapsedTime();
		stepClock.restart();

		tickTime += tickClock.getElapsedTime();
		tickClock.restart();


		// Fixed tick step
		while (tickTime >= tickInterval)
		{
			tick();
			tickTime -= tickInterval;
		}

		// Sleep to prevent server from consuming 100% CPU
		sf::sleep(sf::milliseconds(100));
	}	
}

void GameServer::tick()
{
	updateClientState();

	// Check for round win = all soccers with position.y < offset
	bool allPlayerPawnsDone = true;
	FOREACH(auto pair, mPlayerPawnInfo)
	{
		// As long as one player has not crossed the finish line yet, set variable to false
		if (pair.second.position.y > 0.f)
			allPlayerPawnsDone = false;
	}
	if (allPlayerPawnsDone)
	{
		sf::Packet missionSuccessPacket;
		missionSuccessPacket << static_cast<sf::Int32>(Server::MissionSuccess);
		sendToAll(missionSuccessPacket);
	}

	// Remove IDs of PlayerPawn that have been destroyed (relevant if a client has two, and loses one)
	for (auto itr = mPlayerPawnInfo.begin(); itr != mPlayerPawnInfo.end(); )
	{
		if (itr->second.hitpoints <= 0)
			mPlayerPawnInfo.erase(itr++);
		else
			++itr;
	}
}

sf::Time GameServer::now() const
{
	return mClock.getElapsedTime();
}

void GameServer::handleIncomingPackets()
{
	bool detectedTimeout = false;
	
	FOREACH(PeerPtr& peer, mPeers)
	{
		if (peer->ready)
		{
			sf::Packet packet;
			while (peer->socket.receive(packet) == sf::Socket::Done)
			{
				// Interpret packet and react to it
				handleIncomingPacket(packet, *peer, detectedTimeout);

				// Packet was indeed received, update the ping timer
				peer->lastPacketTime = now();
				packet.clear();
			}

			if (now() >= peer->lastPacketTime + mClientTimeoutTime)
			{
				peer->timedOut = true;
				detectedTimeout = true;
			}
		}
	}

	if (detectedTimeout)
		handleDisconnections();
}

void GameServer::handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout)
{
	sf::Int32 packetType;
	packet >> packetType;

	switch (packetType)
	{
		case Client::Quit:
		{
			receivingPeer.timedOut = true;
			detectedTimeout = true;
		} break;

		case Client::PlayerEvent:
		{
			sf::Int32 PlayerPawnIdentifier;
			sf::Int32 action;
			packet >> PlayerPawnIdentifier >> action;

			notifyPlayerEvent(PlayerPawnIdentifier, action);
		} break;

		case Client::PlayerRealtimeChange:
		{
			sf::Int32 PlayerPawnIdentifier;
			sf::Int32 action;
			bool actionEnabled;
			packet >> PlayerPawnIdentifier >> action >> actionEnabled;
			mPlayerPawnInfo[PlayerPawnIdentifier].realtimeActions[action] = actionEnabled;
			notifyPlayerRealtimeChange(PlayerPawnIdentifier, action, actionEnabled);
		} break;

		case Client::RequestCoopPartner:
		{
			receivingPeer.PlayerPawnIdentifiers.push_back(mPlayerPawnIdentifierCounter);
			mPlayerPawnInfo[mPlayerPawnIdentifierCounter].position = sf::Vector2f(mSoccerFieldRect.width / 2, mSoccerFieldRect.top + mSoccerFieldRect.height / 2);
			mPlayerPawnInfo[mPlayerPawnIdentifierCounter].hitpoints = 100;

			sf::Packet requestPacket;
			requestPacket << static_cast<sf::Int32>(Server::AcceptCoopPartner);
			requestPacket << mPlayerPawnIdentifierCounter;
			requestPacket << mPlayerPawnInfo[mPlayerPawnIdentifierCounter].position.x;
			requestPacket << mPlayerPawnInfo[mPlayerPawnIdentifierCounter].position.y;

			receivingPeer.socket.send(requestPacket);
			mPlayerPawnCount++;

			// Inform every other peer 
			FOREACH(PeerPtr& peer, mPeers)
			{
				if (peer.get() != &receivingPeer && peer->ready)
				{
					sf::Packet notifyPacket;
					notifyPacket << static_cast<sf::Int32>(Server::PlayerConnect);
					notifyPacket << mPlayerPawnIdentifierCounter;
					notifyPacket << mPlayerPawnInfo[mPlayerPawnIdentifierCounter].position.x;
					notifyPacket << mPlayerPawnInfo[mPlayerPawnIdentifierCounter].position.y;
					peer->socket.send(notifyPacket);
				}
			}
			mPlayerPawnIdentifierCounter++;
		} break;

		case Client::PositionUpdate:
		{
			sf::Int32 numPlayerPawns;
			packet >> numPlayerPawns;

			for (sf::Int32 i = 0; i < numPlayerPawns; ++i)
			{
				sf::Int32 PlayerPawnIdentifier;
				sf::Vector2f PlayerPawnPosition;
				sf::Vector2f SoccerBallPosition;
				sf::Vector2f SoccerBallVelocity;
				packet >> PlayerPawnIdentifier >> PlayerPawnPosition.x >> PlayerPawnPosition.y >> SoccerBallPosition.x >> SoccerBallPosition.y << SoccerBallVelocity.x >> SoccerBallVelocity.y;
				mPlayerPawnInfo[PlayerPawnIdentifier].position = PlayerPawnPosition;
				
				if (PlayerPawnIdentifier == 1) {
					mSoccerBallPosition = SoccerBallPosition;
					mSoccerBallVelocity = SoccerBallVelocity;
				}


			}
		} break;

		case Client::GameEvent:
		{
			sf::Int32 action;
			float x;
			float y;

			packet >> action;
			packet >> x;
			packet >> y;
		}
	}
}

void GameServer::updateClientState()
{
	sf::Packet updateClientStatePacket;
	updateClientStatePacket << static_cast<sf::Int32>(Server::UpdateClientState);
	updateClientStatePacket << mSoccerBallPosition.x << mSoccerBallPosition.y << mSoccerBallVelocity.x << mSoccerBallVelocity.y;
	updateClientStatePacket << static_cast<sf::Int32>(mPlayerPawnInfo.size());

	FOREACH(auto PlayerPawn, mPlayerPawnInfo)
		updateClientStatePacket << PlayerPawn.first << PlayerPawn.second.position.x << PlayerPawn.second.position.y;

	sendToAll(updateClientStatePacket);
}

void GameServer::handleIncomingConnections()
{
	if (!mListeningState)
		return;

	if (mListenerSocket.accept(mPeers[mConnectedPlayers]->socket) == sf::TcpListener::Done)
	{
		// order the new client to spawn its own soccer pawn ( player 1 )
		mPlayerPawnInfo[mPlayerPawnIdentifierCounter].position = sf::Vector2f(mSoccerFieldRect.width / 2, mSoccerFieldRect.top + mSoccerFieldRect.height / 2);
		mPlayerPawnInfo[mPlayerPawnIdentifierCounter].hitpoints = 100;

		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::SpawnSelf);
		packet << mPlayerPawnIdentifierCounter;
		packet << mPlayerPawnInfo[mPlayerPawnIdentifierCounter].position.x;
		packet << mPlayerPawnInfo[mPlayerPawnIdentifierCounter].position.y;
		
		mPeers[mConnectedPlayers]->PlayerPawnIdentifiers.push_back(mPlayerPawnIdentifierCounter);
		
		broadcastMessage("New player!");
		informWorldState(mPeers[mConnectedPlayers]->socket);
		notifyPlayerSpawn(mPlayerPawnIdentifierCounter++);

		mPeers[mConnectedPlayers]->socket.send(packet);
		mPeers[mConnectedPlayers]->ready = true;
		mPeers[mConnectedPlayers]->lastPacketTime = now(); // prevent initial timeouts
		mPlayerPawnCount++;
		mConnectedPlayers++;

		if (mConnectedPlayers >= mMaxConnectedPlayers)
			setListening(false);
		else // Add a new waiting peer
			mPeers.push_back(PeerPtr(new RemotePeer()));
	}
}

void GameServer::handleDisconnections()
{
	for (auto itr = mPeers.begin(); itr != mPeers.end(); )
	{
		if ((*itr)->timedOut)
		{
			// Inform everyone of the disconnection, erase 
			FOREACH(sf::Int32 identifier, (*itr)->PlayerPawnIdentifiers)
			{
				sendToAll(sf::Packet() << static_cast<sf::Int32>(Server::PlayerDisconnect) << identifier);

				mPlayerPawnInfo.erase(identifier);
			}

			mConnectedPlayers--;
			mPlayerPawnCount -= (*itr)->PlayerPawnIdentifiers.size();

			itr = mPeers.erase(itr);

			// Go back to a listening state if needed
			if (mConnectedPlayers < mMaxConnectedPlayers)
			{
				mPeers.push_back(PeerPtr(new RemotePeer()));
				setListening(true);
			}
				
			broadcastMessage("An ally has disconnected.");
		}
		else
		{
			++itr;
		}
	}
}

// Tell the newly connected peer about how the world is currently
void GameServer::informWorldState(sf::TcpSocket& socket)
{
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Server::InitialState);
	packet << mWorldHeight << mSoccerFieldRect.top + mSoccerFieldRect.height;
	packet << static_cast<sf::Int32>(mPlayerPawnCount);

	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			FOREACH(sf::Int32 identifier, mPeers[i]->PlayerPawnIdentifiers)
				packet << identifier << mPlayerPawnInfo[identifier].position.x << mPlayerPawnInfo[identifier].position.y << mSoccerBallPosition.x << mSoccerBallPosition.y << mSoccerBallVelocity.x << mSoccerBallVelocity.y;
		}
	}
	
	socket.send(packet);
}

void GameServer::broadcastMessage(const std::string& message)
{
	for (std::size_t i = 0; i < mConnectedPlayers; ++i)
	{
		if (mPeers[i]->ready)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::BroadcastMessage);
			packet << message;

			mPeers[i]->socket.send(packet);
		}	
	}
}

void GameServer::sendToAll(sf::Packet& packet)
{
	FOREACH(PeerPtr& peer, mPeers)
	{
		if (peer->ready)
			peer->socket.send(packet);
	}
}
