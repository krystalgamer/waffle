/* Fancy rectangle */
typedef struct _window{
    int16_t x,y;
    uint16_t width, height;
    uint32_t color;
    struct _window *prev, *next;
}Window;

void window_draw();
void window_mouse_handle();
bool create_window(uint16_t width, uint16_t height, uint32_t color);
void init_internal_status();
