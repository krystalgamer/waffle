typedef enum {
    
    BUTTON,
    TEXT_BOX,
    RADIO_BUTTON,

    INVALID

}ElementType;

typedef struct _element{
    
    ElementType type;
    uint16_t x, y;

    uint16_t width, height;

    union{

        struct _button_attr{ 
            char *text;
            uint32_t color, overlay_color;
        }button;

    }attr;

    struct _element *next;

}Element;

/* Fancy rectangle */
typedef struct _window{
    uint32_t id;
    int16_t x,y;
    uint16_t width, height;
    uint32_t color;
    Element *elements;

    struct{
        bool border;
        uint32_t border_width;
    }attr;/* To be filled */
    struct _window *prev, *next;
}Window;

typedef struct _wnd_lst{
    Window *first;
    Window *last;

    /* Cursor status */
    struct{
        int16_t x,y;
        uint16_t width, height;/* temporary fields */
    }cursor;

    struct{
        uint16_t width, height;
        uint32_t color;
        
        struct _menu{
            uint16_t width;
            char *text;
            uint32_t color, overlay_color;
            struct _menu *submenu;
        }menu;

        struct _clock{
            uint8_t n_symbols;
            uint8_t padding_left, padding_top, padding_right;
            uint8_t symbol_width;
            uint32_t symbol_color, background_color;
        }clock;

    }taskbar;

}WindowList;


void window_draw();
void window_mouse_handle();
uint32_t create_window(uint16_t width, uint16_t height, uint32_t color);
void init_internal_status();
bool window_add_element(uint32_t id, ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void * attr);

bool is_window_focused(const Window *wnd);

bool mouse_over_coords(uint16_t x, uint16_t y, uint16_t xf, uint16_t yf);

Element *build_element(ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *attr);
void draw_elements(const Window *wnd);

static const struct _button_attr DEFAULT_BUTTON_ATTR = { "DEFAULT", 0x12131415, 0xFF252319 };

/* TODO fazer isto com bits */
#define L_BUTTON 4 
#define M_BUTTON 2 
#define R_BUTTON 1 

typedef enum{
    L_DEAD = 1,
    L_PRESSED = 2,
    L_KEPT = 4,

    M_DEAD = 8,
    M_PRESSED = 16,
    M_KEPT = 32,

    R_DEAD = 64,
    R_PRESSED = 128,
    R_KEPT = 256

}Event;

uint32_t update_state(const struct packet *pp);

void draw_taskbar();
bool has_taskbar_button_been_pressed();
int draw_taskbar_clock();
