CPP = mpic++
CFLAGS = -std=c++11 -lsfml-graphics -lsfml-window -lsfml-system

TARGETS = Coordinate.o Coordinate.h.gch Actor.h.gch Actor.o Map.o Map.h.gch Server.o Client.o server client

all: Coordinate.o Actor.o Map.o server client

Coordinate.o: Coordinate.h Coordinate.cpp
	${CPP} -c ${CFLAGS} Coordinate.h Coordinate.cpp

Actor.o: Actor.h Actor.cpp
	${CPP} -c ${CFLAGS} Actor.h Actor.cpp

Map.o: Map.h Map.cpp
	${CPP} -c ${CFLAGS} Map.h Map.cpp

server: Server.cpp
	${CPP} NetworkHelpers.cpp Server.cpp Map.cpp Actor.cpp Coordinate.cpp -o server ${CFLAGS} -pthread

client: Client.cpp
	${CPP} ${CFLAGS} NetworkHelpers.cpp Coordinate.cpp Client.cpp -o client

clean:
	-rm -f ${TARGETS}
