default:
	just build

build:
	meson setup build/release --reconfigure
	meson compile -C build/release
	just clangd-link build/release

vbuild:
	meson setup build/release --reconfigure
	meson compile -C build/release -v
	just clangd-link build/release

debug:
	meson setup build/debug --reconfigure -Dvtwm_debug=true
	meson compile -C build/debug
	just clangd-link build/debug

vdebug:
	meson setup build/debug --reconfigure -Dvtwm_debug=true
	meson compile -C build/debug -v
	just clangd-link build/debug

clangd-link DIR='build/release':
	ln -sfn {{DIR}}/compile_commands.json compile_commands.json

clean:
	rm -rf build/ compile_commands.json compiled/ core .cache/

license-check:
	bash scripts/check_license.sh

license-fix:
	bash scripts/fix_license.sh
