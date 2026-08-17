{ pkgs, ... }: {
  projectRootFile = "flake.nix";

  settings = {
    walk = "git"; # 'auto', 'git', 'jujutsu' or 'filesystem'
    # formatters = ["go" "toml" "haskell"];
    verbose = 1;
  };

  programs = {
    nixfmt = {
      enable = true;
      strict = true;
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
    options = [ ];
    # includes = [ "*.nix" "*.md" "*.toml" ];
    includes = [ "*" ];
    excludes = [
      "*.aml"
      "*.diff"
      "**/Makefile"
    ];
    priority = 1;
  };
}
