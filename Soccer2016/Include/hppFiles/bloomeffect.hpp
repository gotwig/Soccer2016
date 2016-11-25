#ifndef Soccer_BLOOMEFFECT_HPP
#define Soccer_BLOOMEFFECT_HPP

#include <hppFiles/PostEffect.hpp>
#include <hppFiles/ResourceIdentifiers.hpp>
#include <hppFiles/ResourceHolder.hpp>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <array>


class BloomEffect : public PostEffect
{
	public:
							BloomEffect();

		virtual void		apply(const sf::RenderTexture& input, sf::RenderTarget& output);


	private:
		typedef std::array<sf::RenderTexture, 2> RenderTextureArray;


	private:
		void				prepareTextures(sf::Vector2u size);

		void				filterBright(const sf::RenderTexture& input, sf::RenderTexture& output);
		void				blurMultipass(RenderTextureArray& renderTextures);
		void				blur(const sf::RenderTexture& input, sf::RenderTexture& output, sf::Vector2f offsetFactor);
		void				downsample(const sf::RenderTexture& input, sf::RenderTexture& output);
		void				add(const sf::RenderTexture& source, const sf::RenderTexture& bloom, sf::RenderTarget& target);


	private:
		ShaderHolder		mShaders;

		sf::RenderTexture	mBrightnessTexture;
		RenderTextureArray	mFirstPassTextures;
		RenderTextureArray	mSecondPassTextures;
};

#endif // Soccer_BLOOMEFFECT_HPP
