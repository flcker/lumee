Lumee is an image viewer based on browsing folders of images. Developed with
GTK+ and designed for GNOME 3, it aims to have an unobtrusive and minimalist
UI. It's still in early development.

![Screenshot](http://i.imgur.com/u2E9M4p.png)

Lumee's main requirement is gtkmm 3.10 or newer. To build from source, you need
a C++ compiler and gtkmm headers (for example, on Fedora: `yum install gcc-c++
gtkmm30-devel`). Then, use the following commands:

    $ autoreconf -i
    $ ./configure
    $ make
    $ ./lumee
