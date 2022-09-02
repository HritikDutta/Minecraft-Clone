#include "application.h"
#include "application_internal.h"
#include "input.h"
#include "input_processing.h"
#include "engine/engine.h"
#include "graphics/graphics.h"
#include "platform/platform.h"
#include "math/common.h"

// For random numbers
#include <ctime>

extern void CreateApp(Application& app);

int main()
{
    // For Random Numbers
    srand(time(0));

    Application app;
    CreateApp(app);

    app.window.refHeight = app.window.height;
    app.window.refWidth = app.window.width;

    SetActiveApplication(&app);

    PlatformState pstate;

    // Null iconPath means the default app icon will be used
    const char* iconPath = (app.window.iconPath.size() != 0) ? app.window.iconPath.cstr() : nullptr;
    if (!PlatformWindowStartup(pstate,
        app.window.name.cstr(),
        app.window.x, app.window.y,
        app.window.width, app.window.height,
        iconPath))
    {
        return 1;
    }

    GraphicsSetVsync(true);
    GraphicsSetClearColor(app.clearColor.r, app.clearColor.g, app.clearColor.b, app.clearColor.a);

    Engine::Init(app);

    app.OnInit(app);

    f32 prevTime = PlatformGetTime();

    while (IsApplicationRunning())
    {
        app.time = PlatformGetTime();
        app.deltaTime = Min(app.time - prevTime, 0.2f);
        prevTime = app.time;

        PlatformPumpMessages();
        GraphicsClearCanvas();

        InputGetState(app);

        app.OnUpdate(app);
        app.OnRender(app);

        GraphicsSwapBuffers(pstate);
        InputStateUpdate(app);
    }

    app.OnShutdown(app);

    Engine::Shutdown();

    PlatformWindowShutdown(pstate);
}