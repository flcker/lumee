bin_PROGRAMS = lumee
dist_pkgdata_DATA = data/app_menu.ui data/main.ui data/style.css
gsettings_SCHEMAS = data/com.github.bmars.Lumee.gschema.xml
lumee_SOURCES = src/application.cpp src/application.h src/image_list.cpp \
                src/image_list.h src/image_view.cpp src/image_view.h \
                src/image_worker.cpp src/image_worker.h src/main.cpp \
                src/main_window.cpp src/main_window.h src/utils.cpp \
                src/utils.h src/work_queue.cpp src/work_queue.h
lumee_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" -DBINDIR=\"$(bindir)\" \
                 $(gtkmm_CFLAGS)
lumee_LDADD = $(gtkmm_LIBS)

@GSETTINGS_RULES@

# This is needed for running from the build directory.
data/gschemas.compiled: $(gsettings_SCHEMAS:.xml=.valid)
	$(AM_V_GEN) $(GLIB_COMPILE_SCHEMAS) data
all-local: data/gschemas.compiled

EXTRA_DIST = README.md $(gsettings_SCHEMAS)
CLEANFILES = data/gschemas.compiled
