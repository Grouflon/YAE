#pragma once

#include <yae/types.h>

// Never include this in a .h (gl headers should not propagate into headers)
#define GL_GLEXT_PROTOTYPES
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define EGL_EGLEXT_PROTOTYPES
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#else
#include <GL/gl3w.h>
#endif

#define YAE_GL_VERIFY(_instruction) { _instruction; GLint ___error = glGetError(); YAE_ASSERT_MSGF(___error == GL_NO_ERROR, "GL Error %s(0x%04x) -> " #_instruction, yae::glErrorToString(___error), ___error); }
#define YAE_GL_TEST(_instruction) [&](){ _instruction; GLint ___error = glGetError(); if (___error != GL_NO_ERROR) { YAE_ERRORF_CAT("renderer", "GL Error %s(0x%04x) -> ", #_instruction, yae::glErrorToString(___error), ___error); } return ___error == GL_NO_ERROR; }()

namespace yae {

const char* glErrorToString(GLint _errorCode);

} // namespace yae
