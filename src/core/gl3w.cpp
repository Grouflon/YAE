#include "gl3w.h"

namespace yae {

#if YAE_PLATFORM_WINDOWS
#include <GL/gl3w.h>
#endif

void initGl3w()
{
#if YAE_PLATFORM_WINDOWS
	static bool s_isInitialized = false;

	if (!s_isInitialized)
	{
		YAE_CAPTURE_SCOPE("gl3wInit");
		int result = gl3wInit();
		YAE_ASSERT(result == GL3W_OK);
		s_isInitialized = true;
	}
#endif
}

} // namespace yae
