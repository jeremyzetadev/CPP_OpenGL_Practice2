#HeaderFiles = HeaderFiles/EBO.h HeaderFiles/VAO.h HeaderFiles/VBO.h HeaderFiles/shaderClass.h HeaderFiles/texture.h
#src=src/main.cpp src/EBO.cpp src/VAO.cpp src/VBO.cpp src/shaderClass.cpp src/stb.cpp src/texture.cpp
#dep=dep/stb/stb_image.h
#files=${dep} ${src} ${HeaderFiles}

src=main.cpp
files=$(src)

glad=dependencies/glad.c 
libs=-lm `sdl2-config --cflags --libs` -lSDL2_mixer `pkg-config --libs glfw3` -ldl

build:
	g++ -g3 -O0 ${glad} ${files} $(libs) -o mainrun -g

clean:
	rm *.o mainrun
