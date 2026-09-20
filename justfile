default:
	just local

local:
	cmake --preset local
	cmake --build --preset local
	just clangd-link

debug:
	cmake --preset debug
	cmake --build --preset debug
	just clangd-link .cmake/debug

clangd-link DIR='.cmake/release/local':
	ln -sfn {{DIR}}/compile_commands.json compile_commands.json

bleeding:
	docker compose up --build bleeding
normal:
	docker compose up --build normal
stable:
	docker compose up --build stable
musl:
	docker compose up --build musl
all:
	just bleeding normal stable musl

clean:
	rm -rf .cmake/ compile_commands.json compiled/
