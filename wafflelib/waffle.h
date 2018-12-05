/* Documentation of the Waffle library */

typedef enum{
    MSG_HI = 0,
    INVALID

}WaffleMessageType;

typedef struct __attribute__((packed)){
    uint16_t type;
    uint32_t pid;
    uint8_t pad[50];
}MsgWaffleHi;


