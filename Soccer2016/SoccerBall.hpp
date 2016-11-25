
#include <hppFiles/Entity.hpp>
#include <hppFiles/Command.hpp>
#include <hppFiles/ResourceIdentifiers.hpp>
#include <SFML/System/Clock.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <hppFiles\SoccerPawn.hpp>

class SoccerPawn;

class SoccerBall : public Entity
{

public:

	SoccerBall(const TextureHolder& textures, bool isHost);

	virtual unsigned int	getCategory() const;
	virtual sf::FloatRect	getBoundingRect() const;

	void 					apply(SoccerPawn& player);
	void					ReboundSides();
	void					ReboundTopSides();
	sf::Time                dt; // delta time
	sf::Time                elapsedTime;
	sf::Clock				clock;
	void					reset();
	bool					mIsHost;
private:
	virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void 			updateCurrent(sf::Time dt, CommandQueue& commands);
	sf::Sprite				mSprite;

};
