#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "autoinput.h"
#include "graphics.h"
#include "util.h"

#define MAX_THREADS 16

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

	struct input_conf in_conf = { 1., 1., 0, 0, 1 };
	struct input_thread* in_thread = start_input_thread(&in, in_conf, argv[0]);
	if (!in_thread)
		fputs("Failed to open event listener!\n", stderr);

	int x_max = INT_MIN;
	int x_min = INT_MAX;
	int y_max = INT_MIN;
	int y_min = INT_MAX;

	while (!stop) {
		if (in.x > x_max)
			x_max = in.x;
		if (in.x < x_min)
			x_min = in.x;
		if (in.y > y_max)
			y_max = in.y;
		if (in.y < y_min)
			y_min = in.y;
		fprintf(stderr, "\r\e[KRel: %d %d Bounds: %d to %d by %d to %d", in.x, in.y, x_min, x_max, y_min, y_max);
		usleep(4000);
	}

	fputs("\n", stderr);
	stop_input_thread(in_thread);

	struct frame_buffer* fb = get_frame_buffer();

	float mult_x = 1.0 / ((float)x_max - (float)x_min) * fb->w;
	float mult_y = 1.0 / ((float)y_max - (float)y_min) * fb->h;
	int off_x = -(float)x_min * mult_x;
	int off_y = -(float)y_min * mult_y;

	printf("%s\t%f\t%f\t%d\t%d\n", argv[0], mult_x, mult_y, off_x, off_y);
	fflush(stdout);

	free_frame_buffer(fb);
}

struct input_thread** setup_input_threads(char* conf_file, struct input* in)
{
	char file[33];
	char num_buf[17];
	int fd = open(conf_file, O_RDONLY);
	if (fd == -1)
		return NULL;

	struct input_thread** in_threads = malloc(MAX_THREADS * sizeof(struct input_thread*));
	if (!in_threads)
		return NULL;
	memset(in_threads, 0, MAX_THREADS * sizeof(struct input_thread*));

	for (int i = 0; i < MAX_THREADS; i++) {
		read_range(fd, file, 32, '!', '~');
		if (!*file)
			return in_threads;

		read_range(fd, num_buf, 16, '!', '~');
		float mult_x = atof(num_buf);

		read_range(fd, num_buf, 16, '!', '~');
		float mult_y = atof(num_buf);

		read_range(fd, num_buf, 16, '!', '~');
		int off_x = atoi(num_buf);

		read_range(fd, num_buf, 16, '!', '~');
		int off_y = atoi(num_buf);

		struct input_conf in_conf = { mult_x, mult_y, off_x, off_y };
		in_threads[i] = start_input_thread(in, in_conf, file);
		if (in_threads[i])
			i--;
	}

	return in_threads;
}

void cleanup_input_threads(struct input_thread** in_threads)
{
	for (int i = 0; i < MAX_THREADS; i++) {
		if (!in_threads[i])
			return;
		stop_input_thread(in_threads[i]);
	}
	free(in_threads);
}
