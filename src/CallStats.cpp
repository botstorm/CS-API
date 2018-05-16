#include "CallStats.h"

#include <atomic>
#include <array>
#include <thread>
#include "DebugLog.h"

namespace csconnector
{
	namespace call_stats
	{
        constexpr size_t NumCommands = static_cast<size_t>(Commands::Max);
		using Counter = std::atomic<int>;

		using NumCallsPerCommand = std::array<Counter, NumCommands>;

        static NumCallsPerCommand numCallsPerCommand;

		using NumCallsPerCommandStats = std::array<int, NumCommands>;

        static std::chrono::steady_clock::time_point lastUpdateTime = std::chrono::steady_clock::now();

		static std::thread thread;
        static std::atomic_bool quit{false};

		NumCallsPerCommandStats get()
		{
			NumCallsPerCommandStats stats;
			std::copy(std::begin(numCallsPerCommand), std::end(numCallsPerCommand), std::begin(stats));
			return stats;
		}

		void count(Commands command)
		{
            const size_t commandIndex = static_cast<size_t>(command);
            numCallsPerCommand.at(commandIndex).fetch_add(1, std::memory_order_relaxed);
		}

		void clear()
		{
			for (auto& c : numCallsPerCommand)
			{
				c.store(0, std::memory_order_relaxed);
			}
		}

		void start()
		{
			thread = std::thread([]()
			{
				while (!quit)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));

					NumCallsPerCommandStats stats = get();
					clear();

                    const size_t commandNumber = static_cast<size_t>(Commands::TransactionFlow);
                    const size_t requestsPerSecond = stats.at(commandNumber);
                   // DebugLog("TransactionFlow: ", requestsPerSecond, " RPS");
				}
			});
		}

		void stop()
		{
			quit = true;

			if (thread.joinable())
				thread.join();
		}
	}
}
