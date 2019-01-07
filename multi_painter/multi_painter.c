#include <lcom/lcf.h>
#include "../window/window.h"
#include "../interrupts/serial_port.h"
#include "../font/font.h"


extern WindowList wnd_list;

/** @addtogroup multi_painter
 *  @{
 */
/**
 * @brief Handle the input
 * @param el the input element
 * @param type type of message
 * @param data data of the message
 * @param wnd the current window
 * @return true/false depending if everything was sorted
 */
typedef struct painter_serial{
    uint16_t x; /**< x position */
	uint16_t y; /**< y position */
    int16_t delta_x; /**< delta_x */
	int16_t delta_y; /**< delat y */
}painter_serial;

typedef struct slider_serial{
    uint32_t new_pos; /**< new position on the slider */
    uint32_t id; /**< id of the slider */
}slider_serial;

/**
 * @brief Handle the input
 * @param el the input element
 * @param type type of message
 * @param data data of the message
 * @param wnd the current window
 * @return true/false depending if everything was sorted
 */
bool m_painter_input_handler(Element *el, unsigned type, void *data, Window *wnd);

static bool connected = false; /**< whether the multi painter is connected */
static bool expecting = false; /**< whether it's expecting a response */
/** @} */

void create_multi_painter(){

    uint32_t wnd_width = 500, wnd_height = 700;
    uint32_t wnd_id = create_window(wnd_width, wnd_height, 0x008A8A8A, "Multi Paint", &m_painter_input_handler);
    if(!wnd_id)
        return;

    void *image = malloc(500*500*4);
    memset(image, 0xFF, 500*500*4);
    window_add_element(wnd_id, CANVAS, 0, 0, wnd_width, wnd_width, (void*)&image, "canvas");

    window_add_element(wnd_id, SLIDER, 50, wnd_width+50, 255, 30, NULL, "red"); 
    window_add_element(wnd_id, SLIDER, 50, wnd_width+50+30+5, 255, 30, NULL, "green"); 
    window_add_element(wnd_id, SLIDER, 50, wnd_width+50+30*2+10, 255, 30, NULL, "blue"); 
    window_add_element(wnd_id, SLIDER, 50, wnd_width+50+30*3+10+5, 50, 30, NULL, "brush"); 


    void *color_display = malloc((30*2+10)*(30*2+10)*4);
    memset(color_display, 0, (30*2+10)*(30*2+10)*4);
    window_add_element(wnd_id, IMAGE, 50+255+20, wnd_width+50+(10*2+10)/2, 30*2+10, 30*2+10, &color_display, "color"); 

    if(!ser_set_handler((void*)&m_painter_input_handler, (void*)find_by_id(window_get_by_id(wnd_id), "canvas"), window_get_by_id(wnd_id)))
		delete_window(window_get_by_id(wnd_id));

    struct _checkbox_attr checkbox = {"Host", false};
    window_add_element(wnd_id, CHECKBOX, 100, wnd_width+20, 40, 40,&checkbox, "host"); 

    struct _button_attr button = {"Connect", 0x007A7A7A, 0x005A5A5A};
    window_add_element(wnd_id, BUTTON, 200, wnd_width+20, 100, FONT_HEIGHT, &button, NULL);

    struct _text_attr text = { "Connected", 0, false, false};
    window_add_element(wnd_id, TEXT, 350, wnd_width+20, 0, 0, (void*)&text, "connected");

}

void do_horizontal_brush(Element *canvas, uint32_t *pixels, uint32_t brush_size, uint32_t pos, uint32_t color){

    if(pos > (canvas->width * canvas->height)){
        return;
	}
    /* Only paints pixels on the same line */
    uint32_t line = pos/canvas->width;

    uint32_t first_of_line = line*canvas->width;
    uint32_t last_of_line = (line+1)*canvas->width;
    
    /* TODO What if pos = 0? */
    if(first_of_line < (pos - brush_size))
        first_of_line = pos - brush_size;

    if(last_of_line > (pos + brush_size))
        last_of_line = (pos + brush_size);

    for(uint32_t i = first_of_line; i < last_of_line; i++)
        pixels[i] = color;
}

void paint_with_brush(Element *canvas, uint32_t brush_size, uint32_t x, uint32_t y, uint32_t color){

    uint32_t pos = x + (y*canvas->width);

    if(pos > (canvas->width * canvas->height)){
        return;
	}

    uint32_t *pixels = canvas->attr.canvas.space;

    if(brush_size == 1)
        pixels[pos] = color;
    else{
        do_horizontal_brush(canvas, pixels, brush_size, pos, color);
        brush_size--;

        for(int i = 1; brush_size > 1; brush_size--, i++){
            do_horizontal_brush(canvas, pixels, brush_size, pos+i*canvas->width, color);
            do_horizontal_brush(canvas, pixels, brush_size, pos-i*canvas->width, color);
        }
    }
}

bool m_painter_input_handler(Element *el, unsigned type, void *data, Window *wnd){

    if(type == CANVAS_MSG){
		if(connected == false)
			return false;

		uint32_t brush_size = find_by_id(wnd, "brush")->attr.slider.pos+1;
        uint32_t color = *(uint32_t*)find_by_id(wnd, "color")->attr.image.space;

        uint32_t x =  wnd_list.cursor.x - wnd->x - el->x, y = wnd_list.cursor.y - wnd->y - el->y;
        paint_with_brush(el, brush_size, x, y, color); 
        struct packet *pp = data;

        painter_serial ps = {(uint16_t)x, (uint16_t)y, pp->delta_x, pp->delta_y};
        ser_write_msg_fifo((char*)&ps, sizeof(ps), SERIAL_DRAW);

        if(pp->delta_x == 0 && pp->delta_y == 0)
            return false;

        /* None of them are 0 */
        uint32_t steps_x = abs(pp->delta_x);
        uint32_t steps_y = abs(pp->delta_y);
        bool upwards = (pp->delta_y > 0); 
        bool right = (pp->delta_x > 0); 

		if(right)
			steps_x = (x + steps_x > el->width) ? (el->width-x-1) : steps_x;
		else
			steps_x = (x < steps_x) ? x : steps_x;

		if(upwards)
			steps_y = (y < steps_y) ? y : steps_y;
		else
			steps_y = (y + steps_y > el->height) ? (el->height-y-1) : steps_y;

        for(; steps_x || steps_y; ){
                if(steps_x){
                    if(right) x++;
                    else x--;
                    steps_x--;
                }
                if(steps_y){
                    if(upwards) y--;
                    else y++;
                    steps_y--;
                }
            
                paint_with_brush(el, brush_size, x, y, color); 
        }
    }
    else if(type == SLIDER_MSG){
		if(connected == false)
			return false;
        bool color_change = true;
        uint32_t *pixels = (uint32_t*)find_by_id(wnd, "color")->attr.image.space;
        uint32_t color = *pixels;
        if(!strcmp(el->identifier, "red")){
            slider_serial ss = { el->attr.slider.pos, 0 }; 
            uint8_t red = (uint8_t)el->attr.slider.pos;
            color &= 0xFF00FFFF;
            color |= red<<16;
            ser_write_msg_fifo((char*)&ss, sizeof(ss), SLIDER_SERIAL);

        }
        else if(!strcmp(el->identifier, "blue")){
            slider_serial ss = { el->attr.slider.pos, 1 }; 
            uint8_t blue = (uint8_t)el->attr.slider.pos;
            color &= 0xFFFFFF00;
            color |= blue;
            ser_write_msg_fifo((char*)&ss, sizeof(ss), SLIDER_SERIAL);
        }
        else if(!strcmp(el->identifier, "green")){
            slider_serial ss = { el->attr.slider.pos, 2 }; 
            uint8_t green = (uint8_t)el->attr.slider.pos;
            color &= 0xFFFF00FF;
            color |= green<<8;
            ser_write_msg_fifo((char*)&ss, sizeof(ss), SLIDER_SERIAL);
        }
        else{
            slider_serial ss = { el->attr.slider.pos, 3 }; 
            ser_write_msg_fifo((char*)&ss, sizeof(ss), SLIDER_SERIAL);
        }

        if(color_change)
            for(uint32_t i = 0; i<(30*2+10)*(30*2+10); i++)
                pixels[i] = color;
    }
    else if(type == SERIAL_DRAW){
		if(connected == false)
			return false;
        painter_serial *ps = data;

		uint32_t brush_size = find_by_id(wnd, "brush")->attr.slider.pos+1;
        uint32_t color = *(uint32_t*)find_by_id(wnd, "color")->attr.image.space;

        uint32_t x =  ps->x, y = ps->y;
        paint_with_brush(el, brush_size, x, y, color); 

        if(ps->delta_x == 0 && ps->delta_y == 0)
            return false;

        /* None of them are 0 */
        uint32_t steps_x = abs(ps->delta_x);
        uint32_t steps_y = abs(ps->delta_y);
        bool upwards = (ps->delta_y > 0); 
        bool right = (ps->delta_x > 0); 

		if(right)
			steps_x = (x + steps_x > el->width) ? (el->width-x-1) : steps_x;
		else
			steps_x = (x < steps_x) ? x : steps_x;

		if(upwards)
			steps_y = (y < steps_y) ? y : steps_y;
		else
			steps_y = (y + steps_y > el->height) ? (el->height-y-1) : steps_y;

        for(; steps_x || steps_y; ){
                if(steps_x){
                    if(right) x++;
                    else x--;
                    steps_x--;
                }
                if(steps_y){
                    if(upwards) y--;
                    else y++;
                    steps_y--;
                }
            
                paint_with_brush(el, brush_size, x, y, color); 
        }
    }
    else if(type == SLIDER_SERIAL){
		if(connected == false)
			return false;

        bool color_change = true;
        slider_serial *ss = data;
        uint32_t color = *(uint32_t*)find_by_id(wnd, "color")->attr.image.space;
        if(ss->id == 0){
            find_by_id(wnd, "red")->attr.slider.pos = ss->new_pos;
            uint8_t red = (uint8_t)ss->new_pos;
            color &= 0xFF00FFFF;
            color |= red<<16;
        }
        else if(ss->id == 1){
            find_by_id(wnd, "blue")->attr.slider.pos = ss->new_pos;
            uint8_t blue = (uint8_t)ss->new_pos;
            color &= 0xFFFFFF00;
            color |= blue;
        }
        else if(ss->id == 2){
            find_by_id(wnd, "green")->attr.slider.pos = ss->new_pos;
            uint8_t green = (uint8_t)ss->new_pos;
            color &= 0xFFFF00FF;
            color |= green<<8;
        }
        else{
            find_by_id(wnd, "brush")->attr.slider.pos = ss->new_pos;
        }
        uint32_t *pixels = find_by_id(wnd, "color")->attr.canvas.space;
        if(color_change)
            for(uint32_t i = 0; i<(30*2+10)*(30*2+10); i++)
                pixels[i] = color;

    }
	else if(type == CLOSE_MSG){
		if(connected){
			static uint8_t hello[8] = {0,0,0,0,0,0,0,0};
			ser_write_msg_fifo((char*)hello, 8, SERIAL_GOODBYE);
		}
		ser_set_handler(NULL, NULL, NULL);
		connected = false;
		expecting = false;
	}
	else if(type == CHECKBOX_MSG){
		expecting = el->attr.checkbox.enabled;
	}
	else if(type == BUTTON_MSG){
		if(connected)
			return false;
		find_by_id(wnd, "connected")->attr.text.active = false;
		static uint8_t hello[8] = {0x10, 0x12, 0x13, 0x14, 0x15, 0x16};
        ser_write_msg_fifo((char*)hello, 8, SERIAL_HELLO);
		expecting = true;
	}
	else if(type == SERIAL_HELLO){
		/* Not the host */
		if(!find_by_id(wnd, "host")->attr.checkbox.enabled)
			return false;
		static uint8_t hello[8] = {0,0,0,0,0,0,0,0};
        ser_write_msg_fifo((char*)hello, 8, SERIAL_HELLO_RESPONSE);
		connected = true;
		find_by_id(wnd, "connected")->attr.text.active = true;
		find_by_id(wnd, "red")->attr.slider.pos = 0;
		find_by_id(wnd, "blue")->attr.slider.pos = 0;
		find_by_id(wnd, "green")->attr.slider.pos = 0;
		find_by_id(wnd, "brush")->attr.slider.pos = 0;
		memset(find_by_id(wnd, "color")->attr.image.space, 0, (30*2+10)*(30*2+10)*4);
		memset(find_by_id(wnd, "canvas")->attr.canvas.space, 0xFF, (500)*(500)*4);

	}
	else if(type == SERIAL_HELLO_RESPONSE){
		connected = expecting;
		find_by_id(wnd, "connected")->attr.text.active = connected;
		find_by_id(wnd, "red")->attr.slider.pos = 0;
		find_by_id(wnd, "blue")->attr.slider.pos = 0;
		find_by_id(wnd, "green")->attr.slider.pos = 0;
		find_by_id(wnd, "brush")->attr.slider.pos = 0;
		memset(find_by_id(wnd, "color")->attr.image.space, 0, (30*2+10)*(30*2+10)*4);
		memset(find_by_id(wnd, "canvas")->attr.canvas.space, 0xFF, (500)*(500)*4);
	}
	else if(type == SERIAL_GOODBYE){
		connected = false;
		expecting = false;
		find_by_id(wnd, "connected")->attr.text.active = false;
	}
	else if(type == MAXIMIZE_MSG){
		wnd->maximized = false;
		return  true;
	}

    return false;
}
