cmake_minimum_required(VERSION 3.21)

project(ScreenTranslator VERSION 4.0.0 LANGUAGES C CXX)

include(CTest)
include(GNUInstallDirs)

option(SCREEN_TRANSLATOR_BUILD_LEGACY
       "Compile the dormant Qt 6 port of the version 3 application" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS
  Concurrent
  Core
  DBus
  Gui
  Network
  WebChannel
  WebEngineCore
  Widgets
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Tesseract REQUIRED IMPORTED_TARGET tesseract>=5.2)
pkg_check_modules(Leptonica REQUIRED IMPORTED_TARGET lept>=1.82)
pkg_get_variable(SCREEN_TRANSLATOR_TESSDATA_DIR tesseract tessdata)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)
endif()

set(SCREEN_TRANSLATOR_V4_SOURCES
  resources.qrc
  src/languagecodes.cpp
  src/languagecodes.h
  src/ocr/tesseract.cpp
  src/ocr/tesseract.h
  src/service/debug.cpp
  src/service/debug.h
  src/service/globalaction.cpp
  src/service/globalaction.h
  src/stfwd.h
  src/task.h
  src/v4/appcontroller.cpp
  src/v4/appcontroller.h
  src/v4/appsettings.cpp
  src/v4/appsettings.h
  src/v4/capturebackend.cpp
  src/v4/capturebackend.h
  src/v4/main.cpp
  src/v4/ocrservice.cpp
  src/v4/ocrservice.h
  src/v4/providerregistry.cpp
  src/v4/providerregistry.h
  src/v4/regionselector.cpp
  src/v4/regionselector.h
  src/v4/resultcard.cpp
  src/v4/resultcard.h
  src/v4/settingsdialog.cpp
  src/v4/settingsdialog.h
  src/v4/shortcutservice.cpp
  src/v4/shortcutservice.h
  src/v4/translationservice.cpp
  src/v4/translationservice.h
)

add_executable(screen-translator ${SCREEN_TRANSLATOR_V4_SOURCES})
target_compile_definitions(screen-translator PRIVATE
  SCREEN_TRANSLATOR_VERSION="${PROJECT_VERSION}"
)
if(SCREEN_TRANSLATOR_TESSDATA_DIR)
  target_compile_definitions(screen-translator PRIVATE
    ST_DEFAULT_TESSDATA_DIR="${SCREEN_TRANSLATOR_TESSDATA_DIR}"
  )
endif()
target_include_directories(screen-translator PRIVATE
  src
  src/ocr
  src/service
  src/v4
)
target_link_libraries(screen-translator PRIVATE
  PkgConfig::Leptonica
  PkgConfig::Tesseract
  Qt6::Concurrent
  Qt6::Core
  Qt6::DBus
  Qt6::Gui
  Qt6::Network
  Qt6::WebChannel
  Qt6::WebEngineCore
  Qt6::Widgets
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(screen-translator PRIVATE X11::X11)
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
  install(FILES share/io.github.OneMoreGres.ScreenTranslator.desktop
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
  install(FILES share/io.github.OneMoreGres.ScreenTranslator.metainfo.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")
  install(FILES share/images/screentranslator.png
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps")
endif()

if(BUILD_TESTING)
  find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
  add_executable(screen-translator-tests
    resources.qrc
    src/languagecodes.cpp
    src/languagecodes.h
    src/ocr/tesseract.cpp
    src/ocr/tesseract.h
    src/service/debug.cpp
    src/service/debug.h
    src/stfwd.h
    src/task.h
    src/v4/appsettings.cpp
    src/v4/appsettings.h
    src/v4/ocrservice.cpp
    src/v4/ocrservice.h
    src/v4/providerregistry.cpp
    src/v4/providerregistry.h
    tests/v4_test.cpp
  )
  target_include_directories(screen-translator-tests PRIVATE
    src
    src/ocr
    src/service
    src/v4
  )
  target_link_libraries(screen-translator-tests PRIVATE
    PkgConfig::Leptonica
    PkgConfig::Tesseract
    Qt6::Concurrent
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    Qt6::Widgets
  )
  add_test(NAME screen-translator-tests COMMAND screen-translator-tests)
endif()

if(SCREEN_TRANSLATOR_BUILD_LEGACY)
  find_package(Qt6 6.8 REQUIRED COMPONENTS Core5Compat WebEngineWidgets)
  find_package(miniz 3.1 CONFIG REQUIRED)
  pkg_check_modules(Hunspell REQUIRED IMPORTED_TARGET hunspell)

  set(SCREEN_TRANSLATOR_LEGACY_SOURCES
    resources.qrc
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
  add_executable(screen-translator-legacy ${SCREEN_TRANSLATOR_LEGACY_SOURCES})
  target_compile_definitions(screen-translator-legacy PRIVATE
    VERSION="${PROJECT_VERSION}"
  )
  target_include_directories(screen-translator-legacy PRIVATE
    src
    src/capture
    src/correct
    src/ocr
    src/represent
    src/service
    src/translate
  )
  target_link_libraries(screen-translator-legacy PRIVATE
    PkgConfig::Hunspell
    PkgConfig::Leptonica
    PkgConfig::Tesseract
    Qt6::Core
    Qt6::Core5Compat
    Qt6::Gui
    Qt6::Network
    Qt6::WebChannel
    Qt6::WebEngineWidgets
    Qt6::Widgets
    miniz::miniz
  )
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(screen-translator-legacy PRIVATE X11::X11)
  elseif(WIN32)
    target_link_libraries(screen-translator-legacy PRIVATE user32)
  elseif(APPLE)
    target_link_libraries(screen-translator-legacy PRIVATE "${CARBON_FRAMEWORK}")
  endif()

  if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(screen-translator-legacy-tests
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
    target_include_directories(screen-translator-legacy-tests PRIVATE
      src/service
    )
    target_link_libraries(screen-translator-legacy-tests PRIVATE
      GTest::gtest
      Qt6::Core
      Qt6::Network
      Qt6::Test
      Qt6::Widgets
      miniz::miniz
    )
    add_test(NAME screen-translator-legacy-tests
             COMMAND screen-translator-legacy-tests)
  endif()
endif()
