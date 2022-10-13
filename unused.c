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
