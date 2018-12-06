// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lib.h>
#include "vbe.h"
#include "interrupts/mouse.h"
#include <minix/sysinfo.h>
#include "window/window.h"
#include "interrupts/timer_user.h"
#include <sys/types.h>
#include <unistd.h>
#include "font/letters.h"
#include "messages.h"

typedef void (*tmr_func_t)(int arg);
typedef struct minix_timer
{
  struct minix_timer	*tmr_next;	/* next in a timer chain */
  clock_t 		tmr_exp_time;	/* expiration time (absolute) */
  tmr_func_t		tmr_func;	/* function to call when expired */
  int			tmr_arg;	/* integer argument */
} minix_timer_t;

struct mproc {
  char mp_exitstatus;		/* storage for status when process exits */
  char mp_sigstatus;		/* storage for signal # for killed procs */
  char mp_eventsub;		/* process event subscriber, or NO_EVENTSUB */
  pid_t mp_pid;			/* process id */
  endpoint_t mp_endpoint;	/* kernel endpoint id */
  pid_t mp_procgrp;		/* pid of process group (used for signals) */
  pid_t mp_wpid;		/* pid this process is waiting for */
  vir_bytes mp_waddr;		/* struct rusage address while waiting */
  int mp_parent;		/* index of parent process */
  int mp_tracer;		/* index of tracer process, or NO_TRACER */

  /* Child user and system times. Accounting done on child exit. */
  clock_t mp_child_utime;	/* cumulative user time of children */
  clock_t mp_child_stime;	/* cumulative sys time of children */

  /* Real, effective, and saved user and group IDs. */
  uid_t mp_realuid;		/* process' real uid */
  uid_t mp_effuid;		/* process' effective uid */
  uid_t mp_svuid;		/* process' saved uid */
  gid_t mp_realgid;		/* process' real gid */
  gid_t mp_effgid;		/* process' effective gid */
  gid_t mp_svgid;		/* process' saved gid */

  /* Supplemental groups. */
  int mp_ngroups;		/* number of supplemental groups */
  gid_t mp_sgroups[NGROUPS_MAX];/* process' supplemental groups */

  /* Signal handling information. */
  sigset_t mp_ignore;		/* 1 means ignore the signal, 0 means don't */
  sigset_t mp_catch;		/* 1 means catch the signal, 0 means don't */
  sigset_t mp_sigmask;		/* signals to be blocked */
  sigset_t mp_sigmask2;		/* saved copy of mp_sigmask */
  sigset_t mp_sigpending;	/* pending signals to be handled */
  sigset_t mp_ksigpending;	/* bitmap for pending signals from the kernel */
  sigset_t mp_sigtrace;		/* signals to hand to tracer first */
  void *mp_sigact;	/* as in sigaction(2), pointer into mpsigact */
  vir_bytes mp_sigreturn; 	/* address of C library __sigreturn function */
  minix_timer_t mp_timer;	/* watchdog timer for alarm(2), setitimer(2) */
  clock_t mp_interval[3];	/* setitimer(2) repetition intervals */
  clock_t mp_started;		/* when the process was started, for ps(1) */

  unsigned mp_flags;		/* flag bits */
  unsigned mp_trace_flags;	/* trace options */
  message mp_reply;		/* reply message to be sent to one */

  /* Process execution frame. Both fields are used by procfs. */
  vir_bytes mp_frame_addr;	/* ptr to proc's initial stack arguments */
  size_t mp_frame_len;		/* size of proc's initial stack arguments */

  /* Scheduling priority. */
  signed int mp_nice;		/* nice is PRIO_MIN..PRIO_MAX, standard 0. */

  /* User space scheduling */
  endpoint_t mp_scheduler;	/* scheduler endpoint id */

  char mp_name[PROC_NAME_LEN];	/* process name */

  int mp_magic;			/* sanity check, MP_MAGIC */
};

// Any header files included below this line should have been created by you
bool pressed_the_secret_button = false;

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    lcf_log_output("/home/lcom/labs/proj/output.txt");

    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
    return 1;

    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();

    return 0;
}


int (proj_main_loop)(int argc, char *argv[]) {

    /* Suprimir warnings */
    if(argc == 0 && argv != NULL)
        printf("PENIS\n");

	printf("%d pid\n", getpid());

    struct mproc processes[256];
	printf("%d vou hcorar\n", getsysinfo(PM_PROC_NR, SI_PROC_TAB, &processes, sizeof(processes)));

    struct mproc *meu = NULL;
    for(int i = 0; i < 256; i++){
        if ((processes[i].mp_flags & 1) && processes[i].mp_pid == getpid()){
            meu = &processes[i];
            break;
        }
    }

    if(meu == NULL){
        printf("Couldn't find my endpoint, I'm killing myself\n");
        return 1;
    }

    FILE *fp = fopen("/home/lcom/waffle_endpoint", "wb+");
    if(fp == NULL){
        printf("Cant open my endpoint file\n");
        return 1;
    }

    if(fwrite(&(meu->mp_endpoint), sizeof(endpoint_t), 1, fp) != 1){
        printf("Im a loser that can write his own endpoint to a file\n");
        fclose(fp);
        return 1;
    }
    fclose(fp);

	
    /* Initialize graphics mode */

    /*Codigo do souto contem um mode cuidado meninas! */

    if(vg_init(0x14C) == NULL){
        printf("(%s) vg_init failed..quitting", __func__);
        return 1;
    }

    /* Initialize the font */
    if (initLetters() != OK) {
        printf("(%s) Error initializing the font", __func__);
        return 1;
    }

    /* Subscribe KBC Interrupts */
    uint8_t bitNum;
    if(mouse_subscribe_int(&bitNum) != OK) {
        vg_exit();
        return 1; 
    } 
    uint32_t irq_set = BIT(bitNum);


    /* Subscribe Timer 0 Interrupts */
    if(timer_subscribe_int(&bitNum) != OK) {
        printf("(%s) There was a problem enabling timer interrupts", __func__);
        vg_exit();
        return 1;
    }
    uint32_t timer_irq_set = BIT(bitNum);

    /* Variables to hold results */
    int ipc_status;
    message msg;
    uint32_t r = 0;
    
    /* Mask used to verify if a caught interrupt should be handled */
    uint8_t mouse_packet[MOUSE_PACKET_SIZE];
	struct packet pp;
    /* Keep receiving and handling interrupts until the ESC key is released. */

    int maxFrames = 2;
    int curFrame = 1;

    init_internal_status();
    create_window(200, 100, 0x0AAAAAA, "Janela Fixe");
    create_window(100, 200, 0x0AAAAAA, "Feia");
    uint32_t id_fixe = create_window(400, 300, 0x00AAAAAA, "Vonita");

    window_add_element(id_fixe, BUTTON, 20, 20, 50, 50, NULL);
    window_add_element(id_fixe, BUTTON, 80, 80, 20, 10, NULL);


    while(!pressed_the_secret_button) {
        /* Get a request message.  */
        if( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }

        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: /* hardware interrupt notification */
                if ( msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */

                    mouse_ih();
	                if (assemble_mouse_packet(mouse_packet)) {
                        parse_mouse_packet(mouse_packet, &pp);
						if(pp.mb)
							pressed_the_secret_button = true;
                        window_mouse_handle(&pp);
                    }	

                }

                if( msg.m_notify.interrupts & timer_irq_set){
                    if((++curFrame)%maxFrames == 0){
                        window_draw();
                        swap_buffers();
                    }


                }
                break;
            default:
                return 1;
                break; /* no other notifications expected: do nothing */
            }
        }
        else { /* received a standard message, not a notification */

            MsgDefaultFormat *dformat = (MsgDefaultFormat*)&msg.m_u32;
            if(msg.m_type == 46){
                    MsgWaffleHi *hi_msg = (void*)dformat;
                    printf("A process %d says hi!\n", hi_msg->pid);
                    ipc_send(msg.m_source, &msg);

            }
        }
    }


    timer_unsubscribe_int();
    /* Unsubscribe KBC Interrupts */
    if(mouse_unsubscribe_int() != OK) {
      vg_exit();
      return 1;
    }

    /* Returns to default Minix text mode */
    vg_exit();
    
    return 0;
  return 1;
}
