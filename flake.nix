{
  description = "vtwm";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { nixpkgs, ... }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      devShells = forAllSystems (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};

          libinputNoUdev = pkgs.libinput.override {
            udev = pkgs.libudev-zero;
            wacomSupport = false;
          };

          wlrootsNoUdev = pkgs.wlroots.override {
            libinput = libinputNoUdev;
          };
        in
        {
          default = pkgs.mkShell {
            packages = with pkgs; [
              clang-tools
              fish
              gcc
              just
              libinputNoUdev
              libudev-zero
              libxkbcommon
              meson
              ninja
              pixman
              pkg-config
              wayland
              wayland-protocols
              wlrootsNoUdev
            ];

            shellHook = ''
              if [[ $- == *i* ]]; then
                exec ${pkgs.fish}/bin/fish
              fi
            '';
          };
        });

      formatter = forAllSystems (system:
        nixpkgs.legacyPackages.${system}.nixpkgs-fmt);
    };
}
