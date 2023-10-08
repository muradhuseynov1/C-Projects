/* Print out the two half pyramids from the first two levels of Mario! Their size should be read from the stand input with scanf.
For example if the input is 4, the output should be:
   X  X
  XX  XX
 XXX  XXX
XXXX  XXXX

parameters:

there two spaces between the half pyramids in each line
the program prints capital 'X's
the program does NOT print anything on the output, even when reading the size
if the input is less than 1, the program does not print anything
there are no extra empty spaces at the end of the last 'X' in a line
*/

#include <stdio.h>

int main()
{
    int height;
    scanf("%d", &height);

    if (height > 0) {
        int i = 1;
        while (i <= height) {
            int j = height;
            while (j > i) {
                printf(" ");
                j--;
            }
            int k = 1;
            while (k <= i) {
                printf("X");
                k++;
            }
            printf("  ");
            int l = 1;
            while (l <= i) {
                printf("X");
                l++;
            }
            printf("\n");
            i++;
        }
    }
    else {
        printf("");
    }
    return 0;
}
