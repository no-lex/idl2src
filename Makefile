CXXFLAGS= --pedantic  -O3 -fno-rtti -fomit-frame-pointer -ffast-math -std=c++17 -Wextra -fsigned-char -fno-exceptions

INCLUDES=-Iinclude -Ilib -pthread

all: idl2src

idl2src : main.o parsegdl.o parsestructs.o
		g++ $(CXXFLAGS) $(INCLUDES) -o idl2src main.o parsegdl.o parsestructs.o lib/libsourcetraildb.a -ldl
		mv main.o parsegdl.o parsestructs.o ./src/

main.o :
		g++ $(CXXFLAGS) $(INCLUDES) -c src/main.cpp

parsegdl.o :
		g++ $(CXXFLAGS) $(INCLUDES) -c src/parsegdl.cpp

parsestructs.o :
		g++ $(CXXFLAGS) $(INCLUDES) -c src/parsestructs.cpp

clean:
		rm -f src/main.o src/parsegdl.o src/parsestructs.o idl2src
