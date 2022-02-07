all:
	g++ -pthread -std=c++2a -o main main.cpp teams.cpp
	g++ -pthread -std=c++2a -o new_process new_process.cpp
