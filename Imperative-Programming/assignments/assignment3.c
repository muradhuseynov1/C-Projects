/*
Create a program which reads times and notes as command line arguments and writes these time-note pairs to a file. Input format: <time> <note> <time> <note>...
The output file name should be todolist.txt. If all arguments are correct then don't print anything to the output. 
If too few arguments are given (0 or 1 input) then print "Wrong input!" and finish execution.

In case of wrong input, the program should print "Wrong input!" (as many times as many wrong inputs there are) and 
continue the processing of further time-note pairs.
The input is wrong if:
- arguments are not meeting the requirements of the format
- any time is not a proper time between 00:00 and 23:59
- any note contains characters other than the lower or upper case English alphabet (you don't need to deal with accented letters)

Correctness of time should be checked in function "isValidTime()" and correctness of notes should be checked in "isValidNote()".

Correct example:
./todo 13:58 assignment 00:00 sleep 08:30 breakfast

Incorrect examples:
./todo 13:58 assignment 24:00 sleep 08:30 breakfast
./todo 13:58 assignment sleep 00:00 08:30 breakfast

In the first example, the time component of the second pair is incorrect. After testing "24:00" the processing should continue with testing 08:30. 
In the second example, the components of the second pair are swapped. 
After testing "sleep" we continue with processing 08:30. 
You may assume that the input is not missing a time or a note. In the worst case, only their format is wrong.

Example todolist.txt:
13:58 - assignment
00:00 - sleep
08:30 - breakfast
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isValidTime(char *time) {
	
	char hour[3];
	char minute[3];
	while (time[2] == ':') {
		hour[0] = time[0];
		hour[1] = time[1];
		minute[0] = time[3];
		minute[1] = time[4];
		

		if((atoi(hour) > 23) || (atoi(minute) > 59)) {
			return 0;
		}
		else {
			return 1;
		}
	}
    return 0;
}

int isValidNote(char *note) {
	int i=0;
	while(note[i] != '\0') {
		if ((note[i] < 'A' || note[i] > 'z') && (note[i] > 'Z' || note[i] < 'a')) {
			return 1;
		}
	i++;
	}
	return 1;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Wrong input!\n");
		return 1;
	}
	else {
		FILE *fl;
		for(int i=1; i<argc; i++) {
			if(isValidTime(argv[i]) && isValidNote(argv[i+1])) {
				fl = fopen ("todolist.txt", "a");
				fprintf(fl, "%s - %s\n", argv[i], argv[i+1]);
				fclose(fl);
			}
			else {
				printf("Wrong Input!\n");
			}
			++i;
		}
		return 0;
	}
}
