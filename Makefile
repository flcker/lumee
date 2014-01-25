FILES = src/app.cpp src/image.cpp src/main.cpp src/model.cpp src/window.cpp
DATA_DIR = "`pwd`/data"

all:
	g++ -std=c++11 `pkg-config --cflags --libs gtkmm-3.0` \
		-DDATA_DIR=\"$(DATA_DIR)\" -o lumee $(FILES)
