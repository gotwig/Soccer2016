#ifndef Soccer_GAMESTATE_HPP
#define Soccer_GAMESTATE_HPP

#include <hppFiles/State.hpp>
#include <hppFiles/World.hpp>
#include <hppFiles/Player.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class GameState : public State
{
	public:
							GameState(StateStack& stack, Context context);

		virtual void		draw();
		virtual bool		update(sf::Time dt);
		virtual bool		handleEvent(const sf::Event& event);


	private:
		World				mWorld;
		Player				mPlayer;
};

#endif // Soccer_GAMESTATE_HPP
