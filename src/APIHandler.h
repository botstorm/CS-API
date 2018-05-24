#pragma once

#include <mutex>
#include "APIHandlerInterface.h"


namespace csconnector {
  class APIHandler : public csconnector::APIHandlerInterface {
  public:
    APIHandler()           = default;
    ~APIHandler() override = default;

    void BalanceGet(api::BalanceGetResult& _return, const api::Address& address, const api::Currency& currency) override;

    void TransactionGet(api::TransactionGetResult& _return
            , const api::TransactionId& transactionId) override;
    void TransactionsGet(api::TransactionsGetResult& _return
            , const api::Address& address, const int64_t offset, const int64_t limit) override;
    void TransactionFlow(api::TransactionFlowResult& _return
            , const api::Transaction& transaction) override;

    // Get list of pools from last one (head pool) to the first one.
    void PoolListGet(api::PoolListGetResult& _return, const int64_t offset
                                     , const int64_t limit) override;

    // Get pool info by pool hash. Starts looking from last one (head pool).
    void PoolInfoGet(api::PoolInfoGetResult& _return, const api::PoolHash& hash
                                                , const int64_t index) override;
    void PoolTransactionsGet(api::PoolTransactionsGetResult& _return, const api::PoolHash& hash
                    , const int64_t index, const int64_t offset, const int64_t limit) override;
    void StatsGet(api::StatsGetResult& _return) override;

    void NodesInfoGet(api::NodesInfoGetResult& _return) override;
	
    void SmartContractGet(api::SmartContractGetResult& _return, const api::Address& address) override ;
  private:
    bool GetTransaction(const api::TransactionId& transactionId, api::Transaction& transaction);
  };
} // namespace csconnector


