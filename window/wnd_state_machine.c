#include <lcom/lcf.h>
#include "window.h"


/*TODO so ve alteracoes no estado de pressionado nao ve levantamentos*/

static uint32_t state = 0; /* Initial state is all keys up */
uint32_t update_state(const struct packet *pp){

    uint32_t new_state = 0;
    if(pp->lb) new_state |= L_BUTTON;
    if(pp->mb) new_state |= M_BUTTON; /* Ignored for now */
    if(pp->rb) new_state |= R_BUTTON;
    
    uint32_t state_change = state ^ new_state; /* Get the differences */
    state = new_state;
    
    Event event = 0;
    
    event |= (state & L_BUTTON ? L_PRESSED : L_DEAD) | (state_change & L_BUTTON ? 0 : L_KEPT);
    event |= (state & M_BUTTON ? M_PRESSED : M_DEAD) | (state_change & M_BUTTON ? 0 : M_KEPT);
    event |= (state & R_BUTTON ? R_PRESSED : R_DEAD) | (state_change & R_BUTTON ? 0 : R_KEPT);

    return event;
}
