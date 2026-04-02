
#include <stdio.h>

int main() {
    char input[256];
    while (11){
	if (fgets(input, sizeof(input), stdin) == NULL)
	    break;
	printf("Program 2 demo got: %s", input);
    }
    return 0;

}
