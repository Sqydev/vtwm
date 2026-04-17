{
  description = "install vtwm for nixos";

  outputs = { self, nixpkgs }: {
    packages.x86_64-linux.vtwm =
      nixpkgs.legacyPackages.x86_64-linux.stdenv.mkDerivation {
        name = "vtwm";

        src = ./.;

        buildPhase = ''
          make
        '';

        installPhase = ''
          make install PREFIX=$out
        '';
      };
  };
}
