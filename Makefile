all: Mem_Allocation_Project

clean:
	rm -f Mem_Allocation_Project

Mem_Allocation_Project:
	gcc -o Mem Mem_Allocation_Project.c -Werror -lm
	./Mem
