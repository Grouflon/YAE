#include <stdio.h>
#include <cstdlib>

//#include <GLFW/glfw3.h>

#include <00-Type/GlobalContext.h>
//#include <02-Log/Log.h>
#include <03-Application/Application.h>

int main(int _argc, char** _argv)
{
    /*yae::g_context.logger->setCategoryVerbosity("glfw", yae::LogVerbosity_Verbose);
    yae::g_context.logger->setCategoryVerbosity("vulkan", yae::LogVerbosity_Verbose);
    yae::g_context.logger->setCategoryVerbosity("vulkan_internal", yae::LogVerbosity_Log);
    yae::g_context.logger->setCategoryVerbosity("resource", yae::LogVerbosity_Verbose);*/


    yae::Application application;
    application.init("yae", 800u, 600u, _argv, _argc);
    application.run();
    application.shutdown();

    return EXIT_SUCCESS;
}
