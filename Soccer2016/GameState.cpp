#include <hppFiles/GameState.hpp>
#include <hppFiles/MusicPlayer.hpp>

#include <SFML/Graphics/RenderWindow.hpp>


GameState::GameState(StateStack& stack, Context context)
: State(stack, context)
, mWorld(*context.window, *context.fonts, *context.sounds, false)
, mPlayer(nullptr, 1, context.keys1)
{
	mWorld.addPlayerPawn(1);
	mWorld.spawnCenterBall(true);
	mWorld.isHost = true;
	mPlayer.setMissionStatus(Player::MissionRunning);

	// Play game theme 
	//context.music->play(Music::MissionTheme);
}

void GameState::draw()
{
	mWorld.draw();
}

bool GameState::update(sf::Time dt)
{
	mWorld.update(dt);

	if (!mWorld.hasAlivePlayer())
	{
		mPlayer.setMissionStatus(Player::MissionFailure);
		requestStackPush(States::GameOver);
	}

	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleRealtimeInput(commands);

	return true;
}

bool GameState::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleEvent(event, commands);

	// Escape pressed, trigger the pause screen
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
		requestStackPush(States::Pause);

	return true;
}