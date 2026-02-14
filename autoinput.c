#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
#include <signal.h>
#include "input.h"
#include "graphics.h"

static int stop = 0;

void sig_handler(int sig_num)
{
	stop = 1;
}

int config_input(int argc, char** argv)
{
	if (argc < 1) {
		fputs("Not enough args.\n", stderr);
		return 1;
	}

	signal(SIGINT, sig_handler);

	struct input in = create_input_state(0, 0);
	struct input_thread* in_thread;

	struct input_conf in_conf = { 1., 1., 0., 0., 1., 1., 1 };
	if (start_input_thread(&in, in_conf, argv[0], &in_thread))
		fputs("Failed to open event listener!\n", stderr);

	int rel_x_max = INT_MIN;
	int rel_x_min = INT_MAX;
	int rel_y_max = INT_MIN;
	int rel_y_min = INT_MAX;

	while (!stop) {
		if (in.x > rel_x_max)
			rel_x_max = in.x;
		if (in.x < rel_x_min)
			rel_x_min = in.x;
		if (in.y > rel_y_max)
			rel_y_max = in.y;
		if (in.y < rel_y_min)
			rel_y_min = in.y;
		fprintf(stderr, "\r\e[KRel: %d %d Bounds: %d to %d by %d to %d", in.x, in.y, rel_x_min, rel_x_max, rel_y_min, rel_y_max);
		usleep(4000);
	}

	fputs("\n", stderr);
	stop_input_thread(in_thread);

	struct frame_buffer* fb = get_frame_buffer();

	float abs_mult_x = 1.0 / ((float)rel_x_max - (float)rel_x_min) * fb->w;
	float abs_mult_y = 1.0 / ((float)rel_y_max - (float)rel_y_min) * fb->h;
	int abs_off_x = -(float)rel_x_min * abs_mult_x;
	int abs_off_y = -(float)rel_y_min * abs_mult_y;

	printf("%s\t%f\t%f\t%d\t%d\n", argv[0], abs_mult_x, abs_mult_y, abs_off_x, abs_off_y);
	fflush(stdout);

	free_frame_buffer(fb);
}
