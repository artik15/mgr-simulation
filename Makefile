CFLAGS= -Wno-implicit-function-declaration 
CLIB= -lngspice -ldl -lm -lgomp -lc -lgcc_s 

sim: sim-posix.o ssdi.o
	gcc $(CFLAGS) sim-posix.o ssdi.o -o sim $(CLIB)

sim-posix.o: sim-posix.c
	gcc $(CFLAGS) -c sim-posix.c $(CLIB)

ssdi.o: ssdi.h ssdi.c
	gcc $(CFLAGS) -c ssdi.c $(CLIB)

clean:
	rm *.o
	rm sim
	
