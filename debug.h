#ifndef DEBUG_H
#define DEBUG_H

#define info(msg, ...) printf("[INFO] " msg "\n", ##__VA_ARGS__)


int print_buffer(qword_t *b, int len);

void error_forever(struct draw_state *st);

#endif