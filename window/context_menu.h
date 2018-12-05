
ContextMenu *create_context_menu(uint32_t entries);
bool add_context_menu_entry(ContextMenu *menu, const char *text, bool is_callback, void *third);
void draw_context_menu(ContextMenu *menu, uint32_t x, uint32_t y);
