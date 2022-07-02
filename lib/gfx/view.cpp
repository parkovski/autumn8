#include "a8/gfx/views/view.h"

using namespace a8::gfx;

View::~View() = default;

void View::childCreated(View *) {}
void View::childInitialized(View *) {}
