shapes:
	g++ -std=c++17 -O2 -Iinclude src/shapes.cpp src/glad.c -o part1 -lglfw -lGL -ldl -pthread
cube:
	g++ -std=c++17 -O2 -Iinclude src/cube.cpp src/glad.c -o cube -lglfw -lGL -ldl -pthread
