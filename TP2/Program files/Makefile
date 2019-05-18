CC=gcc
CFLAGS=-Werror -Wall -pedantic -fopenmp -lnetcdf -O3 -std=gnu11

FILE_ORG=OR_ABI-L2-CMIPF-M6C02_G16_s20191011800206_e20191011809514_c20191011809591.nc

FILE_PROC=procedural.nc
FILE_PAR =parallel.nc
FILE_RES = parallel2.nc
all: bin/main 

bin/main : bin/main.o
	$(CC) $(CFLAGS) bin/main.o -o bin/main
	cp $(FILE_ORG)  bin/
	cp $(FILE_ORG)  output/$(FILE_PROC)
	cp $(FILE_ORG)  output/$(FILE_PAR)
	cp $(FILE_ORG)  output/$(FILE_RES)
	
bin/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o bin/main.o  


clean:
	rm bin/main*
