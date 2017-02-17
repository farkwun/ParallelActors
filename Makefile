CPP = g++
CFLAGS = -std=c++11

TARGETS = Coordinate.o Coordinate.h.gch Actor.h.gch Actor.o Map.o Map.h.gch Server.o Client.o server client

all: Coordinate.o Actor.o Map.o server client

Coordinate.o: Coordinate.h Coordinate.cpp
	${CPP} -c ${CFLAGS} Coordinate.h Coordinate.cpp

Actor.o: Actor.h Actor.cpp
	${CPP} -c ${CFLAGS} Actor.h Actor.cpp

Map.o: Map.h Map.cpp
	${CPP} -c ${CFLAGS} Map.h Map.cpp

server: Server.cpp
	${CPP} ${CFLAGS} Server.cpp Map.cpp Actor.cpp Coordinate.cpp -o server

client: Client.cpp
	${CPP} ${CFLAGS} Client.cpp -o client

clean:
	-rm -f ${TARGETS}
