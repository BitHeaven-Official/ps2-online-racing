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
#include "mesh.h"
#include "debug.h"


#define SHIFT_AS_I64(x, b) (((int64_t)x)<<b)
#define BITftoi4(x) ((x)<<4)
#define BITftoi4_I64(x) (((uint64_t)(x))<<4)
#define fatalerror(st, msg, ...) printf("FATAL: " msg "\n", ##__VA_ARGS__); error_forever(st); ((void)0)

// 640x480 (4:3)
// 853x480 (16:9)
// 768x576 (4:3)
// 1024x576 (16:9)
#define VID_W 640
#define VID_H 448

#define TGT_FILE "host:cube.gif"


void error_forever(struct draw_state *st);


void mesh_transform(struct model *m, char *b, int cx, int cy, float scale, float tx, float ty)
{
	int stride = m->vertex_size * 16;
	for(int i = 0; i < m->vertex_count; i++) {
		// get address of current vertex data
		float *pos = (float*) (b + (stride * i) + (m->vertex_position_offset * 16));
		float x = pos[0];
		float y = pos[1];
		float z = pos[2];
		// float w = pos[3];
		*((uint32_t*)pos) = BITftoi4_I64((x * scale) + tx) + cx;
		*((uint32_t*)(pos+1)) = BITftoi4_I64((y * scale) + ty) + cy;
		*((uint32_t*)(pos+2)) = (int)z;
		pos[3] = 0;
	}
}

int main()
{
	printf("Hello\n");
	qword_t *buf = malloc(20000*16);
	char *file_load_buffer = malloc(310 * 1024);
	int file_load_buffer_len = 310 * 1024;

	struct draw_state st = {0};
	st.width = VID_W;
	st.height = VID_H;
	st.vmode = graph_get_region();
	st.gmode = GRAPH_MODE_INTERLACED;
	
	// init DMAC
	dma_channel_initialize(DMA_CHANNEL_GIF, 0, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	// init graphics mode
	gs_init(&st, GS_PSM_32, GS_PSMZ_32);

	struct model m = {0};
	m.r = 0xff;
	m.g = 0x88;
	m.b = 0x22;
	int bytes_read = load_file(TGT_FILE, file_load_buffer, file_load_buffer_len);
	if(bytes_read <= 0) {
		fatalerror(&st, "failed to load file %s", TGT_FILE);
	}
	if(bytes_read % 16 != 0) {
		fatalerror(&st, "length of model file %s was not 0 %% 16", TGT_FILE);
	}

	if(!load_model(&m, file_load_buffer, bytes_read)) {
		fatalerror(&st, "failed to process model");
	}

	graph_wait_vsync();
	while(1) {
		dma_wait_fast();
		qword_t *q = buf;
		memset(buf, 0, 20000*16);
		// clear
		q = draw_disable_tests(q, 0, &st.zb);
		q = draw_clear(q, 0, 2048.0f - 320, 2048.0f - 244, VID_W, VID_H, 10, 10, 10);
		q = draw_enable_tests(q, 0, &st.zb);

		qword_t *model_verts_start = q;

		memcpy(q, m.buffer, m.buffer_len);
		info("copied mesh buffer with len=%d", m.buffer_len);
		q += (m.buffer_len / 16);
		q = draw_finish(q);

		mesh_transform(&m, (char *)(model_verts_start + 4), BITftoi4(2048), BITftoi4(2048), 20.0f, 0, 0);

		dma_channel_send_normal(DMA_CHANNEL_GIF, buf, q-buf, 0, 0);

		info("draw from buffer with length %d", q-buf);

		draw_wait_finish();

		// wait vsync
		graph_wait_vsync();
		sleep(2);
	}
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