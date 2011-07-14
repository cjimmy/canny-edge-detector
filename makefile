CC=gcc
CPP=g++


all: main.cpp
	$(CPP) -o BMP main.cpp EasyBMP.cpp

clean:
	rm -f *.o *~ a.out *core* BMP
