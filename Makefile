all: compile_utils compile_server compile_client

compile_utils:
	g++ -std=c++11 -Wall -c -o utils.o utils.cpp

compile_server:
	g++ -std=c++11 -Wall -o server.o server.cpp

compile_client:
	g++ -std=c++11 -Wall -o client.o client.cpp

server:
	./server.o name

client:
	./client.o name file.txt

clean:
	rm -f *.o *.txt