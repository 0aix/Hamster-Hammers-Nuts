// https://github.com/ixchow/15-466-f17-base3
#pragma once

//"GL.hpp" is a convenience header to include a minimal set of "modern" OpenGL function prototypes.
// -- this is in contrast to, e.g., SDL_OpenGL which may include a bunch of OpenGL1.2 cruft.

#ifdef _WIN32
#include "gl_shims.hpp"
#undef near
#undef min
#undef max
#else
#define GL_GLEXT_PROTOTYPES 1
#include "glcorearb.h"
#endif