#ifndef __DISPLAY__
#define __DISPLAY__

int display_frame(char *);

int display_events();

uint8_t controller_read();
int controller_write(uint8_t);

int display_init();
int display_clean();

#endif
