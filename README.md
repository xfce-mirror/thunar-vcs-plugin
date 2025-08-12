[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://gitlab.xfce.org/thunar-plugins/thunar-vcs-plugin/-/blob/master/COPYING)

# thunar-vcs-plugin

The Thunar VCS Plugin adds Subversion and GIT actions to the context menu of thunar.
This gives an SVN and GIT integration to Thunar.

This project was formerly known as Thunar SVN Plugin.

The current features are:
* Most of the svn action: add, blame, checkout, cleanup, commit, copy, delete, export, import, lock, log, move, properties, relocate, resolved, revert, status, switch, unlock, update.
* Subversion info in file properties dialog.
* Basic git actions: add, blame, branch, clean, clone, log, move, reset, stash, status.


----

### Homepage

[Thunar-vcs-plugin documentation](https://docs.xfce.org/xfce/thunar/thunar-vcs-plugin)

### Changelog

See [NEWS](https://gitlab.xfce.org/thunar-plugins/thunar-vcs-plugin/-/blob/master/NEWS) for details on changes and fixes made in the current release.

### Source Code Repository

[thunar-vcs-plugin source code](https://gitlab.xfce.org/thunar-plugins/thunar-vcs-plugin)

### Download a Release Tarball

[Thunar-vcs-plugin vcs](https://archive.xfce.org/src/thunar-plugins/thunar-vcs-plugin)
    or
[Thunar-vcs-plugin tags](https://gitlab.xfce.org/thunar-plugins/thunar-vcs-plugin/-/tags)

### Installation

From source code repository: 

    % cd thunar-vcs-plugin
    % meson setup build
    % meson compile -C build
    % meson install -C build

From release tarball:

    % tar xf thunar-vcs-plugin-<version>.tar.xz
    % cd thunar-vcs-plugin-<version>
    % meson setup build
    % meson compile -C build
    % meson install -C build

### Uninstallation

    % ninja uninstall -C build

### Reporting Bugs

Visit the [reporting bugs](https://docs.xfce.org/thunar-plugins/thunar-vcs-plugin/bugs) page to view currently open bug reports and instructions on reporting new bugs or submitting bugfixes.

