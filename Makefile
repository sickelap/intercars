all:
	llvm-gcc -o i_cars_convert i_cars_convert.c

clean:
	rm -rf i_cars_convert *.o *~ decoded_* *.dSYM

