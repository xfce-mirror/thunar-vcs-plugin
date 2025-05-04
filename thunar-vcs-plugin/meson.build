tvp_sources = [
  'thunar-vcs-plugin.c',
  'tvp-provider.c',
  'tvp-provider.h',
]

if enable_git
  tvp_sources += [
      'tvp-git-action.c',
      'tvp-git-action.h',
  ]
endif

if enable_svn
  tvp_sources += [
    'tvp-svn-action.c',
    'tvp-svn-action.h',
    'tvp-svn-backend.c',
    'tvp-svn-backend.h',
    'tvp-svn-property-page.c',
    'tvp-svn-property-page.h',
  ]
endif

shared_module(
  'thunar-vcs-plugin',
  tvp_sources,
  gnu_symbol_visibility: 'hidden',
  include_directories: [
    include_directories('..'),
  ],
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format(meson.project_name()),
  ],
  dependencies: [
    glib,
    gobject,
    gtk,
    libxfce4util,
    svn_deps,
    thunarx,
  ],
  name_prefix: '',
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / 'thunarx-3',
)
