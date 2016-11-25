#ifndef Soccer_PLAYERPAWN_HPP
#define Soccer_PLAYERPAWN_HPP

#include <hppFiles/Entity.hpp>
#include <hppFiles/Command.hpp>
#include <hppFiles/ResourceIdentifiers.hpp>
#include <hppFiles/TextNode.hpp>
#include <hppFiles/Animation.hpp>

#include <SFML/Graphics/Sprite.hpp>


class SoccerPawn : public Entity
{
	public:
		enum Type
		{
			Blue,
			Red,
			TypeCount
		};


	public:
								SoccerPawn(Type type, const TextureHolder& textures, const FontHolder& fonts);

		virtual unsigned int	getCategory() const;
		virtual sf::FloatRect	getBoundingRect() const;
		virtual void			remove();
		virtual bool 			isMarkedForRemoval() const;
		bool					isAllied() const;
		float					getMaxSpeed() const;

		void 					fire();
		void					playLocalSound(CommandQueue& commands, SoundEffect::ID effect);
		int						getIdentifier();
		void					setIdentifier(int identifier);

	private:
		virtual void			drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
		virtual void 			updateCurrent(sf::Time dt, CommandQueue& commands);
		void					updateMovementPattern(sf::Time dt);

		void					updateTexts();
		void					updateRollAnimation();
		void					increaseSpread();

	private:
		Type					mType;
		sf::Sprite				mSprite;
		Command 				mFireCommand;

		int						mFireRateLevel;
		int						mSpreadLevel;

		float					mTravelledDistance;
		std::size_t				mDirectionIndex;
	
		int						mIdentifier;};

#endif // Soccer_PLAYERPAWN_HPP
