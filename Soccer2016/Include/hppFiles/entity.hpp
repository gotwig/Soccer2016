#ifndef Soccer_ENTITY_HPP
#define Soccer_ENTITY_HPP

#include <hppFiles/SceneNode.hpp>


class Entity : public SceneNode
{
	public:
		explicit			Entity(int hitpoints);

		void				setVelocity(sf::Vector2f velocity);
		void				setVelocity(float vx, float vy);
		void				accelerate(sf::Vector2f velocity);
		void				accelerate(float vx, float vy);
		sf::Vector2f		getVelocity() const;

		int					getHitpoints() const;
		void				setHitpoints(int points);
		virtual void		remove();



	protected:
		virtual void		updateCurrent(sf::Time dt, CommandQueue& commands);


	private:
		sf::Vector2f		mVelocity;
		int					mHitpoints;
};

#endif // Soccer_ENTITY_HPP
