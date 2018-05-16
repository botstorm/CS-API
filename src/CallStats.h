#ifndef CALL_STATS_H
#define CALL_STATS_H

#include <csconnector/csconnector.h>

namespace csconnector
{
	namespace call_stats
	{
		void start();
		void stop();

		void count(Commands command);
	}
}

#endif // CALL_STATS_H