#include <stdio.h>
#include <cstdlib>

#include <GLFW/glfw3.h>

/*#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define assert(_cond)
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>*/

#include <00-Macro/Assert.h>
#include <02-Log/Log.h>
#include <03-Resource/ResourceManager.h>
#include <VulkanWrapper.h>

const uint32_t WIDTH = 800u;
const uint32_t HEIGHT = 600u;


int main(int _argc, char** _argv)
{
    yae::Logger logger;
    yae::ResourceManager resourceManager;

    logger.setCategoryVerbosity("glfw", yae::LogVerbosity_Verbose);
	logger.setCategoryVerbosity("vulkan", yae::LogVerbosity_Verbose);
	logger.setCategoryVerbosity("vulkan_internal", yae::LogVerbosity_Log);
	logger.setCategoryVerbosity("resource", yae::LogVerbosity_Verbose);

    if (glfwInit() != GLFW_TRUE) { YAE_ERROR_CAT("glfw", "Failed to initialize glfw"); return EXIT_FAILURE; }
    YAE_VERBOSE_CAT("glfw", "Initialized glfw");

    // Init Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL context creation
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disable resizable window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    YAE_VERBOSE_CAT("glfw", "Created glfw window");

    // Init Vulkan
	yae::VulkanWrapper vulkanWrapper;

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	vulkanWrapper.Init(window, enableValidationLayers);

    // Loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    // Shutdown
	vulkanWrapper.Shutdown();
    glfwDestroyWindow(window);
    YAE_VERBOSE_CAT("glfw", "Destroyed glfw window");
    glfwTerminate();
    YAE_VERBOSE_CAT("glfw", "Terminated glfw");

    return EXIT_SUCCESS;
}
