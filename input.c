#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <linux/input.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "input.h"

void handle_key(struct input_thread* st, struct input_event data)
{
	st->in->keys[data.code] = (bool)data.value;
}

// ifs are slow; __builtin_expect minimizes this
#define BOUNDS(q, n, b)				\
	if (__builtin_expect(q, 0)) {}		\
	else if (__builtin_expect(n < 0, 0))	\
		n = 0;				\
	else if (__builtin_expect(n > b, 0))	\
		n = b;

void handle_relative(struct input_thread* st, struct input_event data)
{
	switch (data.code) {
	case REL_X:
		st->in->x += (float)data.value * st->conf.rel_mult_x;
		BOUNDS(st->conf.no_bounds, st->in->x, st->in->w);
		break;

	case REL_Y:
		st->in->y -= (float)data.value * st->conf.rel_mult_y;
		BOUNDS(st->conf.no_bounds, st->in->y, st->in->h);
		break;
	}
}

void handle_absolute(struct input_thread* st, struct input_event data)
{
	int mapped;
	switch (data.code) {
	case ABS_X:
		st->in->x = (float)data.value * st->conf.abs_mult_x + st->conf.abs_off_x;
		BOUNDS(st->conf.no_bounds, st->in->x, st->in->w);
		break;

	case ABS_Y:
		st->in->y = (float)data.value * st->conf.abs_mult_y + st->conf.abs_off_y;
		BOUNDS(st->conf.no_bounds, st->in->y, st->in->h);
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
