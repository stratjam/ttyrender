#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
#include "input.h"

int config_input(int argc, char** argv)
{
	if (argc < 1) {
		fputs("Not enough args.\n", stderr);
		return 1;
	}

	struct input in = create_input_state(0, 0);
	struct input_thread* in_thread;

	struct input_conf in_conf = { 1., 1., 0., 0., 1., 1., 1 };
	if (start_input_thread(&in, in_conf, argv[0], &in_thread))
		fputs("Failed to open event listener!", stderr);

	int rel_x_max = INT_MIN;
	int rel_x_min = INT_MAX;
	int rel_y_max = INT_MIN;
	int rel_y_min = INT_MAX;

	while (!in.keys[KEY_ESC]) {
		if (in.x > rel_x_max)
			rel_x_max = in.x;
		if (in.x < rel_x_min)
			rel_x_min = in.x;
		if (in.y > rel_y_max)
			rel_y_max = in.y;
		if (in.y < rel_y_min)
			rel_y_min = in.y;
		fprintf(stderr, "\r\e[KRel: %d %d Bounds: %d to %d by %d to %d", in.x, in.y, rel_x_min, rel_x_max, rel_x_min, rel_x_max);
		usleep(4000);
	}

	stop_input_thread(in_thread);
}
