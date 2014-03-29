all:
	     g++ -o manga manga.cpp `pkg-config --cflags opencv` `pkg-config --libs opencv`
