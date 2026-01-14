all: life life2data

life: life.c loadlife.c
	mpicc -o life life.c loadlife.c

life2data: life2data.c
	gcc life2data.c -o life2data
	
