cube:
	g++ -std=c++17 -O2 -Iinclude src/cube.cpp src/glad.c -o cube -lglfw -lGL -ldl -pthread
shapes:
	g++ -std=c++17 -O2 -Iinclude src/shapes.cpp src/glad.c -o shapes -lglfw -lGL -ldl -pthread

