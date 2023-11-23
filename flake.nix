{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils,  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in
    with pkgs;
    {
      devShells.default = pkgs.mkShell {
        packages = [
          gcc13
          SDL2
          SDL2_ttf
          pkg-config
        ];
      };
    });
}
