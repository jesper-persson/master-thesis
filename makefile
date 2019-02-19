main:
	g++ lodepng/lodepng.cpp main.cpp -o main.exe -g -L. -lglfw3 -lglew32 -lopengl32 -Wno-psabi