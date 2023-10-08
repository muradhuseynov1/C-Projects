#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Person {
	char name[30];
	unsigned int age;
	char job[30];
} Person;

struct Person **iterator;
unsigned int size = 10;

void memory_allocation() {
	iterator=malloc(size * sizeof(struct Person));

	for(unsigned int i=0; i<size; ++i) {
		iterator[i]=NULL;
	}
}

unsigned int count=0;

void memory_reallocation() {
	iterator=realloc(iterator, size*sizeof(struct Person));

	printf("size: %u , cnt: %u\n", size, count);
}

void more_people() {
	if(count%5==0 && (count != 0 && count !=5)) {
		size=size+5;
		memory_reallocation();
	}

	iterator[count]=(struct Person*)malloc(sizeof(struct Person));
	printf("\nname: ");
	scanf("%s", iterator[count]->name);
	printf("\nage: ");
	scanf("%u", &(iterator[count]->age));
	printf("\njob: ");
	scanf("%s", iterator[count]->job);
	count+=1;
}

char functions() {
	char chr;
	printf("\n");
	printf("a: (add a person)            \n");
	printf("l: (list the whole database) \n");
	printf("x: (exit the program)        \n");
	printf("d: (delete person)           \n");
	printf("\n");
	while(!(scanf("%c", &chr)) && chr!='a' && chr!='l' && chr!='x' && chr!='d') {
		printf("\nError Message! Again: ");
		scanf("%c", &chr);
	}
	return chr;
}

void free_dyn_memory() {
	for(unsigned int i=0; i<size; ++i) {
		if(iterator[i]!=NULL) {
			free(iterator[i]);
			iterator[i]=0;
		}
	}
	free(iterator);
	iterator=0;
}

void person_access() {
	if(count==0) {
		printf("no person in the dataBase given!\n");
	}

	for(unsigned int i=0; i<count; ++i) {
		printf("name: %s\n", iterator[i]->name);
		printf("age: %d\n", iterator[i]->age);
		printf("job: %s\n", iterator[i]->job);
	}
}



int main() {
	memory_allocation();
	do{
		char f=functions();
		switch(f) {
			case 'a':
				more_people();
				break;
			case 'l':
				person_access();
				break;
			
			case 'd':
				printf(" ");
				break;
			case 'x':
				free_dyn_memory;
				return 0;

		} 
	} while(1);
	return 0;
}
