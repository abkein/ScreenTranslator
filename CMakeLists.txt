cmake_minimum_required(VERSION 3.16)

project(ScreenTranslator VERSION 3.3.0 LANGUAGES C CXX)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(DEFINED ENV{ST_DEPS_DIR} AND NOT "$ENV{ST_DEPS_DIR}" STREQUAL "")
  set(_st_default_deps_dir "$ENV{ST_DEPS_DIR}")
else()
  set(_st_default_deps_dir "${CMAKE_CURRENT_SOURCE_DIR}/../deps")
endif()
set(ST_DEPS_DIR "${_st_default_deps_dir}" CACHE PATH
    "Directory containing the Tesseract, Leptonica, and Hunspell dependencies")
unset(_st_default_deps_dir)

find_package(Qt5 5.12 REQUIRED COMPONENTS
  Core
  Gui
  LinguistTools
  Network
  WebChannel
  WebEngineWidgets
  Widgets
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(Qt5 5.12 REQUIRED COMPONENTS X11Extras)
  find_package(X11 REQUIRED)
endif()

# The project's CI installs these libraries into ST_DEPS_DIR.  Using find_path
# and find_library also permits normal system installations and package-manager
# toolchains to work without requiring a particular package-config format.
find_path(TESSERACT_INCLUDE_DIR
  NAMES tesseract/baseapi.h
  HINTS "${ST_DEPS_DIR}"
  PATH_SUFFIXES include
  REQUIRED
)
find_library(TESSERACT_LIBRARY
  NAMES tesseract libtesseract
  HINTS "${ST_DEPS_DIR}"
  PATH_SUFFIXES lib lib64
  REQUIRED
)
find_path(LEPTONICA_INCLUDE_DIR
  NAMES leptonica/allheaders.h
  HINTS "${ST_DEPS_DIR}"
  PATH_SUFFIXES include
  REQUIRED
)
find_library(LEPTONICA_LIBRARY
  NAMES leptonica lept libleptonica liblept
  HINTS "${ST_DEPS_DIR}"
  PATH_SUFFIXES lib lib64
  REQUIRED
)
find_path(HUNSPELL_INCLUDE_DIR
  NAMES hunspell/hunspell.hxx
  HINTS "${ST_DEPS_DIR}"
  PATH_SUFFIXES include
  REQUIRED
)
find_library(HUNSPELL_LIBRARY
  NAMES hunspell hunspell-1.7 hunspell-1.6 libhunspell
  HINTS "${ST_DEPS_DIR}"
  PATH_SUFFIXES lib lib64
  REQUIRED
)

add_library(Tesseract::Tesseract UNKNOWN IMPORTED)
set_target_properties(Tesseract::Tesseract PROPERTIES
  IMPORTED_LOCATION "${TESSERACT_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${TESSERACT_INCLUDE_DIR}"
)
add_library(Leptonica::Leptonica UNKNOWN IMPORTED)
set_target_properties(Leptonica::Leptonica PROPERTIES
  IMPORTED_LOCATION "${LEPTONICA_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${LEPTONICA_INCLUDE_DIR}"
)
add_library(Hunspell::Hunspell UNKNOWN IMPORTED)
set_target_properties(Hunspell::Hunspell PROPERTIES
  IMPORTED_LOCATION "${HUNSPELL_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${HUNSPELL_INCLUDE_DIR}"
)

set(SCREEN_TRANSLATOR_SOURCES
  external/miniz/miniz.c
  src/capture/capturearea.cpp
  src/capture/capturearea.h
  src/capture/captureareaeditor.cpp
  src/capture/captureareaeditor.h
  src/capture/captureareaselector.cpp
  src/capture/captureareaselector.h
  src/capture/capturer.cpp
  src/capture/capturer.h
  src/commonmodels.cpp
  src/commonmodels.h
  src/correct/corrector.cpp
  src/correct/corrector.h
  src/correct/correctorworker.cpp
  src/correct/correctorworker.h
  src/correct/hunspellcorrector.cpp
  src/correct/hunspellcorrector.h
  src/languagecodes.cpp
  src/languagecodes.h
  src/main.cpp
  src/manager.cpp
  src/manager.h
  src/ocr/recognizer.cpp
  src/ocr/recognizer.h
  src/ocr/recognizerworker.cpp
  src/ocr/recognizerworker.h
  src/ocr/tesseract.cpp
  src/ocr/tesseract.h
  src/represent/representer.cpp
  src/represent/representer.h
  src/represent/resulteditor.cpp
  src/represent/resulteditor.h
  src/represent/resultwidget.cpp
  src/represent/resultwidget.h
  src/service/apptranslator.cpp
  src/service/apptranslator.h
  src/service/debug.cpp
  src/service/debug.h
  src/service/geometryutils.cpp
  src/service/geometryutils.h
  src/service/globalaction.cpp
  src/service/globalaction.h
  src/service/keysequenceedit.cpp
  src/service/keysequenceedit.h
  src/service/runatsystemstart.cpp
  src/service/runatsystemstart.h
  src/service/singleapplication.cpp
  src/service/singleapplication.h
  src/service/updates.cpp
  src/service/updates.h
  src/service/widgetstate.cpp
  src/service/widgetstate.h
  src/settings.cpp
  src/settings.h
  src/settingseditor.cpp
  src/settingseditor.h
  src/settingseditor.ui
  src/settingsvalidator.cpp
  src/settingsvalidator.h
  src/stfwd.h
  src/substitutionstable.cpp
  src/substitutionstable.h
  src/task.h
  src/translate/translator.cpp
  src/translate/translator.h
  src/translate/webpage.cpp
  src/translate/webpage.h
  src/translate/webpageproxy.cpp
  src/translate/webpageproxy.h
  src/trayicon.cpp
  src/trayicon.h
)

set(SCREEN_TRANSLATOR_TRANSLATIONS
  share/translations/screentranslator_ru.ts
  share/translations/screentranslator_he.ts
)
qt5_add_translation(SCREEN_TRANSLATOR_QM_FILES
  ${SCREEN_TRANSLATOR_TRANSLATIONS}
)

# The original qrc refers to generated .qm files in the source tree.  Generate
# an equivalent qrc in the build tree so out-of-source builds stay clean.
set(SCREEN_TRANSLATOR_QRC "${CMAKE_CURRENT_BINARY_DIR}/screen-translator.qrc")
file(WRITE "${SCREEN_TRANSLATOR_QRC}" "<RCC>\n  <qresource prefix=\"/icons\">\n")
foreach(_resource IN ITEMS
    "app.png|share/images/STIconBlue.png"
    "st_success.png|share/images/STIconGreen.png"
    "st_busy.png|share/images/STIconOrange.png"
    "st_error.png|share/images/STIconRed.png"
    "loadImages.png|share/images/loadImages.png"
    "loadImages@2x.png|share/images/loadImages@2x.png"
    "debug.png|share/images/debug.png"
    "debug@2x.png|share/images/debug@2x.png")
  string(REPLACE "|" ";" _resource_parts "${_resource}")
  list(GET _resource_parts 0 _resource_alias)
  list(GET _resource_parts 1 _resource_path)
  file(APPEND "${SCREEN_TRANSLATOR_QRC}"
    "    <file alias=\"${_resource_alias}\">${CMAKE_CURRENT_SOURCE_DIR}/${_resource_path}</file>\n")
endforeach()
file(APPEND "${SCREEN_TRANSLATOR_QRC}" "  </qresource>\n  <qresource prefix=\"/translations\">\n")
foreach(_qm_file IN LISTS SCREEN_TRANSLATOR_QM_FILES)
  get_filename_component(_qm_name "${_qm_file}" NAME)
  file(APPEND "${SCREEN_TRANSLATOR_QRC}"
    "    <file alias=\"${_qm_name}\">${_qm_file}</file>\n")
endforeach()
file(APPEND "${SCREEN_TRANSLATOR_QRC}" "  </qresource>\n</RCC>\n")
qt5_add_resources(SCREEN_TRANSLATOR_RESOURCES "${SCREEN_TRANSLATOR_QRC}")

if(WIN32)
  file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/share/images/icon.ico" _st_icon)
  set(SCREEN_TRANSLATOR_RC "${CMAKE_CURRENT_BINARY_DIR}/screen-translator.rc")
  file(WRITE "${SCREEN_TRANSLATOR_RC}" "IDI_ICON1 ICON \"${_st_icon}\"\n")
  list(APPEND SCREEN_TRANSLATOR_SOURCES "${SCREEN_TRANSLATOR_RC}")
endif()

add_executable(screen-translator
  ${SCREEN_TRANSLATOR_SOURCES}
  ${SCREEN_TRANSLATOR_QM_FILES}
  ${SCREEN_TRANSLATOR_RESOURCES}
)
target_compile_definitions(screen-translator PRIVATE VERSION="${PROJECT_VERSION}")
target_include_directories(screen-translator PRIVATE
  external
  src
  src/capture
  src/correct
  src/ocr
  src/represent
  src/service
  src/translate
)
target_link_libraries(screen-translator PRIVATE
  Hunspell::Hunspell
  Leptonica::Leptonica
  Tesseract::Tesseract
  Qt5::Core
  Qt5::Gui
  Qt5::Network
  Qt5::WebChannel
  Qt5::WebEngineWidgets
  Qt5::Widgets
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(screen-translator PRIVATE Qt5::X11Extras X11::X11)
elseif(WIN32)
  target_link_libraries(screen-translator PRIVATE user32)
elseif(APPLE)
  find_library(CARBON_FRAMEWORK Carbon REQUIRED)
  target_link_libraries(screen-translator PRIVATE "${CARBON_FRAMEWORK}")
endif()

install(TARGETS screen-translator
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  install(FILES share/screentranslator.desktop
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
  install(FILES share/images/screentranslator.png
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps")
endif()

if(BUILD_TESTING)
  find_package(Qt5 5.12 REQUIRED COMPONENTS Test)

  add_executable(screen-translator-tests
    external/gtest/gtest-all.cc
    external/miniz/miniz.c
    src/service/debug.cpp
    src/service/debug.h
    src/service/geometryutils.cpp
    src/service/geometryutils.h
    src/service/updates.cpp
    src/service/updates.h
    tests/geometryutils_test.cpp
    tests/main.cpp
    tests/updates_test.cpp
  )
  target_include_directories(screen-translator-tests PRIVATE
    external
    src/service
  )
  target_link_libraries(screen-translator-tests PRIVATE
    Qt5::Core
    Qt5::Network
    Qt5::Test
    Qt5::Widgets
  )

  add_test(NAME screen-translator-tests COMMAND screen-translator-tests)
endif()
