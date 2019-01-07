/**
 * @defgroup context_menu context_menu module
 * @ingroup window
 * Contains all ContextMenu code 
 * @{
 */

/**
 * @brief creates a context menu
 * @param entries number of entries
 * @return the created context menu
 */
ContextMenu *create_context_menu(uint32_t entries);

/**
 * @brief adds a entry to a context menu
 * @param menu the menu to be changed
 * @param text the text of the entry
 * @param is_callback whetjer it is a callback
 * @param third depends on the last one
 * @return true if success
 */
bool add_context_menu_entry(ContextMenu *menu, const char *text, bool is_callback, void *third);

/**
 * @brief Draws the context menu
 * @param menu the menu
 * @param x x position
 * @param y y positon
 */
void draw_context_menu(ContextMenu *menu, uint32_t x, uint32_t y);

/** @} */
