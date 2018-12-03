typedef enum{
    MSG_HI = 0,
    MSG_INVALID

}WaffleMessageType;

typedef struct __attribute__((packed)){
    uint16_t type;
    uint8_t pad[54];

}MsgDefaultFormat;

typedef struct __attribute__((packed)){
    uint16_t type;
    uint32_t pid;
    uint8_t pad[50];
}MsgWaffleHi;
