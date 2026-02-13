#ifndef TERM_H_
#define TERM_H_

#include <termios.h>

struct termios prepare_term();

void restore_term(struct termios orig_term);

#endif

