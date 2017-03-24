all:main.cpp log.cpp
	g++ log.cpp main.cpp  -o main -std=c++11 -ggdb -lpthread -I./

toy:toy_handle_segv.cpp
	g++ toy_handle_segv.cpp  -o main -std=c++11 -ggdb -lpthread -I./

cmd_queue:CmdQueue.cpp
	g++ log.cpp CmdQueue.cpp -o main -std=c++11 -ggdb