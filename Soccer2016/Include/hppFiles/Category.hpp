#ifndef Soccer_CATEGORY_HPP
#define Soccer_CATEGORY_HPP


// Entity/scene node category, used to dispatch commands
namespace Category
{
	enum Type
	{
		None				= 0,
		SceneAirLayer		= 1 << 0,
		PlayerPawn			= 1 << 1,
		AlliedPawn			= 1 << 2,
		EnemyPawn			= 1 << 3,
		SoccerBall			= 1 << 4,
		AlliedThing			= 1 << 5,
		AlliedThing2		= 1 << 6,
		AlliedThing3		= 1 << 7,
		SoundEffect			= 1 << 8,
		Network				= 1 << 9,

		Pawn = PlayerPawn | AlliedPawn | EnemyPawn,
	};
}

#endif // Soccer_CATEGORY_HPP
