{
  description = "vtwm nixos installer";
  # NOTE: for the lack of better option. to upgrade type:
  # sudo nixos-rebuild switch --option tarball-ttl 0
  #    To install on system add to your config.nix:
  # let
  # 	vtwm = builtins.getFlake "git+https://github.com/Sqydev/vtwm.git";
  # in
  #   And in environment.systemPackages = with pkgs; [
  # vtwm.packages.${pkgs.system}.default
  # And like enywhere:
  # services.vtwm.enable = true;

  inputs.nixpkgs.url = "github:NixOS/nixpkgs";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};

    vtwmPkg = pkgs.stdenv.mkDerivation {
      pname = "vtwm";
      version = "0.1.0";
      src = ./.;

      buildInputs = [ pkgs.libX11 pkgs.libXinerama ];
      nativeBuildInputs = [ pkgs.pkg-config pkgs.gnumake ];

      buildPhase = "make build";

      installPhase = ''
        make install PREFIX=$out DESTDIR=
      '';
    };
  in
  {
    packages.${system}.default = vtwmPkg;

    nixosModules.default = { config, lib, pkgs, ... }: {
      options.services.vtwm.enable = lib.mkEnableOption "vtwm window manager";

      config = lib.mkIf config.services.vtwm.enable {
        services.xserver.enable = true;

        services.xserver.displayManager.session = [
          {
            name = "vtwm";
            manage = "window";
            start = "exec ${vtwmPkg}/bin/vtwm";
          }
        ];
        environment.systemPackages = [ vtwmPkg ];
      };
    };
  };
}
