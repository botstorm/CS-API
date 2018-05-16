#pragma once
#include "API.h"

namespace csconnector
{
    class APIHandlerBase
	{
    public:
		enum class APIRequestStatusType : uint8_t
		{
			SUCCESS,
			FAILURE,
			NOT_IMPLEMENTED,
			MAX
		};

		static void SetResponseStatus(api::APIResponse& response
				, APIRequestStatusType status, const std::string& details = "");
		static void SetResponseStatus(api::APIResponse& response
				, bool commandWasHandled);
	};
}
