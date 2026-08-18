# Screen Translator 3 to 4 transition

This document inventories features that existed in Screen Translator 3 but are
absent, reduced, or unsupported in the default Screen Translator 4 runtime.
Most of the version 3 implementation remains in the repository and can be
compiled with `SCREEN_TRANSLATOR_BUILD_LEGACY=ON`; the resulting
`screen-translator-legacy` executable is a development target and is not
installed or released.

## Advanced capture workflow

Version 3 allowed several regions to be selected in one session, processing all
selected regions together, retaining locked regions, recapturing saved regions,
repeating capture, and choosing OCR language, translation language, translation
state, and correction state per region. Version 4 accepts one fresh rectangular
selection with application-wide settings.

Corresponding files:

- [src/capture/capturearea.cpp](src/capture/capturearea.cpp)
- [src/capture/capturearea.h](src/capture/capturearea.h)
- [src/capture/captureareaeditor.cpp](src/capture/captureareaeditor.cpp)
- [src/capture/captureareaeditor.h](src/capture/captureareaeditor.h)
- [src/capture/captureareaselector.cpp](src/capture/captureareaselector.cpp)
- [src/capture/captureareaselector.h](src/capture/captureareaselector.h)
- [src/capture/capturer.cpp](src/capture/capturer.cpp)
- [src/capture/capturer.h](src/capture/capturer.h)
- [src/v4/regionselector.cpp](src/v4/regionselector.cpp)
- [src/v4/regionselector.h](src/v4/regionselector.h)

## Combined multi-monitor selection

Version 3 combined screenshots and geometries from every screen into one
selection canvas. On X11, version 4 captures only the screen containing the
cursor. On Wayland, selection is delegated to the desktop screenshot portal.

Corresponding files:

- [src/capture/capturer.cpp](src/capture/capturer.cpp)
- [src/capture/captureareaselector.cpp](src/capture/captureareaselector.cpp)
- [src/v4/capturebackend.cpp](src/v4/capturebackend.cpp)
- [src/v4/capturebackend.h](src/v4/capturebackend.h)

## OCR text correction

The version 3 recognition pipeline could correct OCR output with Hunspell and
with user-defined, language-specific substitutions. Version 4 sends raw
Tesseract output directly to translation and has no correction settings or
substitution editor.

Corresponding files:

- [src/correct/corrector.cpp](src/correct/corrector.cpp)
- [src/correct/corrector.h](src/correct/corrector.h)
- [src/correct/correctorworker.cpp](src/correct/correctorworker.cpp)
- [src/correct/correctorworker.h](src/correct/correctorworker.h)
- [src/correct/hunspellcorrector.cpp](src/correct/hunspellcorrector.cpp)
- [src/correct/hunspellcorrector.h](src/correct/hunspellcorrector.h)
- [src/substitutionstable.cpp](src/substitutionstable.cpp)
- [src/substitutionstable.h](src/substitutionstable.h)
- [src/settings.h](src/settings.h)
- [src/settingseditor.ui](src/settingseditor.ui)

## Rich result presentation

Version 3 could show a result as a tray notification or as one or more floating
windows positioned next to their capture regions. Result windows could display
the captured image, hide or show recognized text, copy the image, and customize
font family, font size, foreground color, and background color. Version 4 uses
one fixed result card containing read-only recognized and translated text.

Corresponding files:

- [src/represent/representer.cpp](src/represent/representer.cpp)
- [src/represent/representer.h](src/represent/representer.h)
- [src/represent/resultwidget.cpp](src/represent/resultwidget.cpp)
- [src/represent/resultwidget.h](src/represent/resultwidget.h)
- [src/settings.cpp](src/settings.cpp)
- [src/settings.h](src/settings.h)
- [src/settingseditor.ui](src/settingseditor.ui)
- [src/v4/resultcard.cpp](src/v4/resultcard.cpp)
- [src/v4/resultcard.h](src/v4/resultcard.h)

## Editable result and reprocessing

Version 3 had a result editor for changing recognized text, choosing different
source and target languages, rerunning OCR, translating edited text, or rerunning
OCR and translation together. Version 4 result text is read-only; only retrying
translation with the current settings is supported.

Corresponding files:

- [src/represent/resulteditor.cpp](src/represent/resulteditor.cpp)
- [src/represent/resulteditor.h](src/represent/resulteditor.h)
- [src/represent/resultwidget.cpp](src/represent/resultwidget.cpp)
- [src/manager.cpp](src/manager.cpp)
- [src/task.h](src/task.h)
- [src/v4/resultcard.cpp](src/v4/resultcard.cpp)
- [src/v4/appcontroller.cpp](src/v4/appcontroller.cpp)

## Multiple global shortcuts

Version 3 exposed separate global shortcuts for new capture, repeat capture,
capturing saved regions, showing the last result, and copying the last result.
Version 4 registers only the capture shortcut globally. Showing the last result
and copying text remain available through the tray menu and result card.

Corresponding files:

- [src/settings.h](src/settings.h)
- [src/settingseditor.cpp](src/settingseditor.cpp)
- [src/settingseditor.ui](src/settingseditor.ui)
- [src/trayicon.cpp](src/trayicon.cpp)
- [src/trayicon.h](src/trayicon.h)
- [src/service/globalaction.cpp](src/service/globalaction.cpp)
- [src/service/globalaction.h](src/service/globalaction.h)
- [src/service/keysequenceedit.cpp](src/service/keysequenceedit.cpp)
- [src/service/keysequenceedit.h](src/service/keysequenceedit.h)
- [src/v4/shortcutservice.cpp](src/v4/shortcutservice.cpp)
- [src/v4/shortcutservice.h](src/v4/shortcutservice.h)

## Integrated resource updates

Version 3 could check for, install, update, and remove application resources,
including Tesseract models, Hunspell dictionaries, and translator scripts. It
also supported periodic update checks and update notifications. Version 4 uses
system-installed Tesseract data, embedded providers, and manually installed user
providers; it contains no updater UI or downloader.

Corresponding files:

- [updates.json](updates.json)
- [src/service/updates.cpp](src/service/updates.cpp)
- [src/service/updates.h](src/service/updates.h)
- [src/settingseditor.cpp](src/settingseditor.cpp)
- [src/settingseditor.ui](src/settingseditor.ui)
- [src/manager.cpp](src/manager.cpp)
- [src/v4/ocrservice.cpp](src/v4/ocrservice.cpp)
- [src/v4/providerregistry.cpp](src/v4/providerregistry.cpp)

## Proxy and TLS controls

Version 3 offered disabled, system, SOCKS5, and HTTP proxy modes, optional proxy
credentials, and an option to accept overridable TLS certificate errors. Version
4 has no proxy configuration UI and rejects translation-provider certificate
errors.

Corresponding files:

- [src/settings.cpp](src/settings.cpp)
- [src/settings.h](src/settings.h)
- [src/settingseditor.cpp](src/settingseditor.cpp)
- [src/settingseditor.ui](src/settingseditor.ui)
- [src/manager.cpp](src/manager.cpp)
- [src/translate/webpage.cpp](src/translate/webpage.cpp)
- [src/translate/webpage.h](src/translate/webpage.h)
- [src/v4/translationservice.cpp](src/v4/translationservice.cpp)

## Desktop and diagnostic preferences

Version 3 supported portable settings and assets, launch at system startup, a
configurable startup notification, and UI-controlled trace-file logging. These
preferences are not exposed by version 4.

Corresponding files:

- [src/settings.cpp](src/settings.cpp)
- [src/settings.h](src/settings.h)
- [src/settingseditor.cpp](src/settingseditor.cpp)
- [src/settingseditor.ui](src/settingseditor.ui)
- [src/service/runatsystemstart.cpp](src/service/runatsystemstart.cpp)
- [src/service/runatsystemstart.h](src/service/runatsystemstart.h)
- [src/service/debug.cpp](src/service/debug.cpp)
- [src/service/debug.h](src/service/debug.h)
- [src/manager.cpp](src/manager.cpp)
- [src/v4/appsettings.h](src/v4/appsettings.h)

## Localized interface

Version 3 loaded Qt translation catalogs at runtime and included Russian and
Hebrew translation sources. Version 4 does not initialize the application
translator, so the redesigned interface is English-only and existing `.qm`
catalogs are not loaded.

Corresponding files:

- [src/service/apptranslator.cpp](src/service/apptranslator.cpp)
- [src/service/apptranslator.h](src/service/apptranslator.h)
- [src/main.cpp](src/main.cpp)
- [src/v4/main.cpp](src/v4/main.cpp)
- [share/translations/screentranslator_he.ts](share/translations/screentranslator_he.ts)
- [share/translations/screentranslator_ru.ts](share/translations/screentranslator_ru.ts)

## Version 3 settings migration

Version 4 uses a fresh settings schema and application identity. It deliberately
does not import version 3 shortcuts, resource locations, correction rules,
provider configuration, appearance, or other preferences.

Corresponding files:

- [src/settings.cpp](src/settings.cpp)
- [src/settings.h](src/settings.h)
- [src/v4/appsettings.cpp](src/v4/appsettings.cpp)
- [src/v4/appsettings.h](src/v4/appsettings.h)
- [src/main.cpp](src/main.cpp)
- [src/v4/main.cpp](src/v4/main.cpp)
- [README.md](README.md)

## Translation-provider diagnostics

Version 3 exposed a translator window with provider tabs, live script logs,
provider page inspection, an image-loading toggle, and Qt WebEngine remote
debugging. Version 4 runs providers in hidden, short-lived pages and has no
equivalent diagnostics UI.

Corresponding files:

- [src/translate/translator.cpp](src/translate/translator.cpp)
- [src/translate/translator.h](src/translate/translator.h)
- [src/translate/webpage.cpp](src/translate/webpage.cpp)
- [src/translate/webpage.h](src/translate/webpage.h)
- [src/translate/webpageproxy.cpp](src/translate/webpageproxy.cpp)
- [src/translate/webpageproxy.h](src/translate/webpageproxy.h)
- [src/trayicon.cpp](src/trayicon.cpp)
- [src/v4/translationservice.cpp](src/v4/translationservice.cpp)

## Batch and queued processing

Multiple selected regions in version 3 produced a batch of tasks. OCR and
correction maintained queues, translation distributed work across available
provider pages, and several result windows could be produced for one generation.
Version 4 permits one active capture, OCR job, and translation at a time.

Corresponding files:

- [src/capture/captureareaselector.cpp](src/capture/captureareaselector.cpp)
- [src/manager.cpp](src/manager.cpp)
- [src/ocr/recognizer.cpp](src/ocr/recognizer.cpp)
- [src/correct/corrector.cpp](src/correct/corrector.cpp)
- [src/translate/translator.cpp](src/translate/translator.cpp)
- [src/represent/representer.cpp](src/represent/representer.cpp)
- [src/v4/appcontroller.cpp](src/v4/appcontroller.cpp)
- [src/v4/ocrservice.cpp](src/v4/ocrservice.cpp)
- [src/v4/translationservice.cpp](src/v4/translationservice.cpp)

## Windows distribution support

Version 3 CI built and published Windows packages in addition to Linux builds.
Version 4 currently builds, tests, and packages only a Linux AppImage. Some
Windows-specific source and deployment helpers still exist, but Windows is not
currently validated or released. macOS was not supported before the rewrite
either.

Corresponding files:

- [.github/workflows/build.yml](.github/workflows/build.yml)
- [CMakeLists.txt](CMakeLists.txt)
- [src/service/globalaction.cpp](src/service/globalaction.cpp)
- [src/service/globalaction.h](src/service/globalaction.h)
- [share/ci/windeploy.py](share/ci/windeploy.py)
- [share/ci/release.py](share/ci/release.py)
- [README.md](README.md)

