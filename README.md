Lumee is an image viewer for GNOME 3. It features a clean user interface that's
designed around browsing folders of images.

![Screenshot](http://i.imgur.com/u2E9M4p.png)

Lumee's main requirement is gtkmm 3.10 or later. To build from source, you need
a C++ compiler and gtkmm headers (for example, on Fedora: `yum install gcc-c++
gtkmm30-devel`). If using Git, run `autoreconf -i` first to generate the build
system. Then, use the following commands:

    $ ./configure
    $ make
    $ ./lumee

You can optionally install Lumee with `sudo make install` and uninstall with
`sudo make uninstall`.
