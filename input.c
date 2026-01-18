#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/param.h>
#include <sys/time.h>
#include <linux/input.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "input.h"

int map(int n, int s1, int s2, int d1, int d2)
{
	return (int)((float)(n - s1) / (float)(s2 - s1) * (float)(d2 - d1) + d1);
}

void handle_key(struct input_thread* st, struct input_event data)
{
	st->in->keys[data.code] = (bool)data.value;
}

void handle_relative(struct input_thread* st, struct input_event data)
{
	switch (data.code) {
	case REL_X:
		st->in->x += data.value * st->conf.rel_mult_x;
		st->in->x = MIN(MAX(st->in->x, 0), st->in->w);
		break;

	case REL_Y:
		st->in->y -= data.value * st->conf.rel_mult_y;
		st->in->y = MIN(MAX(st->in->y, 0), st->in->h);
		break;
	}
}

void handle_absolute(struct input_thread* st, struct input_event data)
{
	int mapped;
	switch (data.code) {
	case ABS_X:
		if (data.value < st->abs_w_min)
			st->abs_w_min = data.value;
		if (data.value > st->abs_w_max)
			st->abs_w_max = data.value;
		mapped = map(data.value, st->abs_w_min, st->abs_w_max, 0, st->in->w);
		st->in->x = (float)mapped * st->conf.abs_mult_x + st->conf.abs_off_x * st->in->w;
		st->in->x = MIN(MAX(st->in->x, 0), st->in->w);
		break;

	case ABS_Y:
		if (data.value < st->abs_h_min)
			st->abs_h_min = data.value;
		if (data.value > st->abs_h_max)
			st->abs_h_max = data.value;
		mapped = map(data.value, st->abs_h_min, st->abs_h_max, 0, st->in->h);
		st->in->y = (float)mapped * st->conf.abs_mult_y + st->conf.abs_off_y * st->in->h;
		st->in->y = MIN(MAX(st->in->y, 0), st->in->h);
		break;
	}
}

struct input create_input_state(int w, int h)
{
	struct input in = { 0 };
	in.w = w;
	in.h = h;
	return in;
}

void* input_thread(struct input_thread* st)
{
	struct input_event data;
	while (1) {
		read(st->fd, &data, sizeof(struct input_event));

		switch (data.type) {
		case EV_KEY:
			handle_key(st, data);
			break;
		case EV_REL:
			handle_relative(st, data);
			break;
		case EV_ABS:
			handle_absolute(st, data);
			break;
		}
	}

	return NULL;
}

int start_input_thread(
		struct input* in,
		const struct input_conf conf,
		char* evfile,
		struct input_thread** st_ptr)
{
	*st_ptr = malloc(sizeof(struct input_thread));

	struct input_thread* st = *st_ptr;
	*st = (const struct input_thread){ 0 };

	st->fd = open(evfile, O_RDWR);
	if (st->fd == -1)
		return 1;

	st->conf = conf;
	st->in = in;

	if (pthread_create(&st->thr, NULL, (void*(*)(void*))input_thread, (void*)st))
		return 2;

	return 0;
}

void stop_input_thread(struct input_thread* st)
{
	pthread_cancel(st->thr);
	close(st->fd);
	free(st);
}
