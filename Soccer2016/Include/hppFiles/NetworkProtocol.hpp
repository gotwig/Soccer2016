#ifndef SOCCER_NETWORKPROTOCOL_HPP
#define SOCCER_NETWORKPROTOCOL_HPP

#include <SFML/Config.hpp>
#include <SFML/System/Vector2.hpp>


const unsigned short ServerPort = 5000;

namespace Server
{
	// Packets originated in the server
	enum PacketType
	{
		BroadcastMessage,	// format: [Int32:packetType] [string:message]
		SpawnSelf,			// format: [Int32:packetType]
		InitialState,
		PlayerEvent,
		PlayerRealtimeChange,
		PlayerConnect,
		PlayerDisconnect,
		AcceptCoopPartner,
		SpawnEnemy,
		SpawnPickup,
		UpdateBallClientState,
		UpdateClientState,
		MissionSuccess
	};
}

namespace Client
{
	// Packets originated in the client
	enum PacketType
	{
		PlayerEvent,
		PlayerRealtimeChange,
		RequestCoopPartner,
		BallPositionUpdate,
		PositionUpdate,
		GameEvent,
		Quit
	};
}

namespace PlayerActions
{
	enum Action
	{
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		Fire,
		ActionCount
	};
}

namespace GameActions
{
	enum Type
	{
		EnemyExplode,
	};

	struct Action
	{
		Action()
		{ // leave uninitialized
		}

		Action(Type type, sf::Vector2f position)
		: type(type)
		, position(position)
		{
		}

		Type			type;
		sf::Vector2f	position;
	};
}

#endif // BOOK_NETWORKPROTOCOL_HPP
