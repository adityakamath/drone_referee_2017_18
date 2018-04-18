/* AUTOGEN File: rtdb_sizeof_tmp.c */

#include <stdio.h>
#include <stdint.h>
#include "Sim2Turtle.h"

int main(void)
{
	FILE* f;
	f= fopen("rtdb_size.tmp", "w");
	fprintf(f, "%lu\n", sizeof(Sim2Turtle));
	fclose(f);

	return 0;
}

/* EOF: rtdb_sizeof_tmp.c */
