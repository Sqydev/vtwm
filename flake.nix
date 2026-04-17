{
  description = "vtwm nixos installer";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};

    vtwm = pkgs.stdenv.mkDerivation {
      pname = "vtwm";
      version = "0.1.0";
      src = ./.;

      buildInputs = with pkgs; [
        libX11
        libXinerama
      ];

      nativeBuildInputs = with pkgs; [
        pkg-config
        gnumake
      ];

      buildPhase = ''
        make build
      '';

		installPhase = ''
		  make install PREFIX=$out DESTDIR=

		  mkdir -p $out/share/xsessions
		  cp vtwm.desktop $out/share/xsessions/vtwm.desktop
		'';
    };
  in
  {
    packages.${system}.default = vtwm;
  };
}
