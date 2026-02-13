#ifndef INPUT_H_
#define INPUT_H_

struct input {
	int w;
	int h;
	int x;
	int y;
	bool keys[1024];
};

struct input_conf {
	float abs_mult_x;
	float abs_mult_y;
	int abs_off_x;
	int abs_off_y;
	float rel_mult_x;
	float rel_mult_y;
	int no_bounds;
};

struct input_thread {
	struct input* in;
	struct input_conf conf;
	pthread_t thr;
	int fd;
};

struct input create_input_state(int w, int h);

void* input_thread(struct input_thread* st);

int start_input_thread(
		struct input* in,
		const struct input_conf conf,
		char* evfile,
		struct input_thread** st_ptr);

void stop_input_thread(struct input_thread* st);

#endif
