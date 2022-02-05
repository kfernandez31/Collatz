all:
	g++ -pthread -std=c++2a -o main main.cpp teams.cpp sharedresults.cpp my_collatz.cpp
	g++ -pthread -std=c++2a -o new_process new_process.cpp
