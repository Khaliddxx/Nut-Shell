
myshell: myshell.o
	gcc myshell.o -o myshell

myshell.o: myshell.c
	gcc -c myshell.c

clean:
	rm *.o myshell
