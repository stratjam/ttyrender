#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "player.h"
#include "autoinput.h"

int main(int argc, char** argv)
{
	if (argc < 2) {
		fputs("Not enough args.\n", stderr);
		return 1;
	}

	if (!strcmp(argv[1], "player"))
		player(argc - 2, argv + 2);
	else if (!strcmp(argv[1], "configinput"))
		config_input(argc - 2, argv + 2);
}
