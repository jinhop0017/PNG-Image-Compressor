EXE = pngCompressor

OBJS_EXE = RGBAPixel.o lodepng.o PNG.o main.o qtree.o qtree-base.o

CXX = clang++
CXXFLAGS = -std=c++1y -c -g -O0 -Wall -Wextra -pedantic 
LD = clang++
#LDFLAGS = -std=c++1y -stdlib=libc++ -lc++abi -lpthread -lm
LDFLAGS = -std=c++1y -lpthread -lm 

all : pngCompressor

$(EXE) : $(OBJS_EXE)
	$(LD) $(OBJS_EXE) $(LDFLAGS) -o $(EXE)

#object files
RGBAPixel.o : imgUtil/RGBAPixel.cpp imgUtil/RGBAPixel.h
	$(CXX) $(CXXFLAGS) imgUtil/RGBAPixel.cpp -o $@

PNG.o : imgUtil/PNG.cpp imgUtil/PNG.h imgUtil/RGBAPixel.h imgUtil/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) imgUtil/PNG.cpp -o $@

lodepng.o : imgUtil/lodepng/lodepng.cpp imgUtil/lodepng/lodepng.h
	$(CXX) $(CXXFLAGS) imgUtil/lodepng/lodepng.cpp -o $@

qtree.o : qtree.h qtree-private.h qtree.cpp imgUtil/PNG.h imgUtil/RGBAPixel.h
	$(CXX) $(CXXFLAGS) qtree.cpp -o $@

qtree-base.o : qtree.h qtree-private.h qtree-base.cpp imgUtil/PNG.h imgUtil/RGBAPixel.h
	$(CXX) $(CXXFLAGS) qtree-base.cpp -o $@

main.o : main.cpp imgUtil/PNG.h imgUtil/RGBAPixel.h qtree.h qtree.h
	$(CXX) $(CXXFLAGS) main.cpp -o main.o

clean :
	-rm -f *.o $(EXE) images-output/*.png
