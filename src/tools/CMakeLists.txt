project(Autumn8Tools)

a8_app(browser browser/browser.cpp TARGETS GUI)
a8_app(minigfx minigfx.cpp TARGETS GUI)
a8_app(uws-server uws-server.cpp TARGETS GUI IF_DEPENDENCIES uWebSockets)
a8_app(midictl midictl.cpp TARGETS CLI DEPENDENCIES a8midi)
a8_app(flsaw flsaw.cpp NAME A8Saw TARGETS FLP)
