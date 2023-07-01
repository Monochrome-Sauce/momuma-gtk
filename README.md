# Momuma Gtk

MoMuMa (**M**ono's **M**usic **M**anager) is a simple GTKmm app for managing simple music playlists.  
The project uses `libmpv` to gain its playback capabilities.

Originally I wanted to write this in C, but chose to use C++ due to enhanced type safety (this is the first time I'm using Gtk).

## Dependencies

- gcc/clang (build)
- pkg-config (build)
- giomm >= 2.60 (library)
- glibmm >= 2.60 (library)
- gtkmm >= 3.24 (library)
- momuma (library)

## Compiling

Go to the base of the project's code and execute the following commands:

1. `meson setup build`
2. `cd build`
3. `meson compile`

## Installation

- To install:   `sudo meson install`.
- To uninstall: `sudo ninja uninstall`.

`ninja install` can also be used, but it's safer to use `meson install` as explained at
[Meson - Installing as the superuser](https://mesonbuild.com/Installing.html#installing-as-the-superuser).

When running the code for debugging, set `LSAN_OPTIONS=suppressions=sanitizer-blacklist.txt`
