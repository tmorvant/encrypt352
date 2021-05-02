encrypt352: encrypt352.o encrypt-module.o
		gcc -o encrypt352 encrypt352.c encrypt-module.c -lpthread

encrypt352.o: encrypt352.c 
		gcc -Wall -g -c encrypt352.c -lpthread

encrypt-module.o: encrypt-module.c encrypt-module.h
		gcc -Wall -g -c encrypt-module.c -lpthread		

clean:
		rm -f encrypt352.o encrypt-module.o encrypt352 output.txt
