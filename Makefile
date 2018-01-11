CPP = g++
CFLAGS = -std=c++11
SFMLFLAGS = -lsfml-graphics -lsfml-window -lsfml-system
ARMAFLAGS = -larmadillo -llapack -lblas

TARGETS = Coordinate.o Coordinate.h.gch Actor.h.gch Actor.o Map.o Map.h.gch Server.o Client.o Model.h.gch Model.o NeuralNet.h.gch NeuralNet.o server client

all: Coordinate.o Actor.o Map.o Model.o NeuralNet.o server client

Coordinate.o: Coordinate.h Coordinate.cpp
	${CPP} -c ${CFLAGS} Coordinate.h Coordinate.cpp

Actor.o: Actor.h Actor.cpp
	${CPP} -c ${CFLAGS} Actor.h Actor.cpp

Map.o: Map.h Map.cpp
	${CPP} -c ${CFLAGS} Map.h Map.cpp

Model.o: Model.h Model.cpp
	${CPP} -c ${CFLAGS} Model.h Model.cpp

NeuralNet.o: NeuralNet.h NeuralNet.cpp
	${CPP} -c ${CFLAGS} Model.cpp NeuralNet.h NeuralNet.cpp

server: Server.cpp
	${CPP} NetworkHelpers.cpp Server.cpp Map.cpp Actor.cpp Coordinate.cpp -o server ${CFLAGS} ${SFMLFLAGS} -pthread

client: Client.cpp
	${CPP} ${CFLAGS} ${ARMAFLAGS} Model.cpp NeuralNet.cpp NetworkHelpers.cpp Coordinate.cpp Client.cpp -o client

clean:
	-rm -f ${TARGETS}
