#ifndef Soccer_COMMANDQUEUE_HPP
#define Soccer_COMMANDQUEUE_HPP

#include <hppFiles/Command.hpp>

#include <queue>


class CommandQueue
{
	public:
		void						push(const Command& command);
		Command						pop();
		bool						isEmpty() const;

		
	private:
		std::queue<Command>			mQueue;
};

#endif // Soccer_COMMANDQUEUE_HPP
