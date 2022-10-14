#include <stdio.h>

#include <dma.h>

#include "debug.h"


int print_buffer(qword_t *b, int len)
{
    printf("-- buffer\n");
    for(int i = 0; i < len; i++) {
        printf("%016llx %016llx\n", b->dw[0], b->dw[1]);
        b++;
    }
    printf("-- /buffer\n");

    return 0;
}

void error_forever(struct draw_state *st)
{
	qword_t *buf = malloc(1200);

	int red = 5;
	int i = 1;

	while(1) {
		if(red < 255 && red > 0) {
			red += 5 * i;
		}
		else {
			i *= -1;
			red += 5 * i;
		}

		dma_wait_fast();
		qword_t *q = buf;
		memset(buf, 0, 1200);
		q = draw_disable_tests(q, 0, &st->zb);
		q = draw_clear(q, 0, 2048.0f - 320, 2048.0f - 244, VID_W, VID_H, red, 0, 0);
		q = draw_finish(q);
		dma_channel_send_normal(DMA_CHANNEL_GIF, buf, q-buf, 0, 0);
		draw_wait_finish();
		graph_wait_vsync();
	}
}