CPP = mpic++
CFLAGS = -std=c++11 -lsfml-graphics -lsfml-window -lsfml-system

TARGETS = test

all: client 

client: mpi_hello_world.cpp 
	${CPP} ${CFLAGS} mpi_hello_world.cpp -o test

clean:
	-rm -f ${TARGETS}
