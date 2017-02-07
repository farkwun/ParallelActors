CPP = g++
CFLAGS = -c -std=c++11

TARGETS = Coordinate.o Coordinate.h.gch Actor.h.gch Actor.o Map.o Map.h.gch Server.o Client.o

all: Coordinate.o Actor.o Map.o Server.o Client.o

Coordinate.o: Coordinate.h Coordinate.cpp
	${CPP} ${CFLAGS} Coordinate.h Coordinate.cpp

Actor.o: Actor.h Actor.cpp
	${CPP} ${CFLAGS} Actor.h Actor.cpp

Map.o: Map.h Map.cpp
	${CPP} ${CFLAGS} Map.h Map.cpp

Server.o: Server.cpp
	${CPP} ${CFLAGS} Server.cpp

Client.o: Client.cpp
	${CPP} ${CFLAGS} Client.cpp

clean:
	-rm -f ${TARGETS}
