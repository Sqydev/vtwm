cmake_minimum_required(VERSION 3.20)

project(vtwm
	VERSION 0.0.1
	LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VTWM_DEBUG
	"Enable vtwm debug configuration"
	OFF
)


set(
	VTWM_OUTPUT_DIR
	"${PROJECT_SOURCE_DIR}/compiled"
	CACHE PATH
	"Directory where the final vtwm binary is placed"
)


find_package(PkgConfig REQUIRED)

pkg_check_modules(WLROOTS
	QUIET IMPORTED_TARGET
	wlroots-0.20
)

if(NOT WLROOTS_FOUND)
	pkg_check_modules(WLROOTS
		QUIET IMPORTED_TARGET
		wlroots-0.19
	)
endif()

if(NOT WLROOTS_FOUND)
	pkg_check_modules(WLROOTS
		QUIET IMPORTED_TARGET
		wlroots
	)
endif()

if(NOT WLROOTS_FOUND)
	message(FATAL_ERROR
		"wlroots not found in the pkg-config search path"
	)
endif()

pkg_check_modules(WAYLAND
	REQUIRED IMPORTED_TARGET
	wayland-server
	wayland-client
)

pkg_check_modules(XKBCOMMON
	REQUIRED IMPORTED_TARGET
	xkbcommon
)

pkg_check_modules(PIXMAN
	REQUIRED IMPORTED_TARGET
	pixman-1
)

pkg_check_modules(LIBINPUT
	REQUIRED IMPORTED_TARGET
	libinput
)


file(GLOB_RECURSE VTWM_SOURCES
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.c"
)

if(NOT VTWM_SOURCES)
	message(FATAL_ERROR
		"No vtwm sources found"
	)
endif()


add_executable(vtwm
	${VTWM_SOURCES}
)

set_target_properties(vtwm PROPERTIES

	OUTPUT_NAME vtwm

	RUNTIME_OUTPUT_DIRECTORY
		"${VTWM_OUTPUT_DIR}"
)


target_include_directories(vtwm
	PRIVATE
		"${PROJECT_SOURCE_DIR}/src"
)


target_compile_options(vtwm
	PRIVATE
		-Wall
		-Wextra
		-Werror
		-Wformat=2
		-Wshadow
		-Wstrict-prototypes
)

if(NOT WIN32)
	target_compile_definitions(vtwm
		PRIVATE
			_POSIX_C_SOURCE=200809L
	)
endif()


if(NOT VTWM_DEBUG)
	target_compile_options(vtwm
		PRIVATE
			-O2
	)

	target_compile_definitions(vtwm
		PRIVATE
			NDEBUG
	)
else()
	target_compile_options(vtwm
		PRIVATE
			-Og
			-g3
			-fno-omit-frame-pointer
			-fsanitize=address,undefined
	)

	target_link_options(vtwm
		PRIVATE
			-fsanitize=address,undefined
			-rdynamic
	)
endif()


target_link_libraries(vtwm
	PRIVATE
		PkgConfig::WLROOTS
		PkgConfig::WAYLAND
		PkgConfig::XKBCOMMON
		PkgConfig::PIXMAN
		PkgConfig::LIBINPUT
)

if(UNIX AND NOT APPLE)
	target_link_libraries(vtwm
		PRIVATE
			m
			${CMAKE_DL_LIBS}
	)
endif()