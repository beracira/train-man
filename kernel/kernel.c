#include "../io/include/bwio.h"
#include "../io/include/ts7200.h"
#include "kernel.h"
#include "syscall.h"
#include "functions.h"
#include "priorityqueue.h"
#include "td.h"
#include "user_syscall.h"
#include "irq.h"
#include "clockserver.h"
#include "io.h"
#include "courier.h"
#include "train_ui.h"
#include "sensors.h"
#include "stop.h"
#include "path_finding.h"

int activate(void);

unsigned int counter = 0;
int err = 0;
int active = 0;
int stop = 0;
int stop_time = 0;

void initialize(void) {

  asm("MRC p15, 0, r0, c1, c0, 0"); // read c1
  asm("AND r0, r0, #0xffffefff"); // clear bits to be cleared
  asm("ORR r0, r0, #0x00001000"); // set bits to be set
  asm("MCR p15, 0, r0, c1, c0, 0"); // write c1

  asm("MRC p15, 0, r0, c1, c0, 0"); // read c1
  asm("AND r0, r0, #0xfffffffb"); // clear bits to be cleared
  asm("ORR r0, r0, #0x00000004"); // set bits to be set
  asm("MCR p15, 0, r0, c1, c0, 0"); // write c1

  counter = 0;
  err = 0;

  // Load label for swi entry
  asm("ldr r0, =activate_enter_kernel;"); 
  asm("mov r1, #0x28;"); 
  asm("str r0, [r1, #0];");
  asm("ldr r0, =activate_enter_kernel_irq;"); 
  asm("mov r1, #0x38;"); 
  asm("str r0, [r1, #0];");

  int first_tid = td_add(firsttask, P_FIRST_TASK, 0);
  set_active(first_tid);
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  ks->num_tasks = 1;

  init_queue();

  IO_init();
  train_velocity_init();
  train_init();
  courier_ready = 0;
  ui_ready = 0;
  stop = 0;
  stop_time = 0;
  sensor_requested = 0;
  time_to_next_sensor = 0;
  train_64_struct.train_number = 64;
  train_64_struct.cur_sensor = 26;
  officer_struct.train_number = 63;
  officer_struct.cur_sensor = 22;
  RUNNING_TRAIN = 64;
  SENSOR_TID = -1;
  // int i = 0;
  // while (i++ < 100000);
  // bwsetfifo(COM1, OFF);
  //bwprintf(COM2, "\n\r");
}

int activate(void) {

  // exiting kernel
  asm("mov	ip, sp;");
	asm("stmfd	sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr};"); // save svc state
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("ldr r8, [r9, #12];");    // load spsr
  asm("and r8, r8, #0xffffff7f");   // clear interrupt 
  // asm("orr r8, r8, #0x00000080");   // set interrupt
  asm("msr spsr, r8;");         // change spsr

  asm("ldr r0, [r9, #8];");     // load return value to r0
  asm("ldr lr, [r9, #16];");     // load lr_svc
  asm("msr CPSR_c, #0xdf;");    // switch to system mode
  asm("ldr sp, [r9, #0];");     // load sp
  
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  if (ks->irq == 1) {
    ks->irq = 0;
    asm("ldmfd  sp, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr};");
    asm("ldmfd  sp!, {ip}");
    asm("msr CPSR_c, #0xd3;");    // back to supervisor mode
    asm("sub lr, lr, #4");
    asm("msr CPSR_c, #0xdf;");    // switch to system mode
  } else if (ks->started == 1) {
    asm("ldmfd  sp, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp};");
  } else {
    ks->started = 1;
  }

  asm("msr CPSR_c, #0xd3;");    // back to supervisor mode
  asm("movs pc, lr;");          // let it go  

  asm("activate_enter_kernel_irq:");

  asm("msr CPSR_c, #0xDF;");    // go into system mode
  asm("stmfd  sp!, {ip};");
  asm("mov  ip, sp;");
  asm("stmfd  sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr};"); // save usr state
  // handle should happen examine the irq code, handle it accordingly

  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("msr CPSR_c, #0xd2;");      // switch to IRQ mode
  asm("mrs r7, spsr;");      //  save spsr to be transfered to svc mode
  asm("mov r8, lr");            // move irq_lr to r8
  asm("msr CPSR_c, #0xd3;");      // switch to svc mode
  asm("mov lr, r8");          // transfer lr from IRQ to SVC
  asm("msr spsr, r7;");      // transfer spsr from IRQ to SVC
  asm("mov r8, #1;");
  asm("str r8, [r9, #20];");     // set ks->irq = 1

  // entering kernel
  asm("activate_enter_kernel:");
  asm("mov r9, #0x01300000;");  // r9 = KERNEL_STACK_START
  asm("str lr, [r9, #16];");     // save lr_svc to stack
  asm("msr CPSR_c, #0xDF;");      // switch to system mode
  asm("str sp, [r9, #0];");     // store sp
  asm("str lr, [r9, #4];");     // save lr to stack
  asm("msr CPSR_c, #0xD3;");      // back to svc
  asm("mrs r8, spsr;");         // move spsr to r8
  asm("str r8, [r9, #12];");    // store spsr
  //asm("mov sp, r9;");           // move kernel stack pointer to sp
  asm("ldmfd	sp, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr};"); // loads svc state
  asm("ldr r0, [lr,#-4];");     // load swi code 
  asm("bic r0, r0, #0xff000000;");  // get number

  if (ks->irq) ks->syscall_code = 0;
  //asm("bl handle;");
  //get r0 into an int

  return 0;
}

int handle(int num) {
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  (void) td;
  num = ks->syscall_code;

  // NOTE: can't declare variables in switch statement; do it up here

  volatile int * int_enable2 = (int *) (VIC2_BASE + VICxIRQStatus);
  volatile int timer_reg = ((*int_enable2 >> (51 - 32)) & 1);
  // int * uart1_clear = 0;
  // int * uart2_clear = 0;

  volatile int *data1, *data2;
  volatile int flags;
  data1 = (int *)( UART1_BASE + UART_DATA_OFFSET );
  data2 = (int *)( UART2_BASE + UART_DATA_OFFSET );
  char c = 0;
  
  switch(num){
    case IRQ:

     // irq_bit = (*int_enable >> (51 - 32)) & 1;
      // Timer
      //if ((*int_enable2 >> (51 - 32)) & 1) {
    	if (timer_reg) {
      	// bwprintf(COM2, "before clear -  - timer reg %d .\n\r", timer_reg);
        irq_clear_timer();
        if (io_ready && time_ticks % 10 == 0) {
          update_time();
          time_to_next_sensor -= 1;
          if (time_to_next_sensor < 0) time_to_next_sensor = 0;
        }
        if (courier_ready) {
          wake_train();
          wake_train_second_part();
        }
        if (io_ready && ui_ready && time_ticks % 20 == 0) {
          // Putc(1, 128 + 5);
          if (SENSOR_TID != -1 && td[SENSOR_TID].state == SENSOR_BLOCKED_1) td[SENSOR_TID].state = READY;
          sensor_requested = 1;
        }
        if (io_ready && ui_ready && time_ticks % 20 == 10) {
          if (SENSOR_TID != -1 && td[CR_TID].state == GET_CHAR_BLOCKED) {
            td[CR_TID].state = READY;
            train_wait_list_ptr->tail -= 1;
            train_wait_list_ptr->tail %= IO_BUFFER_SIZE;
          }
        }
        if (io_ready && ui_ready && time_ticks % 10 == 5) {
          remove_from_stop_queue();
        }
        ++time_ticks;
        remove_delay_list();
      }
      else {
        flags = *((int *) 0x808c001c); // COM1 
        if ((flags & 0x2) && (flags & 0x4)) {
          printf(2, "error 02 04\n\r");
        }
        if (flags & 0x2) {
          c = *data1;
          buffer_add(TRAIN_RECEIVE, c);
        } else if (flags & 0x4 || flags & 0x1) {
          buffer_remove(TRAIN_SEND);

          // if(flags & 0x4) {
          //   //Deactivate the receive interrupt
          //   int * ctlr, buf;
          //   ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
          //   buf = *ctlr;
          //   *ctlr = buf & ~TIEN_MASK;           
          // }
          
          if(flags & 0x1) {
            //clear modem status interrupt
            *((int *) 0x808c001c) = 0;      
            
            //Deactivate the modem status interrupt
            int * ctlr, buf;
            ctlr = (int *)( UART1_BASE + UART_CTLR_OFFSET );
            buf = *ctlr;
            *ctlr = buf & ~MSIEN_MASK;
          }
        } else {

        }

        flags = *((int *) 0x808d001c); // COM2
        if (flags == 2 || flags == 3) {
          c = *data2;
          buffer_add(TERMINAL_RECEIVE, c);
        } else if (flags >= 4) {
          buffer_remove(TERMINAL_SEND);
        }
      }


      break;
    case CREATE:
      ks->usr_r0 = kernel_Create(ks->args[0], (void *) ks->args[1] );
      break;
    case MYTID:
      ks->usr_r0 = kernel_MyTid();
      break;
    case MYPARENTPID:
      ks->usr_r0 =  kernel_MyParentTid();
      break;
    case PASS:
      ks->usr_r0 = kernel_Pass();
      break;
    case EXIT:
      ks->usr_r0 = kernel_Exit();
      break;
    case SEND:
      ks->usr_r0 = kernel_Send((int) ks->args[0], (void *) ks->args[1], (int) ks->args[2], (void *) ks->args[3], (int) ks->args[4]);
      break;
    case REPLY:
      ks->usr_r0 = kernel_Reply((int) ks->args[0], (void *) ks->args[1], (int) ks->args[2]);
      break;
    case RECEIVE:
      ks->usr_r0 = kernel_Receive((int *) ks->args[0], (void *) ks->args[1], (int) ks->args[2]);
      break;
  }
  return 100;
}

int main( int argc, char* argv[] ) {
  (void) argc;
  (void) argv;

  initialize();
  volatile struct task_descriptor * td = (struct task_descriptor *) TASK_DESCRIPTOR_START;
  volatile struct kernel_stack * ks = (struct kernel_stack *) KERNEL_STACK_START;
  (void) td;
  (void) ks;

  while(1 + 1 == 2) {
    active = schedule();
    if (active == -1) return 0;
    set_active(active);

    // if (active != 8) {
    //   printf( 2, "activate: %d\n\r", active);
    // }
    int request = activate(); //active);
    // printf(2, "\033[s\n\ractivate: %d\033[u", active);


    (void) request;
    handle(0);
    // bwprintf( COM2, "handle: %d\n\r", active);
    sync_td(active);
  }

  return 0;
}

