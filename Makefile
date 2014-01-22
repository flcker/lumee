FILES = src/app.cpp src/main.cpp src/model.cpp src/window.cpp
DATA_DIR = "`pwd`/data"

all:
	g++ $(FILES) -o lumee `pkg-config --cflags --libs gtkmm-3.0` \
		-DDATA_DIR=\"$(DATA_DIR)\"
