{
    #inputs.nixpkgs.url = "github:NixOS/nixpkgs/f0636c60556efb4c0330ba69a0ecdce4a6f60b93";
    inputs.nixpkgs.url = "github:NixOS/nixpkgs";

    outputs = {nixpkgs, ...}: 
        let
            pkgs = nixpkgs.legacyPackages.x86_64-linux;
        in {
            devShells.x86_64-linux.default = pkgs.mkShell {
                nativeBuildInputs = [
                    pkgs.cmakeCurses
                    pkgs.gcc
                    pkgs.pkg-config
                ];
                buildInputs = [
                    pkgs.imagemagick6
                    pkgs.libseccomp
                ];
                packages = [
                    pkgs.cmake
                ];
            };
        };
}
