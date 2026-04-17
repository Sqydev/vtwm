{
  description = "install vtwm for nixos";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};

    vtwm = pkgs.stdenv.mkDerivation {
      name = "vtwm";
      src = ./.;

      buildPhase = ''
        make
      '';

      installPhase = ''
        make install PREFIX=$out
      '';
    };
  in {
    packages.${system}.vtwm = vtwm;
    packages.${system}.default = vtwm;
  };
}
