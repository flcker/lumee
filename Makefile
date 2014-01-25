CXXFLAGS = -std=c++11 -MMD -MF $(dir $*).deps/$(notdir $*).d -MP \
		   $(shell pkg-config --cflags gtkmm-3.0) \
		   -DDATA_DIR=\""$(shell pwd)/data"\"
LDLIBS = $(shell pkg-config --libs gtkmm-3.0)

objects = src/app.o src/image.o src/main.o src/model.o src/window.o

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
