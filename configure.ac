AC_INIT(Lumee, 0.1, [https://github.com/bmars/lumee/issues], [],
        [https://github.com/bmars/lumee])
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE([1.11 foreign dist-xz no-dist-gzip subdir-objects -Wall
                  -Werror])
AM_SILENT_RULES(yes)
PKG_CHECK_MODULES(gtkmm, [gtkmm-3.0 >= 3.10.0])
AX_CXX_COMPILE_STDCXX_11(noext)
AC_CONFIG_FILES(Makefile)
AC_OUTPUT
