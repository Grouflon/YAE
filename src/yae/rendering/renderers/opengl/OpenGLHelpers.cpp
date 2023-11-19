#include "OpenGLHelpers.h"

namespace yae {

const char* glErrorToString(GLint _errorCode)
{
	switch (_errorCode)
	{
		case GL_NO_ERROR: return "GL_NO_ERROR";
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		default: return "Unknown error";
	}
}

} // namespace yae
