# build an executable named myprog from myprog.c
all: cic.c 
	gcc -g -Wall -o cic cic.c

#./cic -e1
check: 
	./cic

clean: 
	$(RM) cic
