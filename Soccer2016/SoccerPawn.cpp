#include <hppFiles/SoccerPawn.hpp>
#include <hppFiles/DataTables.hpp>
#include <hppFiles/Utility.hpp>

#include <hppFiles/CommandQueue.hpp>
#include <hppFiles/SoundNode.hpp>
#include <hppFiles/NetworkNode.hpp>
#include <hppFiles/ResourceHolder.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>


using namespace std::placeholders;

namespace
{
	const std::vector<PlayerPawnData> Table = initializePlayerPawnData();
}

SoccerPawn::SoccerPawn(Type type, const TextureHolder& textures, const FontHolder& fonts)
: Entity(Table[type].hitpoints)
, mType(type)
, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
, mFireCommand()
, mFireRateLevel(1)
, mSpreadLevel(1)
, mTravelledDistance(0.f)
, mDirectionIndex(0)
, mIdentifier(0)
{

	centerOrigin(mSprite);

	mFireCommand.category = Category::SceneAirLayer;
	mFireCommand.action   = [this, &textures] (SceneNode& node, sf::Time)
	{
		//put action here
	};



}


void SoccerPawn::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
		target.draw(mSprite, states);
}


void SoccerPawn::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Update texts and roll animation
	updateTexts();
	updateRollAnimation();

	//printf("BEGIN %f,%f END",getPosition().x, getPosition().y);
	// Update enemy movement pattern; apply velocity
	updateMovementPattern(dt);
	Entity::updateCurrent(dt, commands);
}

unsigned int SoccerPawn::getCategory() const
{
	if (isAllied())
		return Category::PlayerPawn;
	else
		return Category::EnemyPawn;
}

sf::FloatRect SoccerPawn::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

bool SoccerPawn::isMarkedForRemoval() const
{
	return false;
}

void SoccerPawn::remove()
{
	Entity::remove();
}

bool SoccerPawn::isAllied() const
{
	return mType == Blue;
}

float SoccerPawn::getMaxSpeed() const
{
	return Table[mType].speed;
}


void SoccerPawn::increaseSpread()
{
	if (mSpreadLevel < 3)
		++mSpreadLevel;
}


void SoccerPawn::fire()
{
// kicking and more doesnt work yet
}


void SoccerPawn::playLocalSound(CommandQueue& commands, SoundEffect::ID effect)
{
	sf::Vector2f worldPosition = getWorldPosition();

	Command command;
	command.category = Category::SoundEffect;
	command.action = derivedAction<SoundNode>(
		[effect, worldPosition] (SoundNode& node, sf::Time)
		{
			node.playSound(effect, worldPosition); 
		});

	commands.push(command);
}

int	SoccerPawn::getIdentifier()
{
	return mIdentifier;
}

void SoccerPawn::setIdentifier(int identifier)
{
	mIdentifier = identifier;
}

void SoccerPawn::updateMovementPattern(sf::Time dt)
{
	// Enemy player: Movement pattern
	const std::vector<Direction>& directions = Table[mType].directions;
	if (!directions.empty())
	{
		// Moved long enough in current direction: Change direction
		if (mTravelledDistance > directions[mDirectionIndex].distance)
		{
			mDirectionIndex = (mDirectionIndex + 1) % directions.size();
			mTravelledDistance = 0.f;
		}

		// Compute velocity from direction
		float radians = toRadian(directions[mDirectionIndex].angle + 90.f);
		float vx = getMaxSpeed() * std::cos(radians);
		float vy = getMaxSpeed() * std::sin(radians);

		setVelocity(vx, vy);

		mTravelledDistance += getMaxSpeed() * dt.asSeconds();
	}
}





void SoccerPawn::updateTexts()
{

}

void SoccerPawn::updateRollAnimation()
{
	if (Table[mType].hasRollAnimation)
	{
		sf::IntRect textureRect = Table[mType].textureRect;

		// Roll left: Texture rect offset once
		if (getVelocity().x < 0.f)
			textureRect.left += textureRect.width;

		// Roll right: Texture rect offset twice
		else if (getVelocity().x > 0.f)
			textureRect.left += 2 * textureRect.width;

		mSprite.setTextureRect(textureRect);
	}
}
