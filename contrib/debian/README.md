
Debian
====================
This directory contains files used to package BSd/BS-qt
for Debian-based Linux systems. If you compile BSd/BS-qt yourself, there are some useful files here.

## BS: URI support ##


BS-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install BS-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your BSqt binary to `/usr/bin`
and the `../../share/pixmaps/BS128.png` to `/usr/share/pixmaps`

BS-qt.protocol (KDE)

