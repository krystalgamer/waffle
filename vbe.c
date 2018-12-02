#include <lcom/lcf.h>
#include <vbe.h>
#include <stdlib.h>
#include "util.h"

vbe_mode_info_t vbe_mode_info;
static uint8_t* mapped_mem;
static uint8_t *backbuffer = NULL;
static uint32_t buffer_size = 0;
static uint8_t bytes_per_pixel = 0;


void *retry_lm_alloc(size_t size, mmap_t *mmap){
    void *result = NULL;
    for(unsigned i = 0; i < 5 ; i++){
        result = lm_alloc(size, mmap);
        if(result != NULL)
            break;
        sleep(1);
    }

    return result;
}

int vbe_get_mode_info_2(uint16_t mode, vbe_mode_info_t * vmi_p) {
    struct reg86u r;
    mmap_t mmap;

    /* Reset the struct values */
    memset(&r, 0, sizeof(r));

    /* Allocate memory block in low memory area */
    if (retry_lm_alloc(sizeof(vbe_mode_info_t), &mmap) == NULL) {
    	printf("(%s): lm_alloc() failed\n", __func__);
    	return VBE_LM_ALLOC_FAILED;
    }

    /* Build the struct */
    r.u.b.ah = VBE_FUNC; 
    r.u.b.al = RETURN_VBE_MODE_INFO;
    r.u.w.cx = mode;
    r.u.w.es = PB2BASE(mmap.phys);
    r.u.w.di = PB2OFF(mmap.phys);
    r.u.b.intno = VIDEO_CARD_SRV;

    /* BIOS Call */
    if( sys_int86(&r) != FUNC_SUCCESS ) {
        lm_free(&mmap);
        printf("(%s): sys_int86() failed \n", __func__);
        return VBE_SYS_INT86_FAILED;
    }

    /* Verify the return for errors */
    if (r.u.w.ax != FUNC_RETURN_OK) {
        lm_free(&mmap);
        printf("(%s): sys_int86() return in ax was different from OK \n", __func__);
        return VBE_INVALID_RETURN;    	
    }

    /* Copy the requested info to vbe_mode_info */
    memcpy(vmi_p, mmap.virt, sizeof(vbe_mode_info_t));

    /* Free allocated memory */
    lm_free(&mmap);

    return VBE_OK;
}

int set_video_mode(uint16_t mode){

    struct reg86u r;

    /* Reset the struct values */
    memset(&r, 0, sizeof(r));

    /* Build the struct */
    r.u.b.ah = VBE_FUNC; 
    r.u.b.al = SET_VBE_MODE; 
    r.u.w.bx = LINEAR_FRAME_BUFFER | mode;
    r.u.b.intno = VIDEO_CARD_SRV;

    /* BIOS Call */
    if( sys_int86(&r) != FUNC_SUCCESS ) {
        printf("(%s): sys_int86() failed \n", __func__);
        return VBE_SYS_INT86_FAILED;
    } 
    return VBE_OK;
}

void* (vg_init)(uint16_t mode){

    /* Initialize lower memory region */
    if(lm_init(true) == NULL){
        printf("(%s) Couldnt init lm\n", __func__);
        return NULL;
    }

    int res = 0;
    if((res = vbe_get_mode_info_2(mode, &vbe_mode_info)) != OK ){
        printf("(%s) Couldnt get mode info\n", __func__);
        return NULL;
    }
    
    bytes_per_pixel = calculate_size_in_bytes(get_bits_per_pixel());
    buffer_size = get_x_res() * get_y_res() * bytes_per_pixel;

    backbuffer = malloc(buffer_size);
    memset(backbuffer, 0, buffer_size);


    struct minix_mem_range mr; /* physical memory range */
    unsigned int vram_base = vbe_mode_info.PhysBasePtr; /* VRAM’s physical addresss */

    unsigned int vram_size = vbe_mode_info.XResolution * vbe_mode_info.YResolution * vbe_mode_info.BitsPerPixel ; /* VRAM’s size, but you can use
    the frame-buffer size, instead */

    void *video_mem; /* frame-buffer VM address */

    /* Allow memory mapping */

    mr.mr_base = (phys_bytes) vram_base;
    mr.mr_limit = mr.mr_base + vram_size;

    if( OK != (res = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
        panic("sys_privctl (ADD_MEM) failed: %d\n", res);
    /* Map memory */
    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
    if(video_mem == MAP_FAILED)
        panic("couldn’t map video memory");

    /* Store the mapped memmory pointer in mapped_mem */
    mapped_mem = video_mem;

    /* Set video mode */
    if(set_video_mode(mode) != OK)
        return NULL;

    return video_mem;    
}


int (pj_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {

    /* Check if out of bounds */
    if (x >= get_x_res() || y >= get_y_res()) {			
        printf("(%s) Invalid coordinates: x=%d, y=%d", __func__, x, y);
        return VBE_INVALID_COORDS;		
    }

    /* Calculate size in bytes each pixel occupies */
    for (uint32_t i = 0; i < len; i++){
        /* Check if x is outside of range */
        if (x+i >= get_x_res())
        return VBE_OK;

        /* Color the pixel */
        uint32_t y_coord = y * get_x_res() * bytes_per_pixel;
        uint32_t x_coord = (x + i) * bytes_per_pixel;  
        memcpy(backbuffer + y_coord + x_coord, &color, bytes_per_pixel);
    }

    return VBE_OK;
}


int (pj_draw_rectangle)(int16_t x, int16_t y, uint16_t width, uint16_t height, uint32_t color) {

    /* Check if completly out of bounds */
    /* TODO: this is unsafe af, but since resolutions dont go over nor near 32,767 we're cool */
    if(x > (int16_t)get_x_res() || y > (int16_t)get_y_res())
        return 1;
    if ((x < 0 && (x+(int16_t)width) < 0) || (y < 0 && (y + (int16_t)height) < 0 )){
        //printf("(%s) Invalid coordinates: x=%d, y=%d\n", __func__, x, y);
        return 2; 
    }

    if(x<0){
        width += x;
        x = 0;
    }

    if(y<0){
        height += y;
        y = 0;
    }

    /* Write a line for the whole height */
    for (uint32_t i = 0; i < height; i++) {
        // Check if out of screen range
        if (y+i >= get_y_res()) break;
            if (pj_draw_hline((uint16_t)x, (uint16_t)y + i, width, color) != OK) {
                //printf("(%s) There was a problem drawing a h line\n", __func__);
                return VBE_DRAW_LINE_FAILED;
        }
    }
    return VBE_OK;
}

void draw_pixmap_on(const char *pixmap, uint16_t x, uint16_t y, int width, int height, uint8_t *buffer){

    /* Iterate lines */
    for(int i = 0; i < height; i++){
        /* Y is out of bounds */
        if((i+y) >= get_y_res())
            break;

        /* Iterate columns */
        for(int j = 0; j<width; j++){         
            /* X is out of bounds */
            if((j+x) >= get_x_res())
                break;
            /* Draw the pixmap pixel */
            buffer[(y+i)*get_x_res() + x + j] = pixmap[i*width + j];
        }
    }
}

void (draw_pixmap)(const char *pixmap, uint16_t x, uint16_t y, int width, int height){
    draw_pixmap_on(pixmap, x, y, width, height, mapped_mem);
}

void (clear_buffer)(uint8_t color){
    memset(backbuffer, color, buffer_size);
}


void swap_buffers(){
    memcpy(mapped_mem, backbuffer, buffer_size);
}


uint8_t get_bits_per_pixel() { return vbe_mode_info.BitsPerPixel; }
uint16_t get_x_res() { return vbe_mode_info.XResolution; }
uint16_t get_y_res() { return vbe_mode_info.YResolution; }
uint8_t get_memory_model() { return vbe_mode_info.MemoryModel; }
uint8_t get_red_mask_size() { return vbe_mode_info.RedMaskSize; }
uint8_t get_red_field_position() { return vbe_mode_info.RedFieldPosition; }
uint8_t get_blue_mask_size() { return vbe_mode_info.BlueMaskSize; }
uint8_t get_blue_field_position() { return vbe_mode_info.BlueFieldPosition; }
uint8_t get_green_mask_size() { return vbe_mode_info.GreenMaskSize; }
uint8_t get_green_field_position() { return vbe_mode_info.GreenFieldPosition; }
uint8_t get_rsvd_mask_size() { return vbe_mode_info.RsvdMaskSize; }
uint8_t get_rsvd_field_position() { return vbe_mode_info.RsvdFieldPosition; }


void draw_font_symbol(uint8_t * symbol, uint16_t x, uint16_t y, int width, int height, uint32_t color) {
    
    /* Iterate lines */
    for(int i = 0; i < height; i++){
        /* Y is out of bounds */
        if((i+y) >= get_y_res())
            break;

        /* Iterate columns */
        for(int j = 0; j<width; j++){         
            /* X is out of bounds */
            if((j+x) >= get_x_res())
                break;
            
            /* Get symbol color */
            uint32_t temp;
            memcpy(&temp, symbol + (i*width + j) * bytes_per_pixel, bytes_per_pixel);

            /* If transparent position, do not draw anything */
            if (temp == TRANSPARENCY_COLOR_8_8_8_8)
                continue;

            /* Draw with specified color */
            temp = color;
            memcpy(backbuffer + ((y+i)*get_x_res() + x + j) * bytes_per_pixel, &temp, bytes_per_pixel);
        }
    }
}

uint8_t get_bytes_per_pixel() { return bytes_per_pixel; }
