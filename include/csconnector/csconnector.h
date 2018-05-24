#pragma once

#include "API.h"
#include <functional>
#include <mutex>
#include <string>


namespace csdb{
    class Storage;
}

namespace csconnector {
struct Config
{
  int port = 9090;
};

void start(csdb::Storage* m_storage,const Config& config = {});
void stop();

enum class Commands
{
  BalanceGet,
  TransactionGet,
  TransactionsGet,
  TransactionFlow,
  PoolListGet,
  PoolInfoGet,
  PoolTransactionsGet,
  StatsGet,
  NodesInfoGet,
  Max,
  SmartContractGet
};

template<Commands C>
struct CommandTraits
{
};

#define DECL_COMMAND(id, ...)                             \
  template<>                                              \
  struct CommandTraits<id>                                \
  {                                                       \
    typedef std::function<void(__VA_ARGS__)> Handler;     \
    static Handler&                          getHandler() \
    {                                                     \
      static Handler handler;                             \
      return handler;                                     \
    }                                                     \
    static std::mutex& getMutex()                         \
    {                                                     \
      static std::mutex mutex;                            \
      return mutex;                                       \
    }                                                     \
  }

using ScopedLock = std::lock_guard<std::mutex>;

template<Commands C>
bool registerHandler(typename CommandTraits<C>::Handler handler)
{
  ScopedLock lock(CommandTraits<C>::getMutex());

  auto& m_handler = CommandTraits<C>::getHandler();
  if (m_handler != nullptr)
    return false;

  m_handler = handler;

  return true;
}

// TODO: Take handler as input, find a way to compare std::function<>
template<Commands C>
bool unregisterHandler()
{
  ScopedLock lock(CommandTraits<C>::getMutex());

  auto& h = CommandTraits<C>::getHandler();
  if (h == nullptr)
    return false;

  h = nullptr;

  return true;
}

template<Commands C, class... Args>
bool handle(Args&&... args)
{
  ScopedLock lock(CommandTraits<C>::getMutex());

  auto& handler = CommandTraits<C>::getHandler();

  bool hasHandler = (handler != nullptr);
  if (hasHandler)
    handler(args...);

  return hasHandler;
}



DECL_COMMAND(Commands::BalanceGet, api::BalanceGetResult& _return, const api::Address& address, const api::Currency& currency);

DECL_COMMAND(Commands::TransactionGet, api::TransactionGetResult& _return, const api::TransactionId& transactionId);

DECL_COMMAND(Commands::TransactionsGet,
             api::TransactionsGetResult& _return,
             const api::Address&         address,
             const int64_t               offset,
             const int64_t               limit);

DECL_COMMAND(Commands::TransactionFlow, api::TransactionFlowResult& _return, const api::Transaction& transaction);

DECL_COMMAND(Commands::PoolListGet, api::PoolListGetResult& _return, const int64_t offset, const int64_t limit);

DECL_COMMAND(Commands::PoolInfoGet, api::PoolInfoGetResult& _return, const api::PoolHash& hash, const int64_t index);

DECL_COMMAND(Commands::PoolTransactionsGet,
             api::PoolTransactionsGetResult& _return,
             const api::PoolHash&            hash,
             const int64_t                   index,
             const int64_t                   offset,
             const int64_t                   limit);

DECL_COMMAND(Commands::StatsGet, api::StatsGetResult& _return);

DECL_COMMAND(Commands::NodesInfoGet, api::NodesInfoGetResult& _return);

DECL_COMMAND(Commands::SmartContractGet, api::SmartContractGetResult& _return,const api::Address& address);
}

