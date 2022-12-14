#ifndef MESH_H
#define MESH_H

#include <tamtypes.h>


struct model {
	qword_t *buffer;
	char r;
	char g;
	char b;
	int face_count;
	int vertex_count;
	int vertex_size;
	int vertex_position_offset;
	int vertex_color_offset;
	int buffer_len;
};


int load_file(const char *fname, char *b, int b_len);

int load_model(struct model *m, char *b, int b_len);

#endif