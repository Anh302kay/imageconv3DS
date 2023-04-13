CXX = clang++
CXXFLAGS = -std=c++20 -Wall -I /usr/local/include
LDFLAGS = -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf 
DEBUGFLAGS = -g

all: debug

release:
	$(CXX) -O3 $(CXXFLAGS) source/*.cpp   -I include  $(LDFLAGS) -o bin/release/game

debug:
	$(CXX) $(DEBUGFLAGS) $(CXXFLAGS) source/*.cpp -I include $(LDFLAGS) -o bin/debug/game

clean:
	rm -r bin/debug/*.* bin/release/*.*

web:
	em++ -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2 -s SDL2_IMAGE_FORMATS='["png"]' -std=c++20 -Iinclude -Ires source/main.cpp source/window_draw.cpp source/window_events.cpp source/renderwindow.cpp source/entity.cpp --preload-file .\res -o b.html --use-preload-plugins -mnontrapping-fptoint