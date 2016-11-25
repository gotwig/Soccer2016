#include <hppFiles/World.hpp>
#include <hppFiles/Foreach.hpp>
#include <hppFiles/TextNode.hpp>
#include <hppFiles/SoundNode.hpp>
#include <hppFiles/NetworkNode.hpp>
#include <hppFiles/Utility.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <cmath>
#include <limits>


World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked)
: mTarget(outputTarget)
, mSceneTexture()
, mWorldView(outputTarget.getDefaultView())
, mTextures() 
, mFonts(fonts)
, mSounds(sounds)
, mSceneGraph()
, mSceneLayers()
, mWorldBounds(0.f, 0.f, mWorldView.getSize().x, 5000.f)
, mSpawnPosition(mWorldView.getSize().x / 2.f, mWorldBounds.height - mWorldView.getSize().y / 2.f)
, mPlayerPawns()
, mEnemySpawnPoints()
, mActiveEnemies()
, mNetworkedWorld(networked)
, mNetworkNode(nullptr)
, mFinishSprite(nullptr)
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	loadTextures();
	buildScene();

	// Prepare the view
	mWorldView.setCenter(mSpawnPosition);

	mTeam1ScoreT.setFont(mFonts.get(Fonts::Main));
	mTeam1ScoreT.setPosition(500.f / 2, 20.f);

	mTeam2ScoreT.setFont(mFonts.get(Fonts::Main));
	mTeam2ScoreT.setPosition(1500.f / 2, 20.f);

}


void World::update(sf::Time dt)
{	

	FOREACH(SoccerPawn* a, mPlayerPawns)
		a->setVelocity(0.f, 0.f);


	// Forward commands to scene graph, adapt velocity (diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);

	adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions();

	// Remove playerpawns that were destroyed (World::removeWrecks() only destroys the entities, not the pointers in mPlayerplayerpawn)
	auto firstToRemove = std::remove_if(mPlayerPawns.begin(), mPlayerPawns.end(), std::mem_fn(&SoccerPawn::isMarkedForRemoval));
	mPlayerPawns.erase(firstToRemove, mPlayerPawns.end());

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();

	updateSounds();

	mTeam1ScoreT.setString("Team1: " + std::to_string(mTeam1Score));
	mTeam2ScoreT.setString("Team2: " + std::to_string(mTeam2Score));

}

void World::draw()
{
	if (PostEffect::isSupported())
	{
		mSceneTexture.clear();
		mSceneTexture.setView(mWorldView);
		mSceneTexture.draw(mSceneGraph);
		mSceneTexture.display();
		mBloomEffect.apply(mSceneTexture, mTarget);

		mTarget.draw(mTeam1ScoreT);
		mTarget.draw(mTeam2ScoreT);
		mTarget.draw(mStatusText);

	}
	else
	{
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);

		mTarget.draw(mTeam1ScoreT);
		mTarget.draw(mTeam2ScoreT);
		mTarget.draw(mStatusText);
	}
}

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}

SoccerPawn* World::getPlayerPawn(int identifier) const
{
	FOREACH(SoccerPawn* a, mPlayerPawns)
	{
		if (a->getIdentifier() == identifier)
			return a;
	}

	return nullptr;
}

void World::removePlayerPawn(int identifier)
{
	SoccerPawn* playerpawn = getPlayerPawn(identifier);
	if (playerpawn)
	{
		mPlayerPawns.erase(std::find(mPlayerPawns.begin(), mPlayerPawns.end(), playerpawn));
	}
}

SoccerPawn* World::addPlayerPawn(int identifier)
{
	std::unique_ptr<SoccerPawn> player(new SoccerPawn(SoccerPawn::Blue, mTextures, mFonts));

	switch (identifier) {
	case 1:
		player->setPosition(450.f, 4620.f);
		break;
	case 2:
		player->setPosition(450.f, 4520.f);
		break;
	case 3:
		player->setPosition(600.f, 4620.f);
		break;
	case 4:
		player->setPosition(600.f, 4520.f);
		break;
	default:
		player->setPosition(450.f, 4620.f);
		break;
	}

	player->setIdentifier(identifier);
	printf("%i", identifier);
	mPlayerPawns.push_back(player.get());
	mSceneLayers[UpperAir]->attachChild(std::move(player));
	return mPlayerPawns.back();
}


bool World::pollGameAction(GameActions::Action& out)
{
	return mNetworkNode->pollGameAction(out);
}

void World::setCurrentBattleFieldPosition(float lineY)
{
	mWorldView.setCenter(mWorldView.getCenter().x, lineY - mWorldView.getSize().y/2);
	mSpawnPosition.y = mWorldBounds.height; 
}

void World::setWorldHeight(float height)
{
	mWorldBounds.height = height;
}

bool World::hasAlivePlayer() const
{
	return mPlayerPawns.size() > 0;
}

bool World::hasPlayerReachedEnd() const
{
	if (SoccerPawn* playerpawn = getPlayerPawn(1))
		return !mWorldBounds.contains(playerpawn->getPosition());
	else 
		return false;
}

void World::loadTextures()
{
	mTextures.load(Textures::Entities, "Media/Textures/Entities.png");
	mTextures.load(Textures::Background, "Media/Textures/soccerfieldBack.png");
}

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;

	FOREACH(SoccerPawn* playerpawn, mPlayerPawns)
	{
		sf::Vector2f position = playerpawn->getPosition();
		position.x = std::max(position.x, viewBounds.left + borderDistance);
		position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
		position.y = std::max(position.y, viewBounds.top + borderDistance);
		position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
		playerpawn->setPosition(position);
	}
}

void World::adaptPlayerVelocity()
{
	FOREACH(SoccerPawn* playerpawn, mPlayerPawns)
	{
		sf::Vector2f velocity = playerpawn->getVelocity();

		// If moving diagonally, reduce velocity (to have always same velocity)
		if (velocity.x != 0.f && velocity.y != 0.f)
			playerpawn->setVelocity(velocity / std::sqrt(2.f));

	}
}

bool matchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
{
	unsigned int category1 = colliders.first->getCategory();
	unsigned int category2 = colliders.second->getCategory();

	// Make sure first pair entry has category type1 and second has type2
	if (type1 & category1 && type2 & category2)
	{
		return true;
	}
	else if (type1 & category2 && type2 & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	if (soccerBallG) {
		if (soccerBallG->getPosition().y > 4495 && soccerBallG->getPosition().y < 4650) {
			if (soccerBallG->getPosition().x >= 875) {
				static_cast<SoccerBall&>(*soccerBallG).reset();
				mTeam1Score++;
			}
			if (soccerBallG->getPosition().x <= 122) {
				static_cast<SoccerBall&>(*soccerBallG).reset();
				mTeam2Score++;
			}
		}
	}

	FOREACH(SceneNode::Pair pair, collisionPairs)
	{
		if (matchesCategories(pair, Category::PlayerPawn, Category::SoccerBall))
		{
			auto& player = static_cast<SoccerPawn&>(*pair.first);
			auto& soccerBall = static_cast<SoccerBall&>(*pair.second);

			if (player.getCategory() == 2 ) {
				const float strength = 0.4f; //adjust this to control amount of power added to ball
				
				sf::Vector2f direction = soccerBall.getPosition() - player.getPosition();
				
				if (isHost){
					// In case you are the host, you can set the velocity of the ball directly.
					// Kicking works by going to the ball, and pushing it away
					soccerBall.setVelocity(soccerBall.getVelocity() + (direction * strength));
				}

			}
		}
	}
}

void World::updateSounds()
{
	sf::Vector2f listenerPosition;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (mPlayerPawns.empty())
	{
		listenerPosition = mWorldView.getCenter();
	}

	// 1 or more players -> mean position between all playerpawns
	else
	{
		FOREACH(SoccerPawn* playerpawn, mPlayerPawns)
			listenerPosition += playerpawn->getWorldPosition();

		listenerPosition /= static_cast<float>(mPlayerPawns.size());
	}

	// Set listener's position
	mSounds.setListenerPosition(listenerPosition);

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

void World::buildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		Category::Type category = (i == LowerAir) ? Category::SceneAirLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	//spawnCenterBall();

	// Prepare the background
	sf::Texture& backgroundTexture = mTextures.get(Textures::Background);
	
	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(backgroundTexture));
	backgroundSprite->setPosition(.0f, 4230.0f);
	mSceneLayers[Background]->attachChild(std::move(backgroundSprite));

	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(mSounds));
	mSceneGraph.attachChild(std::move(soundNode));

	// Add network node, if necessary
	if (mNetworkedWorld)
	{
		std::unique_ptr<NetworkNode> networkNode(new NetworkNode());
		mNetworkNode = networkNode.get();
		mSceneGraph.attachChild(std::move(networkNode));
	}


}

void World::spawnCenterBall(bool isHost)
{
	std::unique_ptr<SoccerBall> soccerBallT(new SoccerBall(mTextures,isHost));

	soccerBallG = soccerBallT.get();

	mSceneLayers[UpperAir]->attachChild(std::move(soccerBallT));

}


sf::FloatRect World::getViewBounds() const
{
	return sf::FloatRect(mWorldView.getCenter() - mWorldView.getSize() / 2.5f, mWorldView.getSize() / 1.25f);
}

sf::FloatRect World::getSoccerfieldBounds() const
{
	// Return view bounds + some area at top, where enemies spawn
	sf::FloatRect bounds = getViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;
}
