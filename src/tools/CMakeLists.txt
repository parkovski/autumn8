project(Autumn8Tools)

a8_app(browser
  browser/browser.cpp
  TARGETS GUI
  DEPENDENCIES nlohmann_json::nlohmann_json
)

a8_app(flsaw
  flsaw.cpp
  NAME A8Saw
  TARGETS FLP
)

a8_app(midictl
  midictl.cpp
  TARGETS CLI
  DEPENDENCIES a8midi
)

a8_app(play
  play/main.cpp
  TARGETS GUI
)

a8_app(uws-server
  uws-server.cpp
  TARGETS GUI
  IF_DEPENDENCIES uWebSockets
)
