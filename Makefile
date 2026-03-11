CXX = g++
CXXFLAGS = -Wall
DEBUGFLAGS = -g -Wall -Werror

all: server client

debug: CXXFLAGS = $(DEBUGFLAGS)
debug: all

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp

clean:
	rm -f server client