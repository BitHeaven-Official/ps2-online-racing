#ifndef DEBUG_H
#define DEBUG_H

#if 1
#define info(msg, ...) printf("[INFO] " msg "\n", ##__VA_ARGS__)
#else
#define info(msg, ...) ((void)0)
#endif


int print_buffer(qword_t *b, int len);

#endif