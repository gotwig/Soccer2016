#ifndef Soccer_NETWORKNODE_HPP
#define Soccer_NETWORKNODE_HPP

#include <hppFiles/SceneNode.hpp>
#include <hppFiles/NetworkProtocol.hpp>

#include <queue>


class NetworkNode : public SceneNode
{
	public:
								NetworkNode();

		void					notifyGameAction(GameActions::Type type, sf::Vector2f position);
		bool					pollGameAction(GameActions::Action& out);

		virtual unsigned int	getCategory() const;


	private:
		std::queue<GameActions::Action>	mPendingActions;
};

#endif // Soccer_NETWORKNODE_HPP
