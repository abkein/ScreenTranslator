{
  lib,
  stdenv,
  cmake,
  curl,
  gtest,
  hunspell,
  ninja,
  pkg-config,
  qt6,
  tesseract5,
  leptonica,
  libarchive,
  libX11,
  miniz,
  buildLegacy ? false,
  tesseractLanguages ? [ "eng" ],
}:

let
  tesseract = tesseract5.override { enableLanguages = tesseractLanguages; };
in
stdenv.mkDerivation (finalAttrs: {
  pname = "screen-translator";
  version = "4.0.0";

  src = lib.fileset.toSource {
    root = ./.;
    fileset = lib.fileset.unions [
      ./CMakeLists.txt
      ./resources.qrc
      ./share/images
      ./share/io.github.OneMoreGres.ScreenTranslator.desktop
      ./share/io.github.OneMoreGres.ScreenTranslator.metainfo.xml
      ./src
      ./tests
      ./translators
    ];
  };

  strictDeps = true;

  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
    qt6.wrapQtAppsHook
  ];

  buildInputs = [
    curl
    leptonica
    libarchive
    libX11
    qt6.qtbase
    qt6.qtwayland
    qt6.qtwebchannel
    qt6.qtwebengine
    tesseract
  ]
  ++ lib.optionals buildLegacy [
    gtest
    hunspell
    miniz
    qt6.qt5compat
  ];

  cmakeFlags = [
    (lib.cmakeBool "BUILD_TESTING" true)
    (lib.cmakeBool "SCREEN_TRANSLATOR_BUILD_LEGACY" buildLegacy)
  ];

  doCheck = true;

  preCheck = ''
    export QT_QPA_PLATFORM=offscreen
  '';

  qtWrapperArgs = [ "--set-default TESSDATA_PREFIX ${tesseract}/share/tessdata" ];

  doInstallCheck = true;
  installCheckPhase = ''
    runHook preInstallCheck

    versionOutput=$(QT_QPA_PLATFORM=offscreen "$out/bin/screen-translator" --version)
    test "$versionOutput" = "ScreenTranslator ${finalAttrs.version}"
    test -e "$out/share/applications/io.github.OneMoreGres.ScreenTranslator.desktop"
    test -e "$out/share/metainfo/io.github.OneMoreGres.ScreenTranslator.metainfo.xml"

    runHook postInstallCheck
  '';

  passthru = {
    inherit buildLegacy tesseract tesseractLanguages;
  };

  meta = {
    description = "Capture, recognize, and translate text on screen";
    homepage = "https://github.com/OneMoreGres/ScreenTranslator";
    license = lib.licenses.mit;
    mainProgram = "screen-translator";
    platforms = lib.platforms.linux;
  };
})
