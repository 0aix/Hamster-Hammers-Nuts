.PHONY : all clean

CPP := g++ -Wall -Werror -g

NAMES := \
	Animation.cpp \
	Assets.cpp \
	Audio.cpp \
	Camera.cpp \
	EndlessScene.cpp \
	EndScene.cpp \
	Game.cpp \
	Graphics.cpp \
	Main.cpp \
	MainMenu.cpp \
	Mesh.cpp \
	Object.cpp \
	Scene.cpp \
	StoryScene.cpp


all : ../Dist/hhn

clean :
	rm -f hhn $(NAMES:.cpp=.o)

%.o : %.cpp
	$(CPP) -I../Assets -I/usr/include/SDL2 -c -o'$(@)' '$<'

../Dist/hhn : $(NAMES:.cpp=.o)
	$(CPP) -o'$(@)' $^ -lpng -lSDL2 -lSDL2_mixer -lGL
