CXXFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -Werror \
           -MMD -MF $(dir $*).deps/$(notdir $*).d -MP \
           $(shell pkg-config --cflags gtkmm-3.0) \
           -DDATA_DIR=\""$(shell pwd)/data"\"
LDLIBS = $(shell pkg-config --libs gtkmm-3.0)

objects = src/application.o src/image_list.o src/image_view.o \
          src/image_worker.o src/lumee.o src/main_window.o src/utils.o \
          src/work_queue.o

all: src/.deps/ lumee
src/.deps/:
	mkdir $@
lumee: $(objects)
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f $(objects) lumee
	rm -rf src/.deps/

-include src/.deps/*.d
