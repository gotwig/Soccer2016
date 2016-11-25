#include <hppFiles/Utility.hpp>
#include <hppFiles/Animation.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <random>
#include <cmath>
#include <ctime>
#include <cassert>


namespace
{
	std::default_random_engine createRandomEngine()
	{
		auto seed = static_cast<unsigned long>(std::time(nullptr));
		return std::default_random_engine(seed);
	}

	auto RandomEngine = createRandomEngine();
}

std::string toString(sf::Keyboard::Key key)
{
	#define Soccer_KEYTOSTRING_CASE(KEY) case sf::Keyboard::KEY: return #KEY;

	switch (key)
	{
		Soccer_KEYTOSTRING_CASE(Unknown)
		Soccer_KEYTOSTRING_CASE(A)
		Soccer_KEYTOSTRING_CASE(B)
		Soccer_KEYTOSTRING_CASE(C)
		Soccer_KEYTOSTRING_CASE(D)
		Soccer_KEYTOSTRING_CASE(E)
		Soccer_KEYTOSTRING_CASE(F)
		Soccer_KEYTOSTRING_CASE(G)
		Soccer_KEYTOSTRING_CASE(H)
		Soccer_KEYTOSTRING_CASE(I)
		Soccer_KEYTOSTRING_CASE(J)
		Soccer_KEYTOSTRING_CASE(K)
		Soccer_KEYTOSTRING_CASE(L)
		Soccer_KEYTOSTRING_CASE(M)
		Soccer_KEYTOSTRING_CASE(N)
		Soccer_KEYTOSTRING_CASE(O)
		Soccer_KEYTOSTRING_CASE(P)
		Soccer_KEYTOSTRING_CASE(Q)
		Soccer_KEYTOSTRING_CASE(R)
		Soccer_KEYTOSTRING_CASE(S)
		Soccer_KEYTOSTRING_CASE(T)
		Soccer_KEYTOSTRING_CASE(U)
		Soccer_KEYTOSTRING_CASE(V)
		Soccer_KEYTOSTRING_CASE(W)
		Soccer_KEYTOSTRING_CASE(X)
		Soccer_KEYTOSTRING_CASE(Y)
		Soccer_KEYTOSTRING_CASE(Z)
		Soccer_KEYTOSTRING_CASE(Num0)
		Soccer_KEYTOSTRING_CASE(Num1)
		Soccer_KEYTOSTRING_CASE(Num2)
		Soccer_KEYTOSTRING_CASE(Num3)
		Soccer_KEYTOSTRING_CASE(Num4)
		Soccer_KEYTOSTRING_CASE(Num5)
		Soccer_KEYTOSTRING_CASE(Num6)
		Soccer_KEYTOSTRING_CASE(Num7)
		Soccer_KEYTOSTRING_CASE(Num8)
		Soccer_KEYTOSTRING_CASE(Num9)
		Soccer_KEYTOSTRING_CASE(Escape)
		Soccer_KEYTOSTRING_CASE(LControl)
		Soccer_KEYTOSTRING_CASE(LShift)
		Soccer_KEYTOSTRING_CASE(LAlt)
		Soccer_KEYTOSTRING_CASE(LSystem)
		Soccer_KEYTOSTRING_CASE(RControl)
		Soccer_KEYTOSTRING_CASE(RShift)
		Soccer_KEYTOSTRING_CASE(RAlt)
		Soccer_KEYTOSTRING_CASE(RSystem)
		Soccer_KEYTOSTRING_CASE(Menu)
		Soccer_KEYTOSTRING_CASE(LBracket)
		Soccer_KEYTOSTRING_CASE(RBracket)
		Soccer_KEYTOSTRING_CASE(SemiColon)
		Soccer_KEYTOSTRING_CASE(Comma)
		Soccer_KEYTOSTRING_CASE(Period)
		Soccer_KEYTOSTRING_CASE(Quote)
		Soccer_KEYTOSTRING_CASE(Slash)
		Soccer_KEYTOSTRING_CASE(BackSlash)
		Soccer_KEYTOSTRING_CASE(Tilde)
		Soccer_KEYTOSTRING_CASE(Equal)
		Soccer_KEYTOSTRING_CASE(Dash)
		Soccer_KEYTOSTRING_CASE(Space)
		Soccer_KEYTOSTRING_CASE(Return)
		Soccer_KEYTOSTRING_CASE(BackSpace)
		Soccer_KEYTOSTRING_CASE(Tab)
		Soccer_KEYTOSTRING_CASE(PageUp)
		Soccer_KEYTOSTRING_CASE(PageDown)
		Soccer_KEYTOSTRING_CASE(End)
		Soccer_KEYTOSTRING_CASE(Home)
		Soccer_KEYTOSTRING_CASE(Insert)
		Soccer_KEYTOSTRING_CASE(Delete)
		Soccer_KEYTOSTRING_CASE(Add)
		Soccer_KEYTOSTRING_CASE(Subtract)
		Soccer_KEYTOSTRING_CASE(Multiply)
		Soccer_KEYTOSTRING_CASE(Divide)
		Soccer_KEYTOSTRING_CASE(Left)
		Soccer_KEYTOSTRING_CASE(Right)
		Soccer_KEYTOSTRING_CASE(Up)
		Soccer_KEYTOSTRING_CASE(Down)
		Soccer_KEYTOSTRING_CASE(Numpad0)
		Soccer_KEYTOSTRING_CASE(Numpad1)
		Soccer_KEYTOSTRING_CASE(Numpad2)
		Soccer_KEYTOSTRING_CASE(Numpad3)
		Soccer_KEYTOSTRING_CASE(Numpad4)
		Soccer_KEYTOSTRING_CASE(Numpad5)
		Soccer_KEYTOSTRING_CASE(Numpad6)
		Soccer_KEYTOSTRING_CASE(Numpad7)
		Soccer_KEYTOSTRING_CASE(Numpad8)
		Soccer_KEYTOSTRING_CASE(Numpad9)
		Soccer_KEYTOSTRING_CASE(F1)
		Soccer_KEYTOSTRING_CASE(F2)
		Soccer_KEYTOSTRING_CASE(F3)
		Soccer_KEYTOSTRING_CASE(F4)
		Soccer_KEYTOSTRING_CASE(F5)
		Soccer_KEYTOSTRING_CASE(F6)
		Soccer_KEYTOSTRING_CASE(F7)
		Soccer_KEYTOSTRING_CASE(F8)
		Soccer_KEYTOSTRING_CASE(F9)
		Soccer_KEYTOSTRING_CASE(F10)
		Soccer_KEYTOSTRING_CASE(F11)
		Soccer_KEYTOSTRING_CASE(F12)
		Soccer_KEYTOSTRING_CASE(F13)
		Soccer_KEYTOSTRING_CASE(F14)
		Soccer_KEYTOSTRING_CASE(F15)
		Soccer_KEYTOSTRING_CASE(Pause)
	}

	return "";
}

void centerOrigin(sf::Sprite& sprite)
{
	sf::FloatRect bounds = sprite.getLocalBounds();
	sprite.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void centerOrigin(sf::Text& text)
{
	sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void centerOrigin(Animation& animation)
{
	sf::FloatRect bounds = animation.getLocalBounds();
	animation.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

float toDegree(float radian)
{
	return 180.f / 3.141592653589793238462643383f * radian;
}

float toRadian(float degree)
{
	return 3.141592653589793238462643383f / 180.f * degree;
}

int randomInt(int exclusiveMax)
{
	std::uniform_int_distribution<> distr(0, exclusiveMax - 1);
	return distr(RandomEngine);
}

float length(sf::Vector2f vector)
{
	return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

sf::Vector2f unitVector(sf::Vector2f vector)
{
	assert(vector != sf::Vector2f(0.f, 0.f));
	return vector / length(vector);
}