#ifndef SOCCER_SOUNDNODE_HPP
#define SOCCER_SOUNDNODE_HPP

#include <hppFiles/SceneNode.hpp>
#include <hppFiles/ResourceIdentifiers.hpp>


class SoundPlayer;

class SoundNode : public SceneNode
{
	public:
		explicit				SoundNode(SoundPlayer& player);
		void					playSound(SoundEffect::ID sound, sf::Vector2f position);

		virtual unsigned int	getCategory() const;


	private:
		SoundPlayer&			mSounds;
};

#endif // SOCCER_SOUNDNODE_HPP
