sizes = [16, 24, 48]
icons = ['org.xfce.thunar-vcs-plugin.git', 'org.xfce.thunar-vcs-plugin.subversion']

foreach size : sizes
  foreach icon : icons
    install_data(
      '@0@x@0@'.format(size) / '@0@.png'.format(icon),
      install_dir: get_option('prefix') / get_option('datadir') / 'icons' / 'hicolor' / '@0@x@0@'.format(size) / 'apps',
    )
  endforeach
endforeach
