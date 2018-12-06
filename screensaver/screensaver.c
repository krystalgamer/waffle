#include <lcom/lcf.h>
#include "vbe.h"

void screensaver_draw() {    
    clear_buffer_four(0x00008081);
    pj_draw_rectangle(300, 300, 300, 300, 0xffffffff);
}
