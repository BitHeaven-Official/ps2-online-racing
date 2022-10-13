#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <draw.h>
#include <graph.h>
#include <gs_psm.h>
#include <gs_gp.h>
#include <dma.h>
#include <dma_tags.h>

#include <inttypes.h>

#include "gs.h"


#define SHIFT_AS_I64(x, b) (((int64_t)x)<<b)
#define BITftoi4(x) ((x)<<4)

// 640x480 (4:3)
// 853x480 (16:9)
// 768x576 (4:3)
// 1024x576 (16:9)
#define VID_W 640
#define VID_H 448


static int tri[] = {
	10, 10, 0,
	500, 20, 0,
	300, 400, 0
};

qword_t *draw(qword_t *q)
{
	uint64_t red = 0xf0;
	uint64_t blue = 0x0f;
	uint64_t green = 0x0f;

	// SET PRIM
	q->dw[0] = 0x1000000000000001;
	q->dw[1] = 0x000000000000000e;
	q++;
	q->dw[0] = GS_SET_PRIM(GS_PRIM_TRIANGLE, 0, 0, 0, 0, 0, 0, 0, 0);
	q->dw[1] = GS_REG_PRIM;
	q++;
	// 6 regs, x1, EOP
	q->dw[0] = 0x6000000000008001;
	// GIFTag header - col, pos, col, pos, col, pos
	q->dw[1] = 0x0000000000515151;
	q++;

	int cx = BITftoi4(2048 - (VID_W/2));
	int cy = BITftoi4(2048 - (VID_H/2));

	for(int i = 0; i < 3; i++) {
		q->dw[0] = (red&0xff) | (green&0xff)<<32;
		q->dw[1] = (blue&0xff) | (SHIFT_AS_I64(0x80, 32));
		q++;
    
		int base = i*3;
		int x = BITftoi4(tri[base+0]) + cx;
		int y = BITftoi4(tri[base+1]) + cy;
		int z = 0;
		q->dw[0] = x | SHIFT_AS_I64(y, 32);
		q->dw[1] = z; 
		q++;
	}

	return q;
}

int main()
{
	printf("Hello\n");
	qword_t *buf = malloc(100*16);
	struct draw_state st = {0};
	st.width = VID_W;
	st.height = VID_H;
	st.vmode = graph_get_region();
	st.gmode = GRAPH_MODE_INTERLACED;
	
	// init DMAC
	dma_channel_initialize(DMA_CHANNEL_GIF, 0, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);
	gs_init(&st, GS_PSM_32, GS_PSMZ_32);

	graph_wait_vsync();

	while(1) {
		dma_wait_fast();
		qword_t *q = buf;
		memset(buf, 0, 100*16);
		// clear
		q = draw_disable_tests(q, 0, &st.zb);
		q = draw_clear(q, 0, 2048.0f - 320, 2048.0f - 244, VID_W, VID_H, 10, 10, 10);
		q = draw_enable_tests(q, 0, &st.zb);
		q = draw(q);
		q = draw_finish(q);
		dma_channel_send_normal(DMA_CHANNEL_GIF, buf, q-buf, 0, 0);
		// print_buffer(buf, q-buf);

		draw_wait_finish();

		// wait vsync
		graph_wait_vsync();
		// sleep(2);
	}
}