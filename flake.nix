{
  description = "install vtwm for nixos";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};

    vtwm = pkgs.stdenv.mkDerivation {
      pname = "vtwm";
      version = "0.1.0"; # Good practice to include a version
      src = ./.;

      # If your Makefile uses a variable for the session path, override it here.
      # If it's hardcoded, we use postPatch to fix it globally.
      postPatch = ''
        substituteInPlace makefile \
          --replace "/usr" "$out"
      '';

      # Using makeFlags is cleaner than overriding phases manually
      makeFlags = [ "PREFIX=$(out)" ];

      # Note: buildPhase and installPhase are usually handled automatically 
      # by stdenv if you have a standard Makefile. 
      # Only override them if your Makefile requires non-standard arguments.
      installPhase = ''
        runHook preInstall
        make install PREFIX=$out
        runHook postInstall
      '';

      nativeBuildInputs = with pkgs; [
        # Add any build tools here (e.g., pkg-config)
      ];

      buildInputs = with pkgs; [
        xorg.libX11
        xorg.libXinerama
        # Add other X11 dependencies here
      ];
    };
  in {
    packages.${system} = {
      vtwm = vtwm;
      default = vtwm;
    };
  };
}
