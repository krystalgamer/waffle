#include <minix/endpoint.h>
#include <minix/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "waffle.h"

static endpoint_t endpoint;

extern int waffle_main(int argc, char **argv);


static void say_hello(){

    /* Not really needed used mostly for debugging */
    message msg;
    memset(&msg, 0, sizeof(message));

    msg.m_type = MSG_HI;
    MsgWaffleHi *hi = (MsgWaffleHi*)&msg.m_u32;
    hi->pid = getpid();
    printf("foi isto %d\n", ipc_sendrec(endpoint, &msg));
}

int main(int argc, char **argv){
    
    FILE *fp = fopen("/home/lcom/waffle_endpoint", "rb");
    if(fp == NULL){
        printf("No endpoint available\n");
        return 1;
    }

    fread(&endpoint, sizeof(endpoint_t), 1, fp);
    fclose(fp);

    say_hello();

    return waffle_main(argc, argv);
}
