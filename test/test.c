#include <sys/types.h>
#include <sys/wait.h>
#include <minix/endpoint.h>
#include <minix/callnr.h>
#include <minix/ipc.h>
#include <minix/ds.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <lib.h>
#include <string.h>

int main()
{
    FILE *fp = fopen("/home/lcom/waffle_endpoint", "rb");

    if(fp == NULL){
        printf("No endpoint available\n");
        return 1;
    }

    endpoint_t endpoint;
    fread(&endpoint, sizeof(endpoint_t), 1, fp);

    message msg;
    memset(&msg, 0, sizeof(message));
    printf("%d result\n", ipc_sendrec(endpoint, &msg));

    fclose(fp);
    return 0;
}
