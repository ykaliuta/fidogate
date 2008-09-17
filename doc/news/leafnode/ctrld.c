#include <stdio.h>

main(int argc, char *argv[])
{
    FILE *fd;
    int c;

    if ((fd = fopen(argv[1], "r")) == NULL) {
	fprintf(stderr, "Can't open %s\n", argv[1]);
	return 1;
    }
    while ((c = fgetc(fd)) != EOF) {
	if (c != 13)
	    putchar(c);
    }
    fclose(fd);
}
