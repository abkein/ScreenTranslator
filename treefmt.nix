{ pkgs, ... }: {
  projectRootFile = "flake.nix";

  settings = {
    walk = "git";
    verbose = 1;
  };

  programs = {
    nixfmt = {
      enable = true;
      strict = true;
    };
    clang-format = {
      enable = true;
    };
    clang-tidy = {
      enable = false;
    };
  };

  settings.formatter.nixfmt.options = [ "--verify" ];

  programs.nixf-diagnose = {
    enable = true;
    variableLookup = true;
    ignore = [ ];
  };
  settings.formatter.nixf-diagnose = {
    # Ensure nixfmt cleans up after nixf-diagnose.
    priority = -1;
    excludes = [ ];
  };

  settings.formatter.editorconfig-checker = {
    command = "${pkgs.lib.getExe pkgs.editorconfig-checker}";
    # options = [ ];
    includes = [ "*" ];
    # excludes = [ ];
    priority = 1;
  };
}
