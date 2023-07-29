project('momuma-gtk', 'cpp',
	default_options: [
		'b_sanitize=address,undefined',
		'buildtype=debug',
		'c_std=gnu17',
		'cpp_std=gnu++20',
		'optimization=s',
		'warning_level=2',
		'werror=true'
	],
	license : 'GPL-3.0-or-later',
	meson_version: '>= 0.61',
	version: '0.1.1',
)


################################################################################
# Version

version_array = meson.project_version().split('.')
version_major = version_array[0].to_int()
version_minor = version_array[1].to_int()
version_patch = version_array[2].to_int()


################################################################################
# Default paths

sys_prefix = get_option('prefix')
sys_bin_dir = sys_prefix / get_option('bindir')
sys_locale_dir = sys_prefix / get_option('localedir')
sys_data_dir = sys_prefix / get_option('datadir')
source_code_root = meson.current_source_dir()

message(f'System prefix = @sys_prefix@')
message(f'System locale dir = @sys_locale_dir@')
message(f'System data dir = @sys_data_dir@')
message(f'Source code dir = @source_code_root@')


################################################################################
# Options

cxx = meson.get_compiler('cpp')
is_debug_build = (get_option('buildtype') == 'debug')

host_os = host_machine.system()
is_os_android = (host_os == 'android')        # By convention only, subject to change
is_os_cygwin = (host_os == 'cygwin')          # The Cygwin environment for Windows
is_os_darwin = (host_os == 'darwin')          # Either OSX or iOS
is_os_dragonfly = (host_os == 'dragonfly')    # DragonFly BSD
is_os_emscripten = (host_os == 'emscripten')  # Emscripten's JavaScript environment
is_os_freebsd = (host_os == 'freebsd')        # FreeBSD and its derivatives
is_os_gnu = (host_os == 'gnu')                # GNU Hurd
is_os_haiku = (host_os == 'haiku')
is_os_linux = (host_os == 'linux')
is_os_netbsd = (host_os == 'netbsd')
is_os_openbsd = (host_os == 'openbsd')
is_os_sunos = (host_os == 'sunos')            # Illumos and Solaris
is_os_windows = (host_os == 'windows')        # Any version of Windows


message(f'Debug build = @is_debug_build@')
message(f'Operating system = @host_os@')
assert(is_os_linux)

application_id = 'monochrome-sauce.package.momuma-gtk'
if is_debug_build
	application_id += '.debug'
endif


################################################################################
# Config

config_h = configuration_data()
config_h.set('MOMUMA_GTK__VERSION_MAJOR', version_major)
config_h.set('MOMUMA_GTK__VERSION_MINOR', version_minor)
config_h.set('MOMUMA_GTK__VERSION_PATCH', version_patch)
config_h.set_quoted('APPLICATION_ID', application_id)
config_h.set_quoted('APPLICATION_TITLE', 'MoMuMa - Mono\'s Music Manager')
config_h.set_quoted('MOMUMA_GTK__NAME', meson.project_name())
config_h.set_quoted('MOMUMA_GTK__VERSION', meson.project_version())
configure_file(configuration: config_h, output: 'build-config.h')


################################################################################
# Dependencies

libmomuma_proj = subproject('mono-music-manager')

asan_dep = cxx.find_library('asan')
ubsan_dep = cxx.find_library('ubsan')

#adwaita_dep = dependency('libadwaita-1', include_type: 'system', version: '>= 1.1')
giomm_dep = dependency('giomm-2.4', include_type: 'system', version: '>= 2.60')
glibmm_dep = dependency('glibmm-2.4', include_type: 'system', version: '>= 2.60')
gtkmm_dep = dependency('gtkmm-3.0', include_type: 'system', version: '>= 3.24')
momuma_dep = libmomuma_proj.get_variable('momuma_dep')

i18n = import('i18n')

if is_debug_build
	package_locale_dir = meson.current_source_dir() / 'build/po'
	message(f'Package locale dir = @package_locale_dir@')
else
	package_locale_dir = sys_locale_dir
endif


################################################################################
# Compiler flags

extra_flags = [
	'-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
	'-DFMT_USE_CONSTEXPR', # let fmt/spdlog functions be type-checked at compile-time
	'-DGDK_DISABLE_DEPRECATED',
	'-DGDKMM_DISABLE_DEPRECATED',
	'-DGIOMM_DISABLE_DEPRECATED',
	'-DGLIBMM_DISABLE_DEPRECATED',
	'-DGTK_DISABLE_DEPRECATED',
	'-DGTKMM_DISABLE_DEPRECATED',
	'-DPACKAGE_LOCALEDIR="@0@"'.format(package_locale_dir),
	'-Wcast-align',
	'-Wcast-qual',
	'-Wchar-subscripts',
	'-Wconversion',
	'-Wdouble-promotion',
	'-Wformat=2',
	'-Wmissing-declarations',
	'-Wnull-dereference',
	'-Wold-style-cast',
	'-Woverflow',
	'-Wpointer-arith',
	'-Wshadow',
	'-Wsign-compare',
	'-Wundef',
	'-Wunused-result',
	'-Wunused-variable',
]
extra_flags += [
	'-DOS_ANDROID=@0@'.format(is_os_android.to_int()),        # By convention only, subject to change
	'-DOS_CYGWIN=@0@'.format(is_os_cygwin.to_int()),          # The Cygwin environment for Windows
	'-DOS_DARWIN=@0@'.format(is_os_darwin.to_int()),          # Either OSX or iOS
	'-DOS_DRAGONFLY=@0@'.format(is_os_dragonfly.to_int()),    # DragonFly BSD
	'-DOS_EMSCRIPTEN=@0@'.format(is_os_emscripten.to_int()),  # Emscripten's JavaScript environment
	'-DOS_FREEBSD=@0@'.format(is_os_freebsd.to_int()),        # FreeBSD and its derivatives
	'-DOS_GNU=@0@'.format(is_os_gnu.to_int()),                # GNU Hurd
	'-DOS_HAIKU=@0@'.format(is_os_haiku.to_int()),
	'-DOS_LINUX=@0@'.format(is_os_linux.to_int()),
	'-DOS_NETBSD=@0@'.format(is_os_netbsd.to_int()),
	'-DOS_OPENBSD=@0@'.format(is_os_openbsd.to_int()),
	'-DOS_SUNOS=@0@'.format(is_os_sunos.to_int()),            # Illumos and Solaris
	'-DOS_WINDOWS=@0@'.format(is_os_windows.to_int()),        # Any version of Windows
]

cxx_flags = extra_flags
add_project_arguments(cxx.get_supported_arguments(cxx_flags), language: 'cpp')


################################################################################
# Subdirectories

root_directory = include_directories('./')
include_directory = include_directories('./include/')

subdir('docs')
subdir('src')
subdir('po')
#subdir('tests')
executable(meson.project_name(),
	cpp_args: cxx_flags + extra_flags,
	dependencies: [
		asan_dep, ubsan_dep,
		
		giomm_dep, glibmm_dep, gtkmm_dep,
		momuma_dep,
	],
	implicit_include_directories: false,
	include_directories: [ include_directory, root_directory ],
	install: true,
	sources: momuma_sources,
)
