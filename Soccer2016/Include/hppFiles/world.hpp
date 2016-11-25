#ifndef Soccer_WORLD_HPP
#define Soccer_WORLD_HPP

#include <hppFiles/ResourceHolder.hpp>
#include <hppFiles/ResourceIdentifiers.hpp>
#include <hppFiles/SceneNode.hpp>
#include <hppFiles/SpriteNode.hpp>
#include <hppFiles/SoccerPawn.hpp>
#include <hppFiles/CommandQueue.hpp>
#include <hppFiles/Command.hpp>
#include <hppFiles/BloomEffect.hpp>
#include <hppFiles/SoundPlayer.hpp>
#include <hppFiles/NetworkProtocol.hpp>

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "SoccerBall.hpp"

#include <array>
#include <queue>


// Forward declaration
namespace sf
{
	class RenderTarget;
}

class NetworkNode;

class World : private sf::NonCopyable
{
	public:
											World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked = false);
		void								update(sf::Time dt);
		void								draw();

		bool								isHost;
		SoccerBall*							soccerBallG;

		sf::FloatRect						getViewBounds() const;		
		CommandQueue&						getCommandQueue();
		SoccerPawn*							addPlayerPawn(int identifier);
		void								spawnCenterBall(bool isHost);
		void								removePlayerPawn(int identifier);
		void								setCurrentBattleFieldPosition(float lineY);
		void								setWorldHeight(float height);

		void								addEnemy(SoccerPawn::Type type, float relX, float relY);
		void								sortEnemies();

		bool 								hasAlivePlayer() const;
		bool 								hasPlayerReachedEnd() const;

		SoccerPawn*							getPlayerPawn(int identifier) const;
		sf::FloatRect						getSoccerfieldBounds() const;

		bool								pollGameAction(GameActions::Action& out);

		sf::Text							mTeam1ScoreT;
		sf::Text							mTeam2ScoreT;
		sf::Text							mStatusText;

		int									mTeam1Score = 0;
		int									mTeam2Score = 0;

	private:
		void								loadTextures();
		void								adaptPlayerPosition();
		void								adaptPlayerVelocity();
		void								handleCollisions();
		void								updateSounds();

		void								buildScene();
		void								spawnEnemies();
		void								destroyEntitiesOutsideView();


	private:
		enum Layer
		{
			Background,
			LowerAir,
			UpperAir,
			LayerCount
		};

		struct SpawnPoint 
		{
			SpawnPoint(SoccerPawn::Type type, float x, float y)
			: type(type)
			, x(x)
			, y(y)
			{
			}

			SoccerPawn::Type type;
			float x;
			float y;
		};


	private:

		sf::RenderTarget&					mTarget;
		sf::RenderTexture					mSceneTexture;
		sf::View							mWorldView;
		TextureHolder						mTextures;
		FontHolder&							mFonts;
		SoundPlayer&						mSounds;

		SceneNode							mSceneGraph;
		std::array<SceneNode*, LayerCount>	mSceneLayers;
		CommandQueue						mCommandQueue;

		sf::FloatRect						mWorldBounds;
		sf::Vector2f						mSpawnPosition;
		std::vector<SoccerPawn*>				mPlayerPawns;

		std::vector<SpawnPoint>				mEnemySpawnPoints;
		std::vector<SoccerPawn*>				mActiveEnemies;

		BloomEffect							mBloomEffect;

		bool								mNetworkedWorld;
		NetworkNode*						mNetworkNode;
		SpriteNode*							mFinishSprite;
};

#endif // Soccer_WORLD_HPP
