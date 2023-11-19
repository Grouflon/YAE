#pragma once

#include <core/types.h>
#include <yae/math_types.h>

#ifndef YAE_TESTS
#define YAE_TESTS 0
#endif

#ifndef YAE_EDITOR
#define YAE_EDITOR 0
#endif

namespace yae {

class Engine;
class ResourceManager;
class Resource;
class Application;
class InputSystem;
class Renderer;

YAE_API Engine& engine();
YAE_API ResourceManager& resourceManager();
YAE_API Application& app();
YAE_API InputSystem& input();
YAE_API Renderer& renderer();

} // namespace yae
