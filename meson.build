project(
  'thunar-vcs-plugin',
  'c',
  version: '0.3.0-dev',
  license: 'GPL-2.0-or-later',
  meson_version: '>= 0.56.0',
  default_options: [
    'c_std=gnu11',
    'buildtype=debugoptimized',
    'warning_level=2',
  ]
)

project_namespace = 'thunar-plugins'
pkgdatadir = get_option('prefix') / get_option('datadir') / meson.project_name()
copyright_year = '2025'

cc = meson.get_compiler('c')
gnome = import('gnome')
i18n = import('i18n')

dependency_versions = {
  'apr': '>=0.9.7',
  'apr-util': '>=0.9.1',
  'glib': '>= 2.66.0',
  'gtk': '>= 3.24.0',
  'xfce4': '>= 4.18.0',
}

glib = dependency('glib-2.0', version: dependency_versions['glib'])
gthread = dependency('gthread-2.0', version: dependency_versions['glib'])
gobject = dependency('gobject-2.0', version: dependency_versions['glib'])
glib = dependency('glib-2.0', version: dependency_versions['glib'])
gtk = dependency('gtk+-3.0', version: dependency_versions['gtk'])
exo = dependency('exo-2', version: dependency_versions['xfce4'])
if libxfce4ui.version().version_compare('< 4.21.0')
  exo = dependency('exo-2', version: dependency_versions['xfce4'])
else
  exo = dependency('', required: false)
endif
libxfce4util = dependency('libxfce4ui-2', version: dependency_versions['xfce4'])
libxfce4ui = dependency('libxfce4util-1.0', version: dependency_versions['xfce4'])
thunarx = dependency('thunarx-3', version: dependency_versions['xfce4'])

feature_cflags = []

# Feature: 'git'
enable_git = not get_option('git').disabled()
if enable_git
  feature_cflags += '-DHAVE_GIT=1'
  feature_cflags += '-DTVP_GIT_HELPER="@0@"'.format(get_option('prefix') / get_option('libexecdir') / 'tvp-git-helper')
endif

# Feature: 'svn'
apr = dependency('apr-1', version: dependency_versions['apr'], required: get_option('svn'))
apr_util = dependency('apr-util-1', version: dependency_versions['apr-util'], required: get_option('svn'))
svn_deps = [apr, apr_util]

enable_svn = not get_option('svn').disabled()
foreach dep : svn_deps
  enable_svn = enable_svn and dep.found()
endforeach
if enable_svn
  foreach lib : ['svn_subr-1', 'svn_fs-1', 'svn_wc-1', 'svn_client-1']
    dep = cc.find_library(lib, required: false)
    svn_deps += dep
    enable_svn = enable_svn and dep.found()
  endforeach
endif
if enable_svn
  feature_cflags += '-DHAVE_SUBVERSION=1'
  feature_cflags += '-DTVP_SVN_HELPER="@0@"'.format(get_option('prefix') / get_option('libexecdir') / 'tvp-svn-helper')
endif

if cc.has_function('bind_textdomain_codeset')
  feature_cflags += '-DHAVE_BIND_TEXTDOMAIN_CODESET=1'
  libintl = dependency('', required: false)
else
  libintl = cc.find_library('intl', required: false)
  if libintl.found() and cc.has_function('bind_textdomain_codeset', dependencies: libintl)
    feature_cflags += '-DHAVE_BIND_TEXTDOMAIN_CODESET=1'
  endif
endif

extra_cflags = []
extra_cflags_check = [
  '-Wmissing-declarations',
  '-Wmissing-noreturn',
  '-Wold-style-definition',
  '-Wredundant-decls',
  '-Wpointer-arith',
  '-Wcast-align',
  '-Winit-self',
  '-Wshadow',
  '-Wmissing-include-dirs',
  '-Wundef',
  '-Wformat',
  '-Wformat-security',
  '-Wformat-y2k',
  '-Wnested-externs',
  '-Wno-unused-parameter',
  '-Wno-declaration-after-statement',
  '-Wno-missing-field-initializers',
  '-Werror=implicit-function-declaration',
  '-Wno-error=deprecated-declarations',
]

headers = [
  'sys/types.h',
  'sys/wait.h',
  'stdlib.h',
  'string.h',
  'unistd.h',
]
foreach header : headers
  if cc.check_header(header)
    extra_cflags += '-DHAVE_@0@=1'.format(header.underscorify().to_upper())
  endif
endforeach

optimization = get_option('optimization')
if get_option('debug') and optimization in ['0', 'g']
  extra_cflags_check += '-fstack-protector-strong'
  extra_cflags += [
    '-DDEBUG=1',
    '-DDEBUG_TRACE=1',
    '-DG_ENABLE_DEBUG',
  ]
elif optimization in ['3', 'minsize']
  extra_cflags += [
    '-DNDEBUG',
    '-DG_DISABLE_CAST_CHECKS',
    '-DG_DISABLE_ASSERT',
  ]
endif

if dependency_versions.has_key('glib')
  glib_version_parts = dependency_versions['glib'].split(' ')
  glib_min_version_parts = glib_version_parts[1].split('.')
  glib_min_version_define = 'GLIB_VERSION_@0@_@1@'.format(glib_min_version_parts[0], glib_min_version_parts[1])
  extra_cflags += [
    '-DGLIB_VERSION_MIN_REQUIRED=@0@'.format(glib_min_version_define),
    '-DGLIB_VERSION_MAX_ALLOWED=@0@'.format(glib_min_version_define),
    '-DG_LOG_USE_STRUCTURED=1',
  ]
endif

version_parts = meson.project_version().split('-dev')[0].split('.')
version_short = '@0@.@1@'.format(version_parts[0], version_parts[1])

extra_cflags += [
  '-DPACKAGE="@0@"'.format(meson.project_name()),
  '-DPACKAGE_NAME="@0@"'.format(meson.project_name()),
  '-DPACKAGE_VERSION="@0@"'.format(meson.project_version()),
  '-DVERSION="@0@"'.format(meson.project_version()),
  '-DVERSION_SHORT="@0@"'.format(version_short),
  '-DPACKAGE_STRING="@0@ @1@"'.format(meson.project_name(), meson.project_version()),
  '-DPACKAGE_DATADIR="@0@"'.format(pkgdatadir),
  '-DCOPYRIGHT_YEAR="@0@"'.format(copyright_year),
  '-DPACKAGE_LOCALE_DIR="@0@"'.format(get_option('prefix') / get_option('localedir')),
  '-DPACKAGE_BUGREPORT="https://gitlab.xfce.org/@0@/@1@/-/issues"'.format(project_namespace, meson.project_name()),
  '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
  '-DPREFIX="@0@"'.format(get_option('prefix')),
  '-DBINDIR="@0@"'.format(get_option('prefix') / get_option('bindir')),
  '-DDATADIR="@0@"'.format(get_option('prefix') / get_option('datadir')),
  '-DINCLUDEDIR="@0@"'.format(get_option('prefix') / get_option('includedir')),
  '-DLIBDIR="@0@"'.format(get_option('prefix') / get_option('libdir')),
  '-DLIBEXECDIR="@0@"'.format(get_option('prefix') / get_option('libexecdir')),
  '-DLOCALEDIR="@0@"'.format(get_option('prefix') / get_option('localedir')),
  '-DLOCALSTATEDIR="@0@"'.format(get_option('prefix') / get_option('localstatedir')),
  '-DSBINDIR="@0@"'.format(get_option('prefix') / get_option('sbindir')),
  '-DSYSCONFDIR="@0@"'.format(get_option('prefix') / get_option('sysconfdir')),
  '-DHAVE_XFCE_REVISION_H=1',
]

add_project_arguments(cc.get_supported_arguments(extra_cflags_check), language: 'c')
add_project_arguments(extra_cflags, language: 'c')
add_project_arguments(feature_cflags, language: 'c')

xfce_revision_h = vcs_tag(
  command: ['git', 'rev-parse', '--short', 'HEAD'],
  fallback: 'UNKNOWN',
  input: 'xfce-revision.h.in',
  output: 'xfce-revision.h',
  replace_string: '@REVISION@',
)

subdir('icons')
subdir('po')
subdir('thunar-vcs-plugin')
if enable_git
  subdir('tvp-git-helper')
endif
if enable_svn
  subdir('tvp-svn-helper')
endif
