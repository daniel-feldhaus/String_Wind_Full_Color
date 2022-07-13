CC := g++
CFLAGS := -g -O2 -Wall
POSTFLAGS := -fopenmp -lX11 -lpng -lm -lpthread


string_wind : string_wind.o image_manipulation.o path_gen.o 
	$(CC) $(CFLAGS) -o string_wind image_manipulation.o path_gen.o string_wind.o   $(POSTFLAGS)
string_wind.o : string_wind.cpp path_gen.h image_manipulation.h
	$(CC) $(CFLAGS) -c string_wind.cpp $(POSTFLAGS)
path_gen.o : path_gen.cpp path_gen.h point.h json.hpp color.h defs.h image_manipulation.h
	$(CC) $(CFLAGS) -c path_gen.cpp $(POSTFLAGS)
image_manipulation.o : image_manipulation.cpp image_manipulation.h defs.h color.h
	$(CC) $(CFLAGS) -c image_manipulation.cpp $(POSTFLAGS)

clean :
	rm string_wind string_wind.o image_manipulation.o path_gen.o *.png

clean_images :
	rm *.png