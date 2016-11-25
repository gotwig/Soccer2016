#include <hppFiles/DataTables.hpp>
#include <hppFiles/SoccerPawn.hpp>


// For std::bind() placeholders _1, _2, ...
using namespace std::placeholders;

std::vector<PlayerPawnData> initializePlayerPawnData()
{
	std::vector<PlayerPawnData> data(SoccerPawn::TypeCount);

	data[SoccerPawn::Blue].hitpoints = 100;
	data[SoccerPawn::Blue].speed = 200.f;
	data[SoccerPawn::Blue].fireInterval = sf::seconds(1);
	data[SoccerPawn::Blue].texture = Textures::Entities;
	data[SoccerPawn::Blue].textureRect = sf::IntRect(0, 0, 48, 64);
	data[SoccerPawn::Blue].hasRollAnimation = true;

	data[SoccerPawn::Red].hitpoints = 100;
	data[SoccerPawn::Red].speed = 200.f;
	data[SoccerPawn::Red].fireInterval = sf::seconds(1);
	data[SoccerPawn::Red].texture = Textures::Entities;
	data[SoccerPawn::Red].textureRect = sf::IntRect(0, 0, 48, 64);
	data[SoccerPawn::Red].hasRollAnimation = true;

	return data;
}



