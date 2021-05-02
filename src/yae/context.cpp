#include "context.h"

namespace yae {

Context g_context;

Context& context() { return g_context; }

} // namespace yae
