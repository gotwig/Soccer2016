#include <hppFiles/Player.hpp>
#include <hppFiles/CommandQueue.hpp>
#include <hppFiles/SoccerPawn.hpp>
#include <hppFiles/Foreach.hpp>
#include <hppFiles/NetworkProtocol.hpp>

#include <SFML/Network/Packet.hpp>

#include <map>
#include <string>
#include <algorithm>


using namespace std::placeholders;

struct PlayerPawnMover
{
	PlayerPawnMover(float vx, float vy, int identifier)
	: velocity(vx, vy)
	, PlayerPawnID(identifier)
	{
	}

	void operator() (SoccerPawn& PlayerPawn, sf::Time) const
	{
		if (PlayerPawn.getIdentifier() == PlayerPawnID)
			PlayerPawn.accelerate(velocity * PlayerPawn.getMaxSpeed());
	}

	sf::Vector2f velocity;
	int PlayerPawnID;
};

struct PlayerPawnFireTrigger
{
	PlayerPawnFireTrigger(int identifier)
	: PlayerPawnID(identifier)
	{
	}

	void operator() (SoccerPawn& PlayerPawn, sf::Time) const
	{
		if (PlayerPawn.getIdentifier() == PlayerPawnID)
			PlayerPawn.fire();
	}

	int PlayerPawnID;
};

Player::Player(sf::TcpSocket* socket, sf::Int32 identifier, const KeyBinding* binding)
: mKeyBinding(binding)
, mCurrentMissionStatus(MissionRunning)
, mIdentifier(identifier)
, mSocket(socket)
{
	// Set initial action bindings
	initializeActions();

	// Assign all categories to player's PlayerPawn
	FOREACH(auto& pair, mActionBinding)
 		pair.second.category = Category::PlayerPawn;
}

void Player::handleEvent(const sf::Event& event, CommandQueue& commands)
{
	// Event
	if (event.type == sf::Event::KeyPressed)
	{
		Action action;
		if (mKeyBinding && mKeyBinding->checkAction(event.key.code, action) && !isRealtimeAction(action))
		{
			// Network connected -> send event over network
			if (mSocket)
			{
				sf::Packet packet;
				packet << static_cast<sf::Int32>(Client::PlayerEvent);
				packet << mIdentifier;
				packet << static_cast<sf::Int32>(action);		
				mSocket->send(packet);
			}

			// Network disconnected -> local event
			else
			{
				commands.push(mActionBinding[action]);
			}
		}
	}

	// Realtime change (network connected)
	if ((event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) && mSocket)
	{
		Action action;
		if (mKeyBinding && mKeyBinding->checkAction(event.key.code, action) && isRealtimeAction(action))
		{
			// Send realtime change over network
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PlayerRealtimeChange);
			packet << mIdentifier;
			packet << static_cast<sf::Int32>(action);
			packet << (event.type == sf::Event::KeyPressed);
			mSocket->send(packet);
		}
	}
}

bool Player::isLocal() const
{
	// No key binding means this player is remote
	return mKeyBinding != nullptr;
}

void Player::disableAllRealtimeActions()
{
	FOREACH(auto& action, mActionProxies)
	{
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::PlayerRealtimeChange);
		packet << mIdentifier;
		packet << static_cast<sf::Int32>(action.first);
		packet << false;
		mSocket->send(packet);
	}
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
	// Check if this is a networked game and local player or just a single player game
	if ((mSocket && isLocal()) || !mSocket)
	{
		// Lookup all actions and push corresponding commands to queue
		std::vector<Action> activeActions = mKeyBinding->getRealtimeActions();
		FOREACH(Action action, activeActions)
			commands.push(mActionBinding[action]);
	}
}

void Player::handleRealtimeNetworkInput(CommandQueue& commands)
{
	if (mSocket && !isLocal())
	{
		// Traverse all realtime input proxies. Because this is a networked game, the input isn't handled directly
		FOREACH(auto pair, mActionProxies)
		{
			if (pair.second && isRealtimeAction(pair.first))
				commands.push(mActionBinding[pair.first]);
		}
	}
}

void Player::handleNetworkEvent(Action action, CommandQueue& commands)
{
	commands.push(mActionBinding[action]);
}

void Player::handleNetworkRealtimeChange(Action action, bool actionEnabled)
{
	mActionProxies[action] = actionEnabled;
}

void Player::setMissionStatus(MissionStatus status)
{
	mCurrentMissionStatus = status;
}

Player::MissionStatus Player::getMissionStatus() const
{
	return mCurrentMissionStatus;
}

void Player::initializeActions()
{
	mActionBinding[PlayerAction::MoveLeft].action      = derivedAction<SoccerPawn>(PlayerPawnMover(-1,  0, mIdentifier));	
	mActionBinding[PlayerAction::MoveRight].action     = derivedAction<SoccerPawn>(PlayerPawnMover(+1,  0, mIdentifier));
	mActionBinding[PlayerAction::MoveUp].action        = derivedAction<SoccerPawn>(PlayerPawnMover( 0, -1, mIdentifier));
	mActionBinding[PlayerAction::MoveDown].action      = derivedAction<SoccerPawn>(PlayerPawnMover( 0, +1, mIdentifier));
	mActionBinding[PlayerAction::Fire].action          = derivedAction<SoccerPawn>(PlayerPawnFireTrigger(mIdentifier));
}
