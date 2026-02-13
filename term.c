#include <stdio.h>
#include <unistd.h>
#include "term.h"

struct termios prepare_term()
{
	struct termios orig_term;
	tcgetattr(STDIN_FILENO, &orig_term);
	struct termios term = orig_term;
	term.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
	fputs("\e[?25l", stdout);
}

void restore_term(struct termios orig_term)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
	fputs("\e[?25h", stdout);
}
