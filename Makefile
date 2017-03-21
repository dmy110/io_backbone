all:main.cpp log.cpp
	g++ log.cpp main.cpp  -o main -std=c++11 -ggdb -lpthread -I./