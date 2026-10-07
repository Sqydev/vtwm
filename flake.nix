{
	description = "vtwm - a Wayland window manager built on wlroots";

	inputs = {
		nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
	};

	outputs = { self, nixpkgs }:
		let
			systems = [ "x86_64-linux" "aarch64-linux" ];

			forAllSystems = nixpkgs.lib.genAttrs systems;

			pkgsFor = system: nixpkgs.legacyPackages.${system};
		in
		{
			devShells = forAllSystems (system:
				let
					pkgs = pkgsFor system;

					libinputZ = pkgs.libinput.override {
						udev = pkgs.libudev-zero;
						wacomSupport = false;
					};

					wlrootsZ = pkgs.wlroots.override {
						libinput = libinputZ;
					};

					wlrootsInclude = "${wlrootsZ}/include/wlroots-${pkgs.lib.concatStringsSep "." (pkgs.lib.take 2 (pkgs.lib.splitString "." wlrootsZ.version))}";

					envIncludes = builtins.concatStringsSep ":" [
						"${pkgs.wayland.dev}/include"
						wlrootsInclude
						"${pkgs.libxkbcommon.dev}/include"
						"${pkgs.pixman}/include/pixman-1"
						"${libinputZ.dev}/include"
						"${pkgs.glibc.dev}/include"
						"${pkgs.wayland-protocols}/include"
						"${pkgs.libudev-zero}/include"
					];
				in
				{
					default = pkgs.mkShell {
						packages = with pkgs; [
							gcc
							clang-tools
							cmake
							just
							gnumake
							pkg-config
							wayland
							wayland-protocols
							wlrootsZ
							libxkbcommon
							pixman
							libinputZ
							fish
							libudev-zero
						];

						shellHook = "exec fish";

						C_INCLUDE_PATH = envIncludes;
						CPLUS_INCLUDE_PATH = envIncludes;
						CPATH = envIncludes;
					};
				});
		};
}
