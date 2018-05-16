#include "DBHandlers.h"

// csdb
#include <csdb/address.h>
#include <csdb/amount.h>
#include <csdb/currency.h>
#include <csdb/pool.h>
#include <csdb/transaction.h>
#include <csdb/wallet.h>
#include <csdb/csdb.h>

#include "csconnector/csconnector.h"
#include <algorithm>
#include <cassert>

csdb::Storage* DbHandlers::s_storage{nullptr};


void DbHandlers::init(csdb::Storage* m_storage) 
{
  std::string message;

  if(m_storage != nullptr)
	s_storage = m_storage;

  const bool isStorageOpen = s_storage->isOpen();

  if (isStorageOpen) 
  {
    message = "Storage is opened normal";
    csconnector::registerHandler<csconnector::Commands::BalanceGet>(BalanceGet);
    csconnector::registerHandler<csconnector::Commands::TransactionGet>(TransactionGet);
    csconnector::registerHandler<csconnector::Commands::TransactionsGet>(TransactionsGet);
    csconnector::registerHandler<csconnector::Commands::PoolInfoGet>(PoolInfoGet);
    csconnector::registerHandler<csconnector::Commands::PoolTransactionsGet>(PoolTransactionsGet);
    csconnector::registerHandler<csconnector::Commands::PoolListGet>(PoolListGet);
  } 
  else
  {
    message = "Storage is not opedened: " + s_storage->last_error_message();
  }

  std::cout << message << std::endl;
}

void DbHandlers::deinit() {
  csconnector::unregisterHandler<csconnector::Commands::BalanceGet>();
  csconnector::unregisterHandler<csconnector::Commands::TransactionGet>();
  csconnector::unregisterHandler<csconnector::Commands::TransactionsGet>();
  csconnector::unregisterHandler<csconnector::Commands::PoolInfoGet>();
  csconnector::unregisterHandler<csconnector::Commands::PoolTransactionsGet>();
  csconnector::unregisterHandler<csconnector::Commands::PoolListGet>();
}


void DbHandlers::BalanceGet(api::BalanceGetResult& _return, const api::Address& address, const api::Currency& currency) {
	//
}

void DbHandlers::TransactionGet(api::TransactionGetResult& _return, const api::TransactionId& transactionId) {
  const csdb::TransactionID& tmpTransactionId = csdb::TransactionID::from_string(transactionId);
  const csdb::Transaction&  transaction       = s_storage->transaction(tmpTransactionId);
  _return.found                               = transaction.is_valid();
  if (_return.found) {
      _return.transaction = convertTransaction(transaction);
  }
}

void DbHandlers::TransactionsGet(api::TransactionsGetResult& _return, const api::Address& addressString, const int64_t offset, const int64_t limit) {
  const auto& address      = csdb::Address::from_string(addressString);
  const auto& transactions = s_storage->transactions(address, limit);
  _return.transactions     = convertTransactions(transactions);
}

void DbHandlers::PoolListGet(api::PoolListGetResult& _return, const int64_t offset, const int64_t limit) {
  csdb::PoolHash offsetedPoolHash;
  for (int64_t i = 0; i < offset; ++i) {
    const csdb::PoolHash& tmp = s_storage->last_hash();
    offsetedPoolHash          = s_storage->pool_load(tmp).previous_hash();
  }
  for (int64_t i = 0; i < limit; ++i) {
    const api::Pool& apiPool = convertPool(offsetedPoolHash);
    _return.pools.push_back(apiPool);
  }
}

void DbHandlers::PoolInfoGet(api::PoolInfoGetResult& _return, const api::PoolHash& hash, const int64_t index) {
  const csdb::PoolHash poolHash = csdb::PoolHash::from_string(hash);
  const csdb::Pool     pool     = s_storage->pool_load(poolHash);
  _return.isFound               = pool.is_valid();

  if (_return.isFound) {
    _return.pool = convertPool(poolHash);
  }
}

void DbHandlers::PoolTransactionsGet(api::PoolTransactionsGetResult& _return, const api::PoolHash& poolHashString, const int64_t index, const int64_t offset, const int64_t limit) {
  const csdb::PoolHash poolHash = csdb::PoolHash::from_string(poolHashString);
  const csdb::Pool&    pool     = s_storage->pool_load(poolHash);

  if (pool.is_valid()) {
    _return.transactions = extractTransactions(pool, limit, offset);
  }
}


api::Transactions DbHandlers::convertTransactions(const std::vector<csdb::Transaction>& transactions) {
  api::Transactions result;
  result.reserve(transactions.size());
  std::transform(transactions.begin(), transactions.end(), result.begin(), convertTransaction);
  return result;
}

api::Transaction DbHandlers::convertTransaction(const csdb::Transaction& transaction) {
  api::Transaction    result;
  const csdb::Amount& amount     = transaction.amount();
  const csdb::Currency& currency = transaction.currency();
  const csdb::Address& target    = transaction.target();
  const csdb::TransactionID& id  = transaction.id();
  const csdb::Address& address   = transaction.source();

  result.amount                  = convertAmount(amount);
  result.currency                = currency.to_string();
  result.innerId                 = id.to_string();
  result.source                  = address.to_string();
  result.target                  = target.to_string();

  return result;
}

api::Amount DbHandlers::convertAmount(const csdb::Amount& amount) {
  api::Amount result;
  result.integral = amount.integral();
  result.fraction = amount.fraction();
  assert(result.fraction >= 0);
  return result;
}



api::Pool DbHandlers::convertPool(const csdb::PoolHash& poolHash) {
  api::Pool         result;
  const csdb::Pool& pool = s_storage->pool_load(poolHash);
  if (pool.is_valid()) {
    result.hash       = poolHash.to_string();
    result.poolNumber = pool.sequence();
    assert(result.poolNumber >= 0);
    result.prevHash          = pool.previous_hash().to_string();
//    result.time              = pool.user_field(); // заменить когда появится дополнительное поле
    result.transactionsCount = pool.transactions_count();
    assert(result.transactionsCount >= 0);
  }
  return result;
}

api::Transactions DbHandlers::extractTransactions(const csdb::Pool& pool, int64_t limit, const int64_t offset) {
  int64_t transactionsCount = pool.transactions_count();
  assert(transactionsCount >= 0);

  if (offset > transactionsCount) {
    return api::Transactions{}; // если запрашиваемые транзакций выходят за пределы пула возвращаем пустой результат
  }
  api::Transactions result;
  transactionsCount -= offset; // мы можем отдать все транзакции в пуле за вычетом смещения

  if (limit > transactionsCount)
    limit = transactionsCount; // лимит уменьшается до реального количества транзакций которые можно отдать

  for (int64_t index = offset; index < limit; ++index) {
    const csdb::Transaction transaction = pool.transaction(index);
    result.push_back(convertTransaction(transaction));
  }
  return result;
}
