all: main.o matrix.o invert.o
	g++ -pthread main.o matrix.o invert.o

main.o: main.cpp matrix.hpp invert.hpp 
	g++ -pthread main.cpp -c -o main.o

matrix.o: matrix.cpp matrix.hpp
	g++ -pthread matrix.cpp -c -o matrix.o

invert.o: invert.cpp invert.hpp
	g++ -pthread invert.cpp -c -o invert.o

clean:
	rm -f *.o *.out

zip:
	zip ex2.zip *.cpp *.hpp Makefile
