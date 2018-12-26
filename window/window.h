#ifndef WINDOW_H
#define WINDOW_H

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

        struct _text_box_attr{
            char *text;
            uint32_t text_size;
            uint32_t background_color, text_color;
            bool selected;
        }text_box;

    }attr;

    struct _element *next;

}Element;

/* Fancy rectangle */
typedef struct _window{
    uint32_t id;
    int16_t x,y;

    uint16_t width, height;
    uint16_t orig_width, orig_height; /* Redundancy for maximize sake */
    uint32_t color;
    bool minimized;
    bool maximized;
    Element *elements;

    struct{
        bool border;
        uint32_t border_width;
        bool frame;
        char *frame_text;
    }attr;/* To be filled */
    struct _window *prev, *next;
}Window;


struct _context_menu;
typedef struct{
    char *text;
    void (*callback)();
    struct _context_menu *menu;
}ContextEntries;

typedef struct _context_menu{
    uint32_t longer_entry;/*length of the longest entry */
    ContextEntries **entries;
    uint32_t size, capacity;
    uint32_t sub_menu_index;
    struct _context_menu *active_sub;
}ContextMenu;

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
            /* b_* prefix means it's the button */
            uint16_t b_width;
            char *b_text;
            uint32_t b_color, b_overlay_color;
            bool b_pressed;

            ContextMenu *context;
        }menu;

        struct _clock{
            uint16_t width;
            uint32_t symbol_color,background_color;
        }clock;

        Window **window_creation_list;
        uint32_t num_created_windows, size_windows;

    }taskbar;

    uint8_t * background_sprite;
    uint16_t bckg_width, bckg_height;

}WindowList;

#define BACKGROUND_COLOR 0x00008081


void window_draw();
void window_mouse_handle();
void window_kbd_handle(const uint8_t *scancode, uint32_t num);
uint32_t create_window(uint16_t width, uint16_t height, uint32_t color, const char *name);
void init_internal_status();
bool window_add_element(uint32_t id, ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void * attr);

bool is_window_focused(const Window *wnd);

bool mouse_over_coords(uint16_t x, uint16_t y, uint16_t xf, uint16_t yf);

Element *build_element(ElementType type, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void *attr);
void draw_elements(const Window *wnd);

static const struct _button_attr DEFAULT_BUTTON_ATTR = { "DEFAULT", 0x12131415, 0xFF252319 };
static const struct _text_box_attr DEFAULT_TEXT_BOX_ATTR = { NULL, 50, 0, 0xFFFFFFFF, true };

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
void init_taskbar_menu();
void free_window();
void set_sub_menu(ContextEntries *entry, ContextMenu *menu);
ContextEntries *get_entry_by_name(ContextMenu *menu, const char *name);
bool call_entry_callback(ContextMenu *menu, uint32_t x, uint32_t y);

int draw_taskbar_clock();
#define N_CLOCK_SYMBOLS 8 /* Number of symbols in clock, hh:mm:ss */
#define MAX_NUM_WINDOWS 6
Window *pressed_window_taskbar();


void so_para_a_nota();
void modify_text_box(Element *element, const uint8_t *scancode, uint32_t num);

#endif
