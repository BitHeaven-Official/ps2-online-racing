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


#define SHIFT_AS_I64(x, b) (((int64_t)x)<<b)
#define BITftoi4(x) ((x) << 4)

#define OFFSET_X 2048
#define OFFSET_Y 2048

#define VID_W 640
#define VID_H 448

static qword_t *buf;

#define DRAWBUF_LEN (100 * 16)


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

int gs_finish()
{
  qword_t *q = buf;
  q = draw_finish(q);
  dma_channel_send_normal(DMA_CHANNEL_GIF, buf, q-buf, 0, 0);
  dma_wait_fast();

  return 0;
}

int gs_init(int w, int h, int psm, int psmz, int vmode, int gmode)
{
  framebuffer_t fb;
  fb.address = graph_vram_allocate(w, h, psm, GRAPH_ALIGN_PAGE);
  fb.width = w;
  fb.height = h;
  fb.psm = psm;
  fb.mask = 0;

  zbuffer_t z;
  z.address = graph_vram_allocate(w, h, psmz, GRAPH_ALIGN_PAGE);
  z.enable = 0;
  z.method = 0;
  z.zsm = psmz;
  z.mask = 0;

  graph_set_mode(gmode, vmode, GRAPH_MODE_FIELD, GRAPH_DISABLE);
  graph_set_screen(OFFSET_X, OFFSET_Y, w, h);
  graph_set_bgcolor(0, 0, 0);
  graph_set_framebuffer_filtered(fb.address, w, psm, 0, 0);
  graph_enable_output();

  qword_t *q = buf;
  memset(buf, 0, DRAWBUF_LEN);
  q = draw_setup_environment(q, 0, &fb, &z);
  q = draw_finish(q);
  dma_channel_send_normal(DMA_CHANNEL_GIF, buf, q-buf, 0, 0);
  draw_wait_finish();

  return 0;
}

static int tri[] = {
  10, 0, 0,
  600, 200, 0,
  20, 400, 0
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

  int x = 0, y = 0, z = 0;

  for(int i = 0; i < 3; i++) {
    q->dw[0] = (red&0xff) | (green&0xff)<<32;
    q->dw[1] = (blue&0xff) | SHIFT_AS_I64(0x80, 32);
    q++;

    int base = i*3;
    x = BITftoi4(tri[base+0] + OFFSET_X);
    y = BITftoi4(tri[base+1] + OFFSET_Y);
    y = 0;
    q->dw[0] = x | SHIFT_AS_I64(y, 32);
    q->dw[1] = z;
    q++;
  }

  return q;
}

int main()
{
  printf("Hello\n");
  buf = malloc(DRAWBUF_LEN);
  // init DMAC
  dma_channel_initialize(DMA_CHANNEL_GIF, 0, 0);
  dma_channel_fast_waits(DMA_CHANNEL_GIF);
  // initialize graphics mode
  // 640x480 (4:3)
  // 853x480 (16:9)
  // 768x576 (4:3)
  // 1024x576 (16:9)
  int vmode = graph_get_region();
  vmode = GRAPH_MODE_NTSC;
  gs_init(VID_W, VID_H, GS_PSM_32, GS_PSMZ_32, vmode, GRAPH_MODE_INTERLACED);

  vertex_t atri[3] = {
    { 10.0f, 2.0f, 0 },
    { 310.0f, 2.0f, 0 },
    { 600.0f, 300.0f, 0}
  };

  color_t col;
  col.rgbaq = 0xff00f;

  triangle_t ttri = {
    atri[0], atri[1], atri[2], col
  };

  graph_wait_vsync();

  while(1) {
    dma_wait_fast();
    qword_t *q = buf;
    memset(buf, 0, DRAWBUF_LEN);
    // clear
    q = draw_clear(q, 0, 0, 0, VID_W, VID_H, 128, 0, 128);
    q = draw_triangle_filled(q, 0, &ttri);
    q = draw(q);
    q = draw_finish(q);
    dma_channel_send_normal(DMA_CHANNEL_GIF, buf, q-buf, 0, 0);

    // draw
    // draw();

    // wait vsync
    draw_wait_finish();
    graph_wait_vsync();
  }
}
