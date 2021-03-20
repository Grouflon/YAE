#include "GlobalContext.h"

namespace yae {

GlobalContext g_context;

yae::GlobalContext* GetGlobalContext()
{
	return &g_context;
}

}
