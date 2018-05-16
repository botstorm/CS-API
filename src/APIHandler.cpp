#include "APIHandler.h"
#include "CallStats.h"
#include "DebugLog.h"
#include "Validation.h"
#include "csconnector/csconnector.h"

using namespace api;

VALIDATE_BEGIN(Transaction)
VALIDATE(source).Length<Less>(101);
VALIDATE(target).Length<Less>(101);
VALIDATE(currency).Length<Less>(256);
VALIDATE_END()

namespace csconnector {
void APIHandler::BalanceGet(BalanceGetResult& _return, const Address& address, const Currency& currency)
{
    Log("BalanceGet");

    bool handled = handle<Commands::BalanceGet>(_return, address, currency);

    SetResponseStatus(_return.status, handled);
}

void APIHandler::TransactionGet(TransactionGetResult& _return, const TransactionId& transactionId)
{
    Log("TransactionGet");

    bool handled = handle<Commands::TransactionGet>(_return, transactionId);

    SetResponseStatus(_return.status, handled);
}

void APIHandler::TransactionsGet(TransactionsGetResult& _return, const Address& address, const int64_t offset, const int64_t limit)
{
    Log("TransactionsGet");

    bool handled = handle<Commands::TransactionsGet>(_return, address, offset, limit);

    SetResponseStatus(_return.status, handled);
}

namespace detail {
template<class T, class D = validation::Dummy>
validation::ValidationResult Validate(const T& arg)
{
    using namespace validation;

    ValidationResult result = ValidationTraits<T, D>::validate(arg);
    return result;
}

template<class T, class D = validation::Dummy>
std::string GetErrorMessage(const T& arg, const validation::ValidationResult& validationResult)
{
    using namespace validation;

    std::string field;
    std::string error;
    std::tie(field, error) = ValidationTraits<T, D>::formatErrorMessage(validationResult);

    return ": '" + field + "' - " + error;
}
}

void APIHandler::TransactionFlow(TransactionFlowResult& _return, const Transaction& transaction)
{
    // Log("TransactionFlow");

    {
        auto result = detail::Validate(transaction);
        if (result != validation::NoError) {
            std::string errorMessage = detail::GetErrorMessage(transaction, result);
            SetResponseStatus(_return.status, APIRequestStatusType::FAILURE, errorMessage);

            return;
        }
    }

    bool handled = handle<Commands::TransactionFlow>(_return, transaction);

    SetResponseStatus(_return.status, handled);

    call_stats::count(Commands::TransactionFlow);
}

void APIHandler::PoolListGet(PoolListGetResult& _return, const int64_t offset, const int64_t limit)
{
    Log("PoolListGet");

    bool handled = handle<Commands::PoolListGet>(_return, offset, limit);

    SetResponseStatus(_return.status, handled);
}

void APIHandler::PoolTransactionsGet(PoolTransactionsGetResult& _return,
                                     const PoolHash&            hash,
                                     const int64_t              index,
                                     const int64_t              offset,
                                     const int64_t              limit)
{
    Log("PoolTransactionsGet");
    bool isHandled = handle<Commands::PoolTransactionsGet>(_return, hash, index, offset, limit);

    SetResponseStatus(_return.status, isHandled);
}

void APIHandler::PoolInfoGet(PoolInfoGetResult& _return, const PoolHash& hash, const int64_t index)
{
    Log("PoolInfoGet");

    bool handled = handle<Commands::PoolInfoGet>(_return, hash, index);

    SetResponseStatus(_return.status, handled);
}

void APIHandler::StatsGet(api::StatsGetResult& _return)
{
    Log("StatsGet");

    bool handled = handle<Commands::StatsGet>(_return);

    SetResponseStatus(_return.status, handled);
}

void APIHandler::NodesInfoGet(api::NodesInfoGetResult& _return)
{
    Log("NodesInfoGet");

    bool isHandled = handle<Commands::NodesInfoGet>(_return);

    SetResponseStatus(_return.status, isHandled);
}


    void APIHandler::SmartContractGet(api::SmartContractGetResult &_return, const api::Address &address) {
        Log("SmartContractGet");

        bool isHandled = handle<Commands::SmartContractGet>(_return,address);

        SetResponseStatus(_return.status, isHandled);
    }
}
