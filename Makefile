CXXFLAGS= -O0 -fomit-frame-pointer -ffast-math -std=c++17 -Wall -fsigned-char -fno-exceptions -ggdb3

INCLUDES=-Iinclude -Ilib -pthread

all: idl2src

idl2src : main.o parsegdl.o parsestructs.o
		g++ $(CXXFLAGS) $(INCLUDES) -o idl2src main.o parsegdl.o parsestructs.o libsourcetraildb.a -ldl

main.o :
		g++ $(CXXFLAGS) $(INCLUDES) -c main.cpp

parsegdl.o :
		g++ $(CXXFLAGS) $(INCLUDES) -c parsegdl.cpp

parsestructs.o :
		g++ $(CXXFLAGS) $(INCLUDES) -c parsestructs.cpp

clean:
		rm -f main.o parsegdl.o parsestructs.o idl2src
