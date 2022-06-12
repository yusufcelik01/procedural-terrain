CC=gcc
CXX=g++
CFLAGS= -Wall -Werror -g

all:
	g++ main.cpp -o main -g -lglfw -lpthread -lX11 -ldl -lXrandr -lGLEW -lGL -DGL_SILENCE_DEPRECATION -DGLM_ENABLE_EXPERIMENTAL -I.



cubemap: cubemap.cpp helpers.o
	g++  helpers.o cubemap.cpp -o cubemap -g -lm -lglfw3 -lpthread -lX11 -ldl -lXrandr -lGLEW -lGL -DGL_SILENCE_DEPRECATION -DGLM_ENABLE_EXPERIMENTAL -I. -msse2

helpers.o: helpers.hpp helpers.cpp stb_image.h
	$(CXX) -c $(CFLAGS) helpers.cpp -o helpers.o
	
.PHONY: clean
clean:
	-rm *.o
	-rm main
