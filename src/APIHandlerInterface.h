#ifndef API_HANDLER_INTERFACE_H
#define API_HANDLER_INTERFACE_H

#include "API.h"
#include "APIHandlerBase.h"

namespace csconnector
{
	struct APIHandlerInterface : virtual public api::APIIf, public APIHandlerBase
	{
	};
}

#endif // API_HANDLER_INTERFACE_H