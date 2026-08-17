{
  description = "Screen Translator 4 — Qt 6 screen capture, OCR, and translation";

  inputs = {
    system-flake.url = "path:/home/kein/nixos-configuration";
    nixpkgs.follows = "system-flake/nixpkgs";
    flake-utils.follows = "system-flake/flake-utils";

    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      treefmt-nix,
      ...
    }:
    {
      overlays.default = final: _prev: { screen-translator = final.callPackage ./package.nix { }; };
    }
    // (flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        screen-translator-pkg = self.packages.${system}.screen-translator;
        screen-translator-legacy-pkg = screen-translator-pkg.override { buildLegacy = true; };

        treefmtEval = (treefmt-nix.lib.evalModule pkgs ./treefmt.nix).config.build;
      in
      {
        packages = rec {
          default = screen-translator;
          screen-translator = pkgs.callPackage ./package.nix { };
        };

        apps = {
          default = {
            type = "app";
            program = "${screen-translator-pkg}/bin/screen-translator";
            meta.description = "Launch Screen Translator";
          };
        };

        formatter = treefmtEval.wrapper;

        # checks = {
        #   formatting = treefmtEval.check self;
        #   screen-translator = packageFor system;
        # };

        devShells = rec {
          default = build-env;
          build-env = pkgs.mkShell {
            inputsFrom = [ screen-translator-legacy-pkg ];
            packages = [
              pkgs.clang-tools
              pkgs.qt6.qttools
            ];
            TESSDATA_PREFIX = "${screen-translator-pkg.tesseract}/share/tessdata";
          };
        };
      }
    ));
}
