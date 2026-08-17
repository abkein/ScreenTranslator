# Screen Translator

Screen Translator 4 is a tray-first Linux application for capturing text from
the screen, recognizing it with Tesseract, and optionally translating it with
JavaScript providers. Version 4 is a focused Qt 6 rewrite with a single fresh
area capture and a compact floating result card.

## Features

- Qt 6.8 LTS baseline and CMake-only build
- Native single-screen area selection on X11
- Screenshot portal area picker on Wayland
- Tesseract OCR using language data installed on the system
- Ordered fallback across bundled or user-provided translation scripts
- Floating result card with recognized text, translation, copy, retry, and
  close actions
- Native X11 global shortcut and the Global Shortcuts portal on Wayland

The version 3 interface and its advanced capture/correction/update features are
not part of the version 4 runtime. They remain available as a compile-tested,
opt-in legacy target while the new product stabilizes.

## Installation

Linux x86-64 AppImages are produced by GitHub Actions. Download an AppImage
from the Releases page, make it executable, and run it:

```sh
chmod +x ScreenTranslator-*.AppImage
./ScreenTranslator-*.AppImage
```

Install at least one Tesseract language package through your distribution. For
example, Debian and Ubuntu package English as `tesseract-ocr-eng`. The AppImage
uses system language data intentionally; it does not download or bundle OCR
models.

Windows and macOS releases are not currently produced or validated.

## First launch

The application opens a required setup dialog on first launch. Choose:

1. a Tesseract language-data directory, or leave the field empty for automatic
   discovery;
2. an installed OCR source language;
3. whether to translate the result;
4. a target language and one or more translation providers when translation is
   enabled.

On X11, the shortcut is registered directly. On Wayland, the desktop portal
owns the shortcut and may show its own permission or assignment dialog. Portal
behavior depends on the desktop environment; capture and settings always remain
available from the tray menu.

Settings are deliberately fresh for version 4 and are not imported from older
Screen Translator releases.

## Translation providers

The existing Baidu, Bing, DeepL, Google, Google API, Papago, and Yandex scripts
are bundled. These scripts automate third-party websites and can stop working
when those sites change. Providers are tried in the order shown in Settings.

The Settings dialog can open the per-user provider directory. A user script
must have a unique `.js` filename; user scripts cannot replace a bundled name.
It must expose an `init()` function, connect to `proxy.translate`, and finish by
calling either `proxy.setTranslated(text)` or `proxy.setFailed(error)`.

## Building from source

Required development dependencies:

- CMake 3.21 or newer and a C++17 compiler
- Qt 6.8 or newer with Core, Concurrent, DBus, Gui, Network, WebChannel,
  WebEngineCore, Widgets, and Test
- Tesseract 5.2 or newer and Leptonica 1.82 or newer
- X11 development libraries on Linux

Configure, build, and test the version 4 application:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Install with the usual CMake mechanism:

```sh
cmake --install build --prefix /usr/local
```

To compile the dormant version 3 application and run its regression tests,
also install Qt Core5Compat, Qt WebEngineWidgets, Hunspell, miniz 3.1 or newer,
and GoogleTest, then configure with:

```sh
cmake -S . -B build-legacy -G Ninja \
  -DSCREEN_TRANSLATOR_BUILD_LEGACY=ON
cmake --build build-legacy
ctest --test-dir build-legacy --output-on-failure
```

The resulting compatibility executable is `screen-translator-legacy`; it is a
development target and is not installed or released.

### Nix

The flake follows the `nixpkgs` input from the local system flake and provides
the application, an app entry, checks, an overlay, and a development shell:

```sh
nix build
nix run
nix flake check
nix develop
```

The default Nix package includes English Tesseract data. The standalone
[`package.nix`](package.nix) expression accepts `tesseractLanguages`, so Nix
consumers can select additional models without bundling them into the
application itself. Set `buildLegacy = true` in the same call to compile and
test the compatibility target with packaged GoogleTest and miniz:

```nix
pkgs.callPackage ./package.nix {
  buildLegacy = true;
  tesseractLanguages = [ "eng" "deu" "rus" ];
}
```

## Current limitations

- Capture is one fresh rectangular region at a time.
- Wayland requires working Screenshot and Global Shortcuts portals for the full
  experience.
- The redesigned interface is English-only.
- Translation providers need network access and are not guaranteed by their
  respective services.

## License and attribution

Screen Translator is distributed under the MIT License. See [LICENSE.md](LICENSE.md).
The application icons include work by Smashicons and Freepik from Flaticon.
