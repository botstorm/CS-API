#pragma once

#include "API.h"
#include "APIHandlerBase.h"

namespace csconnector
{
	struct APIHandlerInterface : virtual public api::APIIf, public APIHandlerBase
	{
	};
}
