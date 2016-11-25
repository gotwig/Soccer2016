#ifndef Soccer_GAMEOVERSTATE_HPP
#define Soccer_GAMEOVERSTATE_HPP

#include <hppFiles/State.hpp>
#include <hppFiles/Container.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class GameOverState : public State
{
	public:
							GameOverState(StateStack& stack, Context context, const std::string& text);

		virtual void		draw();
		virtual bool		update(sf::Time dt);
		virtual bool		handleEvent(const sf::Event& event);


	private:
		sf::Text			mGameOverText;
		sf::Time			mElapsedTime;
};

#endif // Soccer_GAMEOVERSTATE_HPP