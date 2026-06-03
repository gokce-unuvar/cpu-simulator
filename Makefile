all: main1 main2 main3 main4 main5 main6 main7 main8



clean :
	rm -f main1 main2 main3 main4 main5 main6 main7 main8 *.o



main1 : exo1.h exo1.o main1.o
	gcc -Wall -o main1 main1.o exo1.o
	
main1.o : main1.c
	gcc -c main1.c
	
exo1.o : exo1.c
	gcc -c exo1.c
	
	
	
main2 : exo1.h exo1.o exo2.h exo2.o main2.o
	gcc -Wall -o main2 main2.o exo1.o exo2.o
	
main2.o : main2.c
	gcc -c main2.c
	
exo2.o : exo2.c
	gcc -c exo2.c



main3 : exo3.h exo3.o main3.o exo1.o
	gcc -Wall -o main3 main3.o exo3.o exo1.o
	
main3.o : main3.c
	gcc -c main3.c
	
exo3.o : exo3.c
	gcc -c exo3.c
	
	
	
main4 : exo4.h exo4.o main4.o exo1.o exo2.o exo3.o
	gcc -Wall -o main4 main4.o exo4.o exo1.o exo2.o exo3.o 

main4.o : main4.c
	gcc -c main4.c
	
exo4.o : exo4.c
	gcc -c exo4.c
	


main5 : exo5.h exo5.o main5.o exo1.o exo2.o 
	gcc -Wall -o main5 main5.o exo5.o exo1.o exo2.o 

main5.o : main5.c
	gcc -c main5.c
	
exo5.o : exo5.c
	gcc -c exo5.c
	
	
	
main6 : exo6.h exo6.o main6.o exo1.o exo2.o exo3.o
	gcc -Wall -o main6 main6.o exo6.o exo1.o exo2.o exo3.o

main6.o : main6.c
	gcc -c main6.c
	
exo6.o : exo6.c
	gcc -c exo6.c
	
	
	
main7 : exo7.h exo7.o main7.o exo1.o exo2.o exo3.o
	gcc -Wall -o main7 main7.o exo7.o exo1.o exo2.o exo3.o

main7.o : main7.c
	gcc -c main7.c
	
exo7.o : exo7.c
	gcc -c exo7.c
	
	
	
main8 : exo8.h exo8.o main8.o exo1.o exo2.o exo3.o
	gcc -Wall -o main8 main8.o exo8.o exo1.o exo2.o exo3.o

main8.o : main8.c
	gcc -c main8.c
	
exo8.o : exo8.c
	gcc -c exo8.c
