#ifndef Soccer_DATATABLES_HPP
#define Soccer_DATATABLES_HPP

#include <hppFiles/ResourceIdentifiers.hpp>

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <vector>
#include <functional>


class PlayerPawn;

struct Direction
{
	Direction(float angle, float distance)
	: angle(angle)
	, distance(distance)
	{
	}

	float angle;
	float distance;
};

struct PlayerPawnData
{
	int								hitpoints;
	float							speed;
	Textures::ID					texture;
	sf::IntRect						textureRect;
	sf::Time						fireInterval;
	std::vector<Direction>			directions;
	bool							hasRollAnimation;
};


std::vector<PlayerPawnData>	initializePlayerPawnData();

#endif // Soccer_DATATABLES_HPP
