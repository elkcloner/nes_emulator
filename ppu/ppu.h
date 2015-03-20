#ifndef __PPU__
#define __PPU__

int ppu_run(int);

int render_frame();

int update_memory();

int set_vBlank();
int clr_vBlank();

#endif
