project(
	'vtwm',
	'c',
	version: '0.0.1',
	meson_version: '>=1.1.0',
	default_options: [
		'c_std=c99',
		'warning_level=2',
		'werror=true',
	],
)

cc = meson.get_compiler('c')

deps = [
	dependency('wlroots-0.20'),
	dependency('wayland-server'),
	dependency('wayland-client'),
	dependency('xkbcommon'),
	dependency('pixman-1'),
	dependency('libinput'),
	cc.find_library('m', required: false),
]

add_project_arguments(
	cc.get_supported_arguments([
		'-Wformat=2',
		'-Wshadow',
		'-Wstrict-prototypes',
	]),
	language: 'c',
)

add_project_arguments(
	'-D_POSIX_C_SOURCE=200809L',
	'-DWLR_USE_UNSTABLE',
	language: 'c',
)

vtwm_sources = []

find = find_program('find', required: false)
if find.found()
	discovered = run_command(
		find,
		meson.current_source_dir() / 'src',
		'-type', 'f',
		'-name', '*.c',
		'-printf', 'src/%P\n',
		check: true,
	).stdout().strip().split('\n')

	foreach source : discovered
		if source != ''
			vtwm_sources += files(source)
		endif
	endforeach
else
	vtwm_sources += files('src/main.c')
endif

if vtwm_sources.length() == 0
	error('no vtwm sources found')
endif

vtwm_c_args = []
vtwm_link_args = []

assets_dir = get_option('vtwm_assets_dir')
if assets_dir != ''
	vtwm_c_args += [ '-DVTWM_ASSETS_DIR="@0@"'.format(assets_dir) ]
endif

if get_option('vtwm_debug')
	vtwm_c_args += [
		'-Og',
		'-g3',
		'-fno-omit-frame-pointer',
		'-fsanitize=undefined,leak',
		'-fno-sanitize-recover=all',
	]
	vtwm_link_args += [
		'-fsanitize=undefined,leak',
		'-rdynamic',
	]
else
	vtwm_c_args += [ '-O2', '-DNDEBUG' ]
endif

executable(
	'vtwm',
	vtwm_sources,
	include_directories: include_directories('src'),
	dependencies: deps,
	c_args: vtwm_c_args,
	link_args: vtwm_link_args,
	install: true,
)
