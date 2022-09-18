project(Autumn8Apps)

add_compile_definitions(A8_MODULE_NAME="app")

a8_app(browser
  browser/browser.cpp
  TARGETS GUI
  DEPENDENCIES nlohmann_json::nlohmann_json
)

a8_app(b2
  b2/main.cpp
  b2/browser.cpp
  b2/routes.cpp
  TARGETS GUI
)

a8_app(flsaw
  flsaw.cpp
  NAME A8Saw
  TARGETS FLP
)

a8_app(mapish
  mapish/mapish.cpp
  mapish/lex.cpp
  mapish/parse.cpp
  TARGETS CLI
  DEPENDENCIES a8midi
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
