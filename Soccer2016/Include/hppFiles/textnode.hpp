#ifndef Soccer_TEXTNODE_HPP
#define Soccer_TEXTNODE_HPP

#include <hppFiles/ResourceHolder.hpp>
#include <hppFiles/ResourceIdentifiers.hpp>
#include <hppFiles/SceneNode.hpp>

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>


class TextNode : public SceneNode
{
	public:
		explicit			TextNode(const FontHolder& fonts, const std::string& text);

		void				setString(const std::string& text);


	private:
		virtual void		drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;


	private:
		sf::Text			mText;
};

#endif // Soccer_TEXTNODE_HPP
