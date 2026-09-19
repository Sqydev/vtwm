default:
	just local

local:
	cmake --preset local
	cmake --build --preset local

debug:
	cmake --preset debug
	cmake --build --preset debug
	just clangd-link

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
