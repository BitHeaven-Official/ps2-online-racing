#include <math3d.h>
#include <draw_types.h>

#include <stdint.h>
#include <stdio.h>

#include "draw.h"
#include "mesh.h"
#include "debug.h"


#define ZMAX (1024 * 1024)


void mesh_transform(char *b, struct model_instance *inst, struct render_state *d)
{
	MATRIX tmp;
	MATRIX model;
	matrix_unit(model);
	create_model_matrix(model, inst->translate, inst->scale, inst->rotate);
	matrix_unit(tmp);
	matrix_multiply(tmp, model, d->world_to_screen);

	int stride = inst->m->vertex_size * 16;
	for (int i = 0; i < inst->m->vertex_count; i++) {
		// get address of current vertex data
		float *pos = (float*)(b + (stride * i) + (inst->m->vertex_position_offset * 16));
		VECTOR *v = (float (*)[4])pos;

		vector_apply((float *)v, (float *)v, tmp);

        // float z = pos[2];

		*((uint32_t*)pos) = ftoi4(pos[0] + d->offset_x);
		*((uint32_t*)(pos+1)) = ftoi4(pos[1] + d->offset_y);
		uint32_t zv = (uint32_t)(ZMAX * (pos[2] + 1000.0f) / 1700.0f);
		*((uint32_t*)(pos+2)) = zv;

		uint32_t * col = (uint32_t*)(b + (stride * i) + (inst->m->vertex_color_offset * 16));
		col[1] = 0x0f;
		col[2] = 0x0f;
		col[0] = (char)(((zv * 1.0f) / (ZMAX * 1.0f)) * 0xff);
		col[3] = 0x80;
		
		// ((uint32_t*)pos) = (short)((pos[0]+1.0f)*d->offset_x);
		// ((uint32_t*)(pos+1)) = (short)((pos[1]+1.0f)*d->offset_y);
		// ((uint32_t*)(pos+2)) = (unsigned int)((pos[2]+1.0f)*20);

		pos[3] = 0;
	}
}

void create_model_matrix(MATRIX tgt, VECTOR translate, VECTOR scale, VECTOR rotate)
{
	matrix_unit(tgt);
	matrix_rotate(tgt, tgt, rotate);
	matrix_scale(tgt, tgt, scale);
	matrix_translate(tgt, tgt, translate);
}

void update_draw_matrix(struct render_state *d)
{
	create_view_screen(d->view_screen, 1.0f, -3.0f, 3.0f, -3.0f, 3.0f, 1.0f, 2000.0f);
	create_world_view(d->world_view, d->camera_pos, d->camera_rot);
	matrix_multiply(d->world_to_screen, d->world_view, d->view_screen);
}

