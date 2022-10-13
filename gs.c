int gs_init(int width, int height, int psm, int psmz, int vmode, int gmode)
{
  framebuffer_t fb;
  fb.address = graph_vram_allocate(width, height, psm, GRAPH_ALIGN_PAGE);
  fb.width = width;
  fb.height = height;
  fb.psm = psm;
  fb.mask = 0;

  z->address = graph_vram_allocate(width, height, psmz, GRAPH_ALIGN_PAGE);
  z->enable = 0;
  z->method = 0;
  z->zsm = 0;
  z->mask = 0;

  graph_set_mode(gmode, vmode, GRAPH_MODE_FIELD, GRAPH_DISABLE); 
  graph_set_screen(0, 0, width, height);
  graph_set_bgcolor(0, 0, 0);
  graph_set_framebuffer_filtered(fb.address, width, psm, 0, 0);
  graph_enable_output();

  qword_t *q = buf;
  memset(buf, 0, DRAWBUF_LEN);
  q = draw_setup_environment(q, 0, &fb, z);
  q = draw_primitive_xyoffset(q, 0, 2048-(VID_W/2), 2048-(VID_H/2));
  q = draw_finish(q);
  dma_channel_send_normal(DMA_CHANNEL_GIF, buf, q-buf, 0, 0);
  draw_wait_finish();

  return 0;
}

