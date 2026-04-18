{
  description = "vtwm";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};

    vtwmPkg = pkgs.stdenv.mkDerivation {
      pname = "vtwm";
      version = "0.1.0";
      src = ./.;

      nativeBuildInputs = [ pkgs.pkg-config pkgs.gnumake ];
      buildInputs = [ pkgs.libX11 pkgs.libXinerama ];

      buildPhase = "make build";

      installPhase = ''
        make install PREFIX=$out DESTDIR=
      '';
    };
  in
  {
    packages.${system}.default = vtwmPkg;

    nixosModules.default = { config, lib, pkgs, ... }: {
      options.services.vtwm.enable =
        lib.mkEnableOption "vtwm window manager";

      config = lib.mkIf config.services.vtwm.enable {
        services.xserver.enable = true;

        services.xserver.displayManager.session = [
          {
            name = "Vtwm";
            manage = "window";
			start = "${vtwmPkg}/bin/vtwm";
          }
        ];

        environment.systemPackages = [ vtwmPkg ];
      };
    };
  };
}
