FLAGS=-O3 -Wall
OUT=graph

all: graph

graph: graph.c
	$(CC) $(FLAGS) -o $(OUT) graph.c

runParallel: graphParallel
	mpirun -np 4 graphParallel

parallel: graphParallel.c
	mpicc -o graphParallel graphParallel.c

clean:
	rm $(OUT)
