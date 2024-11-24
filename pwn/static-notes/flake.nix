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
      in
      {
        devShell = with pkgs; mkShell {
          buildInputs = [ zigpkgs.master ];
        };
      });
}
