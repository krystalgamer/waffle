#ifndef VBE_H
#define VBE_H

/* BIOS Services */
#define VIDEO_CARD_SRV 0x10
#define PC_CONF_SRV 0x11
#define MEMORY_CONF_SRV 0x12
#define KEYBOARD_SRV 0x16

/* VBE functions */
#define RETURN_VBE_CONTROLLER_INFO 0x00
#define RETURN_VBE_MODE_INFO 0x01
#define SET_VBE_MODE 0x02
#define RETURN_CURRENT_MODE_INFO 0x03

/* VBE function return in AH*/
#define FUNC_SUCCESS 0x00
#define FUNC_CALL_FAILED 0x01 
#define FUNC_SUPPORTED 0x4F
#define FUNC_NOT_SUPPORTED 0x02
#define FUNC_INVALID 0x03
#define FUNC_RETURN_OK 0x004F
/* VBE function return in AL*/ 
#define VBE_FUNC 0x4F

/* Graphics Mode */
#define R1024x768_INDEXED 0x105
#define R40x480_DIRECT 0x110
#define R800x600_DIRECT 0x115 
#define R1280x1024_DIRECT 0x11A 
#define R1152x864_DIRECT 0x14C

/* Mode Attributes*/
#define MODE_SUPPORTED_HARDWARE BIT(0)
#define TTY_FUNCS_SUPPORTED_BIOS BIT(2)
#define COLOR_MODE BIT(3)
#define GRAPHICS_MODE BIT(4)

/* Set VBE Mode */
#define LINEAR_FRAME_BUFFER BIT(14)

/* Memory Model */
#define INDEXED_COLOR_MODE 0x04
#define DIRECT_COLOR_MODE 0x06

typedef struct __attribute__((packed)) {
  char VbeSignature[4];
  uint16_t VbeVersion;
  uint32_t OemStringPtr;
  uint32_t Capabilities;
  uint32_t VideoModePtr;
  uint16_t TotalMemory;
  uint16_t OemSoftwareRev;
  uint32_t OemVendorNamePtr;
  uint32_t OemProductNamePtr;
  uint32_t OemProductRevPtr;
  uint8_t Reserved[222];
  uint8_t OemData[256];
} VbeInfoBlock;

/**
 * @brief Sets the specified video mode
 * 
 * @param mode Video mode to set
 * @return Return 0 upon success and non-zero otherwise
 */
int set_video_mode(uint16_t mode);

/**
 * @brief Set desired graphics mode, maps VRAM to the process' address space and initializes the vbe_mode_info_t struct
 * 
 * @param mode Video mode to set
 * @return Return 0 upon success and non-zero otherwise
 */
void* (vg_init)(uint16_t mode);

/**
 * @brief Returns information on the specified video mode, initializing the parameter struct
 * 
 * @param mode Video mode to set
 * @param vmi_p Pointer to vbe_mode_info_t struct to initialize
 * @return Return 0 upon success and non-zero otherwise
 */
int vbe_get_mode_info_2(uint16_t mode, vbe_mode_info_t * vmi_p);

/**
 * @brief Draws the pixmap with given dimensions starting from coordinates x and y
 * 
 * @param pixmap Pixmap to draw
 * @param x Top left corner coordinate along the x axis
 * @param y Top left corner coordinate along the y axis
 * @param width Size in pixels of the pixmap along the x axis
 * @param height Size in pixels of the pixmap along the y axis
 */
void draw_pixmap(const char * pixmap, uint16_t x, uint16_t y, int width, int height);

/**
 * @brief Clears the specified buffer with the given color
 * 
 * @param buffer Buffer to clear
 * @param color Color to clear with
 */
void (clear_buffer)(uint8_t color);

/**
 * @brief Draws a pixmap on the specified buffer at given coordinates
 * 
 * @param pixmap Pixmap to draw
 * @param x Top left corner coordinate along the x axis
 * @param y Top left corner coordinate along the y axis
 * @param width Size in pixels of the pixmap along the x axis
 * @param height Size in pixels of the pixmap along the y axis
 * @param buffer Buffer to draw to
 */
void draw_pixmap_on(const char *pixmap, uint16_t x, uint16_t y, int width, int height, uint8_t * buffer);

/**
 * @brief Swaps the mapped memory to the specified buffer
 * 
 */
void swap_buffers();

/**
 * @brief Returns the color of specified position following a pre-determined pattern
 * 
 * @param first Color of first rectangle
 * @param row Row of position
 * @param col Column of position
 * @param step Color increment
 * @param no_rectangles Number of rectangels in the pattern
 * @return Color of specified position in the pattern
 */
uint32_t get_pattern_color(uint32_t first, uint8_t row, uint8_t col, uint8_t step, uint8_t no_rectangles);

/**
 * @brief Draws a pre-determined pattern on screen
 * 
 * @param width Horizontal size
 * @param height Vertical size
 * @param no_rectangles Number of rectangels in the pattern
 * @param first Color of first rectangle
 * @param step Color increment
 * @return Return 0 upon success and non-zero otherwise
 */
int draw_pattern(uint16_t width, uint16_t height, uint8_t no_rectangles, uint32_t first, uint8_t step);

/**
 * @brief Retries lm_alloc 5 times since it can fail sometimes, sleeps in between for 1 second
 * @param size of the alloc
 * @param mmap mmap_t structure
 * @return the result of lm_alloc wether it failed or not
 */
void *retry_lm_alloc(size_t size, mmap_t *mmap);

int pj_draw_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, uint32_t color);

/* Methods to return information from the vbe_mode_info_t struct */

/**
 * @brief Returns the number of bits per pixel
 * @return the number of bits per pixel
 */
uint8_t get_bits_per_pixel();

/**
 * @brief Gets x res of screen 
 * @return x res
 */
uint16_t get_x_res();

/**
 * @brief Gets y res of screen 
 * @return y res
 */
uint16_t get_y_res();

/**
 * @brief Gets the memory model 
 * @return memory model
 */
uint8_t get_memory_model();

/**
 * @brief Gets red mask size
 * @return red mask size
 */
uint8_t get_red_mask_size();

/**
 * @brief Gets red field position
 * @return red field position
 */
uint8_t get_red_field_position();

/**
 * @brief Gets blue mask size
 * @return blue mask size
 */
uint8_t get_blue_mask_size();

/**
 * @brief Gets blue field position
 * @return blue field position
 */
uint8_t get_blue_field_position();

/**
 * @brief Gets green mask size
 * @return green mask size
 */
uint8_t get_green_mask_size();

/**
 * @brief Gets green field position
 * @return green field positon
 */
uint8_t get_green_field_position();

/**
 * @brief Gets reserved mask size
 * @return reserved mask size
 */
uint8_t get_rsvd_mask_size();

/**
 * @brief Gets reserved field position
 * @return reserved field position
 */
uint8_t get_rsvd_field_position();

/**
 * @brief Enumeration that contains possible error codes for the function to ease development and debugging
 *
 */
typedef enum _vbe_status {
	VBE_OK = OK,
    VBE_NOT_OK,

	VBE_LM_ALLOC_FAILED,
	VBE_SYS_INT86_FAILED,
	VBE_INVALID_RETURN,

	VBE_INVALID_COORDS,
	VBE_OUT_OF_BOUNDS,

	VBE_INVALID_COLOR_MODE,
	VBE_DRAW_LINE_FAILED


} vbe_status;

/**
 * @brief Draws a font symbol on the backbuffer
 *
 * @param symbol Array of pixels to draw
 * @param x X position of upper left corner
 * @param y Y position of upper left corner
 * @param width Horizontal size of the symbol
 * @param height Vertical size of the symbol
 * @param color to draw symbol
 * @param fixedColor Bool indicating if should draw with specified color
 */
void draw_pixmap_direct_mode(uint8_t * symbol, uint16_t x, uint16_t y, int width, int height, uint32_t color, bool fixedColor);


/**
 * @brief Returns the number of bytes per pixel of the current vbe mode
 *
 * @return Number of bytes per pixel
 */
uint8_t get_bytes_per_pixel();

/** 
 * @brief Clears the backbuffer with a 32-bit color 
 * @param color The color
 */
void clear_buffer_four(uint32_t color);

/**
 * @brief Draws an horizontal line starting at x and y
 * @param x x pos
 * @param y y pos
 * @param len lenght of the line
 * @param color lenght of the line
 *
 * @return Error code if any when drawing
 */
int pj_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

/**
 * @brief Draws a vertical line starting at x and y
 * @param x x pos
 * @param y y pos
 * @param len lenght of the line
 * @param color lenght of the line
 *
 * @return Error code if any when drawing 
 */
int pj_draw_vline(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

/**
 * @brief Draws a background to the backbuffer
 * @param bckg The buffer containing the background
 * @param width The width of the background 
 * @param height The height of the background 
 */
void draw_background(uint8_t * bckg, int width, int height);

#endif
