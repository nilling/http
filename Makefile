CC  = gcc -O2 -Wall -s
#cpp = g++ -O2 -Wall -s -std=c++0x
cpp = g++ -O0  -g -Wall -std=c++0x 
OBJS = httpres.o test.o
LIBSO = -lstdc++
# CC = g++ - O2 -Wall -s
OPTIONS = -fPIC -shared -D_FILE_OFFSET_BITS=64


.cpp.o:
	$(cpp) -c $< $(OPTIONS)

all: $(OBJS) httptest

httptest:
	$(cpp) -o httptest httpres.o test.o $(LIBSO)

clean:
	rm -f *.o *~core.* httptest