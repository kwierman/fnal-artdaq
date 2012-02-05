
INCS = -I$(BOOST_INC) 

CXXFLAGS = -g -fPIC $(INCS)
LDFLAGS = 
LDLIBS = -ldl 

all: main2

main2 : main2.cu
	nvcc main2.cu -o main2

clean:
	/bin/rm -f *.o *.so main2
