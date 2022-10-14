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
#include "draw.h"
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

#define OFFSET_X 2048
#define OFFSET_Y 2048

#define TGT_FILE "host:cube.bin"


void error_forever(struct draw_state *st);


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
	gs_init(&st, GS_PSM_32, GS_PSMZ_24);

	struct model m = {0};
	m.r = 0xff;

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

	struct render_state r = {0};

	r.camera_pos[0] = 0.0f;
	r.camera_pos[2] = 400.0f;
	r.camera_pos[3] = 1.0f;

	r.camera_rot[3] = 1.0f;

	r.clear_col[0] = 0xb1;
	r.clear_col[1] = 0xce;
	r.clear_col[2] = 0xcb;

	r.offset_x = OFFSET_X;
	r.offset_y = OFFSET_Y;

	struct model_instance inst = {0};
	inst.m = &m;
	inst.scale[0] = 100.0f;
	inst.scale[1] = 100.0f;
	inst.scale[2] = 100.0f;
	inst.scale[3] = 1.0f;

	graph_wait_vsync();
	while(1) {
		update_draw_matrix(&r);
		dma_wait_fast();
		qword_t *q = buf;
		memset(buf, 0, 20000*16);
		// clear
		q = draw_disable_tests(q, 0, &st.zb);
		q = draw_clear(q, 0, 2048.0f - 320, 2048.0f - 244,
			VID_W, VID_H,
			r.clear_col[0], r.clear_col[1], r.clear_col[2]);
		q = draw_enable_tests(q, 0, &st.zb);

		qword_t *model_verts_start = q;
		memcpy(q, m.buffer, m.buffer_len);
		info("copied mesh buffer with len=%d", m.buffer_len);
		
		q += (m.buffer_len / 16);
		q = draw_finish(q);

		mesh_transform((char*)(model_verts_start + MESH_HEADER_SIZE), &inst, &r);

		dma_channel_send_normal(DMA_CHANNEL_GIF, buf, q-buf, 0, 0);

		info("draw from buffer with length %d", q-buf);

		draw_wait_finish();
		graph_wait_vsync();

		inst.rotate[0] += 0.1f;
		inst.rotate[1] += 0.1f;
		r.camera_pos[2] -= 0.1f;
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