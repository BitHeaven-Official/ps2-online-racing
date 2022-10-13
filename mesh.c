#include <tamtypes.h>
#include <stdio.h>
#include <gs_gp.h>
#include <string.h>
#include <stdlib.h>

#include "mesh.h"
#include "debug.h"


#define MODEL_BUFFER_PRE (4*16)


int load_file(const char *fname, char *b, int b_len)
{
	FILE *f = fopen(fname, "rb");
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(len >= b_len) {
		return 0;
	}
	size_t byte_read = fread(b, 1, len, f);
	fclose(f);
	return byte_read;
}

int load_model(struct model *m, char *b, int b_len)
{
	if(!m || !b || b_len <= 0) {
		return 0;
	}
	m->buffer = malloc(b_len + MODEL_BUFFER_PRE);
	m->buffer_len = b_len + MODEL_BUFFER_PRE;
	qword_t *q = m->buffer;

	m->vertex_count = b_len / 16;
	m->vertex_size = 1;
	m->vertex_position_offset = 0;
	m->face_count = m->vertex_count / 3;
	info("initializing model: verts=%d, faces=%d, bytes in buf=%d",
		m->vertex_count, m->face_count, m->buffer_len);

	// Create giftag, set regs via A+D
	q->dw[0] = 0x1000000000000002;
	q->dw[1] = 0x000000000000000e;
	q++;
	// set PRIM = triangle
	q->dw[0] = GS_SET_PRIM(GS_PRIM_TRIANGLE, 0, 0, 0, 0, 0, 0, 0, 0);
	q->dw[1] = GS_REG_PRIM;
	q++;
	// set RGBAQ
	q->dw[0] = GS_SET_RGBAQ(m->r, m->g, m->b, 0x80, 0x80);
	q->dw[1] = GS_REG_RGBAQ;
	q++;
	// start vertex data GIFTAG
	q->dw[0] = 0x3000000000000000 | (m->face_count & 0x3fff);
	q->dw[1] = 0x0000000000000555;
	q++;

	memcpy(q, b, b_len);

	return 1;
}
