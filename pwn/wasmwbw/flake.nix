{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
    zig.url = "github:mitchellh/zig-overlay";
  };

  outputs = { self, nixpkgs, utils, zig }:
    utils.lib.eachDefaultSystem (system:
      let
        overlays = [(final: prev: {
          zigpkgs = zig.packages.${prev.system};
        })];
        pkgs = import nixpkgs { inherit overlays system; };
        wabt = pkgs.wabt.overrideAttrs (oldAttrs: rec {
          version = "1.0.36";

          src = pkgs.fetchFromGitHub {
            owner = "WebAssembly";
            repo = "wabt";
            rev = version;
            hash = "sha256-CswVvL6yxTf0ju/UPvEGMgTuNZ8mumSIcnVbYVfy178=";
            fetchSubmodules = true;
          };
        });
      in
      {
        devShell = with pkgs; mkShell {
          buildInputs = [ cargo rustc rustPackages.clippy wabt zigpkgs.master ];
          RUST_SRC_PATH = rustPlatform.rustLibSrc;
        };
      });
}
