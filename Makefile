CC=gcc
CXX=g++
CFLAGS= -Wall -Werror -g

all:
	g++ main.cpp -o main -g -lglfw -lpthread -lX11 -ldl -lXrandr -lGLEW -lGL -DGL_SILENCE_DEPRECATION -DGLM_ENABLE_EXPERIMENTAL -I.


.PHONY: clean
clean:
	-rm *.o
	-rm main
