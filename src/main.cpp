#include <stdio.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <Log.h>

int main(int _argc, char** _argv)
{
  yae::g_logger.setCategoryVerbosity("glfw", yae::LogVerbosity_Verbose);

  if (glfwInit() != GLFW_TRUE)
  {
    YAE_ERROR_CAT("glfw", "Failed to initialize glfw");
    return 1;
  }
  YAE_VERBOSE_CAT("glfw", "Successfully initialized glfw");

  glfwTerminate();

  return 0;
}
