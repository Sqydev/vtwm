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
				let pkgs = pkgsFor system;
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
						wlroots
						libxkbcommon
						pixman
						libinput
						];
					};
				});
		};
}