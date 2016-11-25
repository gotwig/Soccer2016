#include "SoccerBall.hpp"
#include <hppFiles/DataTables.hpp>
#include <hppFiles/Category.hpp>
#include <hppFiles/CommandQueue.hpp>
#include <hppFiles/Utility.hpp>
#include <hppFiles/ResourceHolder.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <stdio.h>


SoccerBall::SoccerBall(const TextureHolder& textures, bool isHost)
	: Entity(1)
	, mSprite(textures.get(Textures::Entities), sf::IntRect(0, 64, 40, 40))
{
	centerOrigin(mSprite);
	reset();
	mIsHost = isHost;
}

void SoccerBall::reset() {
	setPosition(500.f, 4580.f);
	setVelocity(0.f, 0.f);
	clock.restart();
}

void SoccerBall::updateCurrent(sf::Time dt, CommandQueue& commands)
{

	float stopFactor = 0.9f;

	//printf("BEGIN %f END", getPosition().y);


	if (getPosition().x <= 120 || getPosition().x >= 890) {
		ReboundSides();
	}
	if (getPosition().y <= 4320 || getPosition().y >= 4900) {
		ReboundTopSides();
	}


	if (clock.getElapsedTime() > sf::milliseconds(100)) {

		if (mIsHost) {
			if (getVelocity().x > 0) {
				setVelocity(getVelocity().x *stopFactor, getVelocity().y);
			}

			if (getVelocity().x < 0) {
				setVelocity(getVelocity().x *stopFactor, getVelocity().y);
			}

			if (getVelocity().y > 0) {
				setVelocity(getVelocity().x, getVelocity().y*stopFactor);
			}

			if (getVelocity().y < 0) {
				setVelocity(getVelocity().x, getVelocity().y*stopFactor);
			}
		}

		clock.restart();
	}

	Entity::updateCurrent(dt, commands);
}

void SoccerBall::ReboundSides() {
	setVelocity(-getVelocity().x,getVelocity().y);
}

void SoccerBall::ReboundTopSides() {
	setVelocity(getVelocity().x, -getVelocity().y);
}

unsigned int SoccerBall::getCategory() const
{
	return Category::SoccerBall;
}

sf::FloatRect SoccerBall::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void SoccerBall::apply(SoccerPawn& player)
{
	//Table[mType].action(player);
}

void SoccerBall::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}
