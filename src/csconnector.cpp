#include "csconnector/csconnector.h"
#include "DebugLog.h"
#include "APIHandler.h"
#include "DBHandlers.h"
#include "CallStats.h"

#include <thread>
#include <memory>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/server/TThreadedServer.h>

#include <csdb/Storage.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace api;

namespace csconnector
{

	namespace detail
	{
		std::unique_ptr<TThreadedServer> server = nullptr;

		std::mutex mutex;

		// std::scoped_lock is available only since C++17
		typedef std::lock_guard<std::mutex> ScopedLock;

		std::thread thread;

		void start(const Config& config)
		{
			ScopedLock lock(mutex);

			if (server != nullptr)
				return;

			int port = config.port;

			stdcxx::shared_ptr<api::APIIf> handler(new APIHandler());
			stdcxx::shared_ptr<TProcessor> processor(new APIProcessor(handler));

			// Лучше открывать сервер только на localhost - не надо его делать доступным снаружи!
			//stdcxx::shared_ptr<TServerTransport> serverTransport(new TServerSocket("localhost", port));

			// Java client won't connect with the line above
			// WORKAROUND: Temporary use this
			stdcxx::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));

			stdcxx::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
			stdcxx::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

			server = std::make_unique<TThreadedServer>(processor, serverTransport, transportFactory, protocolFactory);

			thread = std::thread([=]()
								 {
									 try
									 {
										 Log("csconnector started on port ", port);

										 server->run();

									 }
									 catch (...)
									 {
										 std::cout << "Oh no! I'm dead :'-(" << std::endl;
									 }
								 });
		}

		void stop()
		{
			ScopedLock lock(mutex);

			if (server != nullptr)
				server->stop();

			if (thread.joinable())
				thread.join();

			server.reset();

			Log("csconnector stopped");
		}
	}

    void start(csdb::Storage* m_storage,const Config& config /*= {}*/)
    {
        DbHandlers::init(m_storage);
        detail::start(config);
    }

    void stop()
    {
        detail::stop();
        DbHandlers::deinit();
    }



}
