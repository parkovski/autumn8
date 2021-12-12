# a8_app(
#   target_name
#   sources
#   [NAME name]
#   TARGETS targets
#   [RESOURCES resources]
#   [DEPENDENCIES deps]
#   [IF_DEPENDENCIES if_deps]
#   [OPT_DEPENDENCIES opt_deps]
# )
#
# name      = Target output filename.
# targets   = CLI, GUI, FLP, VST3, LV2, AU
# resources = Files/folders to be placed next to the target.
# deps      = CMake dependencies.
# if_deps   = Dependencies that cause the target to be skipped if they are
#             missing.
# opt_deps  = Dependencies that are added to the target if they are available.
function(a8_app APP_NAME)
  cmake_parse_arguments(PARSE_ARGV 1 arg
    ""
    "NAME"
    "TARGETS;RESOURCES;DEPENDENCIES;IF_DEPENDENCIES;OPT_DEPENDENCIES")

  foreach(IFDEP ${arg_IF_DEPENDENCIES})
    if(NOT ${IFDEP}_FOUND)
      message(STATUS "Skipping ${APP_NAME} because ${IFDEP} was not found.")
      return()
    endif()
  endforeach()

  add_library(${APP_NAME} INTERFACE)
  target_sources(${APP_NAME} INTERFACE ${arg_UNPARSED_ARGUMENTS})

  if(NOT arg_NAME)
    set(arg_NAME ${APP_NAME})
  endif()

  if(arg_DEPENDENCIES)
    target_link_libraries(${APP_NAME} INTERFACE ${arg_DEPENDENCIES})
  endif()

  if(arg_IF_DEPENDENCIES)
    target_link_libraries(${APP_NAME} INTERFACE ${arg_IF_DEPENDENCIES})
  endif()

  foreach(OPTDEP ${arg_OPT_DEPENDENCIES})
    if(${OPTDEP}_FOUND)
      target_link_libraries(${APP_NAME} INTERFACE ${OPTDEP})
    else()
      message(STATUS "Skipping optional dependency ${OPTDEP} for ${APP_NAME}")
    endif()
  endforeach()

  if(arg_RESOURCES)
    # add_custom_target("${APP_NAME}-resources")
    # TODO
  endif()

  foreach(TGT ${arg_TARGETS})
    if(TGT STREQUAL CLI)
      add_executable("${APP_NAME}-cli")
      target_link_libraries("${APP_NAME}-cli" PRIVATE ${APP_NAME} a8main)
      set_target_properties("${APP_NAME}-cli" PROPERTIES RUNTIME_OUTPUT_NAME "${arg_NAME}")
    elseif(TGT STREQUAL GUI)
      if(A8_BUILD_GUI)
        add_executable("${APP_NAME}-gui" WIN32 MACOSX_BUNDLE)
        target_link_libraries("${APP_NAME}-gui" PRIVATE ${APP_NAME} a8gfxmain)
        set_target_properties("${APP_NAME}-gui" PROPERTIES RUNTIME_OUTPUT_NAME "${arg_NAME}")
      endif()
    elseif(TGT STREQUAL FLP)
      if(A8_FLSTUDIO_PLATFORM AND fpsdk_FOUND)
        add_library("${APP_NAME}-flp" MODULE)
        target_link_libraries("${APP_NAME}-flp" PRIVATE ${APP_NAME} a8flp)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
          set_target_properties("${APP_NAME}-flp" PROPERTIES LIBRARY_OUTPUT_NAME "${arg_NAME}_x64")
        else()
          set_target_properties("${APP_NAME}-flp" PROPERTIES LIBRARY_OUTPUT_NAME "${arg_NAME}")
        endif()
      elseif(A8_FLSTUDIO_PLATFORM)
        message(WARNING "Skipping ${APP_NAME}-flp because fpsdk was not found.")
      endif()
    elseif(TGT STREQUAL VST3)
      if(A8_VST3_PLATFORM AND A8_BUILD_GUI)
        add_library("${APP_NAME}-vst3" MODULE)
        target_link_libraries("${APP_NAME}-vst3" PRIVATE ${APP_NAME} a8vst3)
        # TODO - Surge/Linux uses "Surge.vst3/Contents/x86_64-linux/Surge.so"
        set_target_properties("${APP_NAME}-vst3" PROPERTIES
          LIBRARY_OUTPUT_NAME "${arg_NAME}"
          PREFIX ""
          SUFFIX ".vst3")
        message(SEND_ERROR "a8vst3 not implemented")
      endif()
    elseif(TGT STREQUAL LV2)
      if(A8_LV2_PLATFORM AND A8_BUILD_GUI)
        add_library("${APP_NAME}-lv2" MODULE)
        target_link_libraries("${APP_NAME}-lv2" PRIVATE ${arg_NAME} a8lv2)
        message(SEND_ERROR "a8lv2 not implemented")
      endif()
    elseif(TGT STREQUAL AU)
      if(A8_AU_PLATFORM AND A8_BUILD_GUI)
        add_library("${APP_NAME}-au" MODULE)
        target_link_libraries("${APP_NAME}-au" PRIVATE ${arg_NAME} a8au)
        message(FATAL_ERROR "a8au not implemented")
      endif()
    else()
      message(SEND_ERROR "Unknown target ${TGT}")
    endif()
  endforeach()
endfunction()
