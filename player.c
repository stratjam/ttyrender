#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <sys/time.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include "player.h"
#include "graphics.h"
#include "input.h"
#include "term.h"

int player(int argc, char** argv)
{
	if (argc < 2) {
		fputs("Not enough args.\n", stderr);
		return 1;
	}

	struct termios orig_term = prepare_term();

	int anim_time = 1. / atof(argv[0]) * 1000000.;

	int im_count = argc - 1;
	struct image* ims[im_count];
	for (int i = 0; i < im_count; i++) {
		ims[i] = load_image(argv[i + 1]);
		if (!ims[i])
			return 1;
	}

	struct frame_buffer* fb = get_frame_buffer();
	if (!fb) {
		fputs("Failed to get frame buffer.\n", stderr);
		return 2;
	}
	struct frame* frame = create_frame(fb);
	if (!frame) {
		fputs("Failed to create frame.\n", stderr);
		return 3;
	}

	struct input in = create_input_state(fb->w, fb->h);
	struct input_thread* in_thread_1;
	struct input_thread* in_thread_2;

	struct input_conf in_conf = { 1., 1., 0., 0., 1., 1. };
	if (start_input_thread(&in, in_conf, "/dev/input/event6", &in_thread_1))
		puts("Failed to open ev4!");

	if (start_input_thread(&in, in_conf, "/dev/input/event8", &in_thread_2))
		puts("Failed to open ev8!");


	int x = in.x;
	int y = in.y;

	int anim_prog = 0;

	uint64_t tot_time = 0;
	uint64_t tot_frames = 0;

	int i = 0;
	int prev_time = 0;
	while (!in.keys[KEY_ESC]) {
		struct timeval tv1, tv2;
		gettimeofday(&tv1, NULL);
		anim_prog += prev_time;
		if (anim_prog > anim_time) {
			anim_prog -= anim_time;
			i++;
			if (i >= im_count)
				i = 0; 
		}

		x = in.x - ims[0]->w / 2;
		y = in.y - ims[0]->h / 2;

		clear(frame);
		draw_image(frame, ims[i], x, y);
		blit(frame);

		gettimeofday(&tv2, NULL);
		prev_time = tv2.tv_usec - tv1.tv_usec + (tv2.tv_sec - tv1.tv_sec) * 1000000;
		tot_time += prev_time;
		tot_frames++;
		int sleep_time = 4000 - prev_time;
		if (sleep_time > 0)
			usleep(sleep_time);
	}
	printf("Avg render time = %dµs\n", tot_time / tot_frames);

	stop_input_thread(in_thread_1);
	stop_input_thread(in_thread_2);

	for (int i = 0; i < im_count; i++) {
		free_image(ims[i]);
	}
	free_frame(frame);
	free_frame_buffer(fb);

	restore_term(orig_term);
}
