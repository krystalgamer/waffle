#include <lcom/lcf.h>
#include "letters.h"

static uint8_t * font_sprites = NULL;
static uint8_t bytes_per_pixel;

int initLetters() {

    /* Check if already was initialized */
    static bool hasInit = false;
    if (hasInit)
        return OK;

    /* Get bytes per pixel from vbe */
    bytes_per_pixel = get_bytes_per_pixel();

    /* Allocate memory for the font sprites */
    /* TODO try to put them closer*/
    font_sprites = malloc(FONT_WIDTH * FONT_HEIGHT * bytes_per_pixel * FONT_SYMBOLS_NUMBER);

    /* Load the entire font */
    xpm_image_t img;
    uint8_t * sprite = xpm_load(font_xpm, XPM_8_8_8_8, &img);
    if (sprite == NULL)
        return 1;

    /* Iterate over the font and initialize each symbol */
    for (int symbol_offset = 0; symbol_offset < FONT_SYMBOLS_NUMBER; symbol_offset++){
         /* Iterate lines */
        for(int i = 0; i < FONT_HEIGHT; i++){
            memcpy(&font_sprites[symbol_offset * FONT_WIDTH * FONT_HEIGHT * bytes_per_pixel] + (i*FONT_WIDTH) * bytes_per_pixel, sprite + (i*img.width + symbol_offset * FONT_WIDTH) * bytes_per_pixel, bytes_per_pixel*FONT_WIDTH);
        }
    }

    /*switcharoo que o mario se enganou*/
    char tmp[FONT_WIDTH*FONT_HEIGHT*4];
    memcpy(tmp, &font_sprites[('.'-STARTING_SYMBOL_OFFSET) * FONT_WIDTH * FONT_HEIGHT * bytes_per_pixel], FONT_HEIGHT*FONT_WIDTH*bytes_per_pixel);

    memcpy(&font_sprites[('.'-STARTING_SYMBOL_OFFSET) * FONT_WIDTH * FONT_HEIGHT * bytes_per_pixel], &font_sprites[('-'-STARTING_SYMBOL_OFFSET) * FONT_WIDTH * FONT_HEIGHT * bytes_per_pixel], FONT_HEIGHT*FONT_WIDTH*bytes_per_pixel);

    memcpy(&font_sprites[('-'-STARTING_SYMBOL_OFFSET) * FONT_WIDTH * FONT_HEIGHT * bytes_per_pixel], tmp, FONT_HEIGHT*FONT_WIDTH*bytes_per_pixel);


    free(sprite);

    hasInit = true;

    return OK;
}


int printSymbol(char symbol, uint16_t x, uint16_t y, uint32_t color) {

    /* Calculate offset of the symbol on the font_sprites array, based on ascii table order */
    int symbol_offset;
    symbol_offset = symbol - STARTING_SYMBOL_OFFSET;
    if (symbol_offset < 0 || symbol_offset > FONT_SYMBOLS_NUMBER) {
        printf("(%s) Tried to print symbol with index in Ascii Table: %d\n", __func__, symbol);
        return 1;
    }

    /* Draw the font symbol */
    draw_pixmap_direct_mode(&font_sprites[symbol_offset*FONT_WIDTH*FONT_HEIGHT*bytes_per_pixel], x, y, FONT_WIDTH, FONT_HEIGHT, color, true);

    return OK;
}

int printHorizontalWord(char * word, uint16_t x, uint16_t y, uint32_t color) {

    if(word == NULL)
        return 1;
    
    /* Print each symbol of the word in the correct position */
    for(int i = 0; word[i]; i++){
        if(word[i] == 32)//skip spaces
            continue;
        if (printSymbol(word[i], x + i*FONT_WIDTH, y, color) != OK){
            printf("(%s) There was an error while printing symbol %d\n", __func__, word[i]);
            return 1;
        }
    }
    return OK;
}

int print_horizontal_word_len(char *word, uint32_t len, uint16_t x, uint16_t y, uint32_t color){

    if(word == NULL)
        return 1;

    /* Print each symbol of the word in the correct position */
    for(int i = 0; len && word[i]; i++,len--){
        if(word[i] == 32)//skip spaces
            continue;
        if (printSymbol(word[i], x + i*FONT_WIDTH, y, color) != OK){
            printf("(%s) There was an error while printing symbol %d\n", __func__, word[i]);
            return 1;
        }
    }
    return OK;

}

int printVerticalWord(char * word, uint16_t x, uint16_t y, uint32_t color) {
    
    /* Print each symbol of the word in the correct position */
    for(int i = 0; word[i]; i++){
        if(word[i] == 32)//skip spaces
            continue;
        if (printSymbol(word[i], x, y + i*FONT_HEIGHT, color) != OK){
            printf("(%s) There was an error while printing symbol %d\n", __func__, word[i]);
            return 1;
        }
    }
    return OK;
}
