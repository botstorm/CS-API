#pragma once

#include "API.h"

#include <csdb/address.h>
#include <csdb/amount.h>
#include <csdb/storage.h>

class DbHandlers {
public:
  static void init(csdb::Storage*);
  static void deinit();

  static void BalanceGet(api::BalanceGetResult& _return
          , const api::Address& address, const api::Currency& currency);
  static void TransactionGet(api::TransactionGetResult& _return
          , const api::TransactionId& transactionId);
  static void TransactionsGet(api::TransactionsGetResult& _return, const api::Address& addressString
          , const int64_t offset, const int64_t limit);
  static void PoolListGet(api::PoolListGetResult& _return, const int64_t offset, const int64_t limit);
  static void PoolInfoGet(api::PoolInfoGetResult& _return, const api::PoolHash& hash, const int64_t index);
  static void PoolTransactionsGet(api::PoolTransactionsGetResult& _return, const api::PoolHash& poolHashString
          , const int64_t index, const int64_t offset, const int64_t limit);

  static void SmartContractGet(api::SmartContractGetResult& _return,const api::Address& address);
  static void StatsGet(api::StatsGetResult& _return);
private:
    static api::Transactions convertTransactions(const std::vector<csdb::Transaction>& transactions);
    static api::Transaction  convertTransaction(const csdb::Transaction& transaction);
    static api::Amount       convertAmount(const csdb::Amount& amount);
    static api::Pool         convertPool(const csdb::PoolHash& poolHash);
    static api::Transactions extractTransactions(const csdb::Pool& pool, int64_t limit, const int64_t offset = 0);
    static api::SmartContract convertStringToContract(const std::string& data);


	static csdb::Address convertAddress(std::string data);
	static unsigned int BalanceTarget(api::Address address, csdb::PoolHash pool_hash);
	static unsigned int BalanceSource(api::Address address, csdb::PoolHash pool_hash);
	static bool BalanceSource(api::Address address, csdb::PoolHash pool_hash, bool flag);


    static csdb::Storage* s_storage;
};


