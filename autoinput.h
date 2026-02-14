#ifndef AUTOINPUT_H_
#define AUTOINPUT_H_

#include "input.h"

int config_input(int argc, char** argv);

struct input_thread** setup_input_threads(char* conf_file, struct input* in);

void cleanup_input_threads(struct input_thread** in_threads);

#endif
