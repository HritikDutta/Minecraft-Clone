#include "application.h"
#include "application_internal.h"
#include "logging.h"
#include "types.h"
#include "platform/platform.h"

static Application* activeApp = nullptr;
static bool isRunning;

void SetActiveApplication(Application* app)
{
    activeApp = app;
    isRunning = true;
}

void ApplicationWindowResizeCallback(s32 width, s32 height)
{
    AssertWithMessage(activeApp, "No active application found!");

    activeApp->window.width = width;
    activeApp->window.height = height;
    activeApp->window.refWidth = (f32) width / (f32) height * activeApp->window.refHeight;

    activeApp->OnWindowResize(*activeApp);
}

bool IsApplicationRunning()
{
    return isRunning;
}

void ApplicationExit()
{
    isRunning = false;
}

Application& GetActiveApplication()
{
    AssertWithMessage(activeApp, "No active application found!");
    return *activeApp;
}

void Application::Exit()
{
    ApplicationExit();
}

void Application::ShowCursor(bool value)
{
    PlatformShowMouseCursor(value);
}