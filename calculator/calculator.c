#include <lcom/lcf.h>
#include "../window/window.h"

/** @addtogroup calculator
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
bool calculator_input_handler(Element *el, unsigned type, void *data, Window *wnd);
/** @} */

void create_calculator(){
    uint32_t wnd_width = 300, wnd_height = 280;

    uint32_t wnd_id = create_window(wnd_width, wnd_height, 0x008A8A8A, "Calculator", &calculator_input_handler);

	static struct _button_attr soma = { "+", 0x007A7A7A, 0x003A3A3A};
    window_add_element(wnd_id, BUTTON, 0, 0, 60, 60, (void*)&soma, "+");

	static struct _button_attr menos = { "-", 0x007A7A7A, 0x003A3A3A};
    window_add_element(wnd_id, BUTTON, 60, 0, 60, 60, (void*)&menos, "-");

	static struct _button_attr div = { "/", 0x007A7A7A, 0x003A3A3A};
    window_add_element(wnd_id, BUTTON, 120, 0, 60, 60, (void*)&div, "/");

	static struct _button_attr mul = { "x", 0x007A7A7A, 0x003A3A3A};
    window_add_element(wnd_id, BUTTON, 180, 0, 60, 60, (void*)&mul, "x");


	static struct _button_attr zero = { "0", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 0, 60, 60, 60, (void*)&zero, "0");

	static struct _button_attr um = { "1", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 60, 60, 60, 60, (void*)&um, "1");

	static struct _button_attr dois = { "2", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 120, 60, 60, 60, (void*)&dois, "2");

	static struct _button_attr tres = { "3", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 180, 60, 60, 60, (void*)&tres, "3");


	static struct _button_attr quatro = { "4", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 0, 120, 60, 60, (void*)&quatro, "4");

	static struct _button_attr cinco = { "5", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 60, 120, 60, 60, (void*)&cinco, "5");

	static struct _button_attr seis = { "6", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 120, 120, 60, 60, (void*)&seis, "6");

	static struct _button_attr sete = { "7", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 180, 120, 60, 60, (void*)&sete, "7");


	static struct _button_attr oito = { "8", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 0, 180, 60, 60, (void*)&oito, "8");

	static struct _button_attr nove = { "9", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 60, 180, 60, 60, (void*)&nove, "9");

	static struct _button_attr igual = { "=", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 120, 180, 120, 60, (void*)&igual, "=");

	static struct _button_attr clear = { "C", 0x007a7a7a, 0x003a3a3a};
    window_add_element(wnd_id, BUTTON, 240, 0, 60, 240, (void*)&clear, "C");

	int *valor = malloc(sizeof(int));
	*valor = 0;
	
	char *texto = malloc(50);
	memset(texto, 0, 50);
	
    struct _text_attr text = { texto, 0xFFFFFFFF, true};
    window_add_element(wnd_id, TEXT, 0, 240, 0, 0, (void*)&text, "text");


    window_add_element(wnd_id, DATA, 0, 120, 0, 0, (void*)&valor, "valor");
	void *op = malloc(1);
    window_add_element(wnd_id, DATA, 0, 120, 0, 0, (void*)&op, "op");
}

bool calculator_input_handler(Element *el, unsigned type, void *data, Window *wnd){
	
	printf("", el, type, data, wnd);
	if(type == BUTTON_MSG){

		if(!strcmp(el->identifier, "x") || !strcmp(el->identifier, "+") || !strcmp(el->identifier, "-") || !strcmp(el->identifier, "/") || !strcmp(el->identifier, "=")){

			if(!strcmp(el->identifier, "=")){
				int val2 = 0; 			
				sscanf(find_by_id(wnd, "text")->attr.text.text, "%d", &val2);

				char *op = find_by_id(wnd, "op")->attr.data.space;

				if(*op == '+')
					sprintf(find_by_id(wnd, "text")->attr.text.text, "%d", val2+*(uint32_t*)find_by_id(wnd, "valor")->attr.data.space);
				else if(*op == '-')
					sprintf(find_by_id(wnd, "text")->attr.text.text, "%d", *(uint32_t*)find_by_id(wnd, "valor")->attr.data.space - val2);
				else if(*op == 'x')
					sprintf(find_by_id(wnd, "text")->attr.text.text, "%d", val2 * *(uint32_t*)find_by_id(wnd, "valor")->attr.data.space);
				else if(*op == '/'){
					if(val2 != 0)
						sprintf(find_by_id(wnd, "text")->attr.text.text, "%d", *(uint32_t*)find_by_id(wnd, "valor")->attr.data.space / val2);
					else
						sprintf(find_by_id(wnd, "text")->attr.text.text, "%d", 0);

				}
			}
			else{

				find_by_id(wnd, "op")->attr.data.space = el->identifier;
				int tmp = 0;
				sscanf(find_by_id(wnd, "text")->attr.text.text, "%d", &tmp);
				memset(find_by_id(wnd, "text")->attr.text.text, 0, 50);
				*(uint32_t*)find_by_id(wnd, "valor")->attr.data.space = tmp;
			}
		}
		else if(!strcmp(el->identifier, "C")){
			strcpy(find_by_id(wnd, "text")->attr.text.text, "0");
		}
		else{
			strcat(find_by_id(wnd, "text")->attr.text.text, el->identifier);
		}
		
	}
	else if(type == FREE_MSG){
		free(el->attr.data.space);
	}
	else if(type == MAXIMIZE_MSG){
		wnd->maximized = false;
		return true;
	}

	return false;
}
