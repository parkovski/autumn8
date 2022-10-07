#pragma once

namespace a8::win {

int runMessageLoop();
int runConstantFPSMessageLoop(void (*callback)(), int fps = 30);
void postQuitMessage(int exitCode);

} // namespace a8::win
