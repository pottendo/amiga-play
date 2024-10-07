#include "mandellib.h"

#include <stdio.h>

char *cols[] = {"..", "* ", " *", "**"};

int main(int argc, char *argv[])
{
    int x, y, col;

    mandel_init(50, 30);
    for (y = 0; y < 30; y++) {
	for (x = 0; x < 50; x++) {
	    col = mandel(x, y);
	    printf("%s", cols[col]);
	}
	printf("\n");
    }
}

    
