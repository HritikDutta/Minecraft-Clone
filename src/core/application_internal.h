#pragma once

#include "application.h"

void SetActiveApplication(Application* app);
void ApplicationWindowResizeCallback(s32 width, s32 height);

bool IsApplicationRunning();
void ApplicationExit();
Application& GetActiveApplication();