#pragma once

#include <core/types.h>
#include <yae/math_types.h>

namespace yae {

class Application;
class ResourceManager;
class Renderer;
class InputSystem;
class Resource;

YAE_API Application& app();
YAE_API ResourceManager& resourceManager();
YAE_API InputSystem& input();
YAE_API Renderer& renderer();

} // namespace yae
