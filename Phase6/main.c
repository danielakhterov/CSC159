// SPEDE inclues
#include "k-include.h"
// Entries to kernal (TimerEntry, etc)
#include "k-entry.h"
// Kernel data types
#include "k-type.h"
// small handy functions
#include "k-lib.h"
// kernal service routines
#include "k-sr.h"
// all user process code here
#include "proc.h"
// all sys calls
#include "sys-call.h"

// kernal data are all here:
// current running PID; if -1, then none selected
int run_pid;
q_t pid_q, ready_q, sleep_q, mux_q;
// Process Control Blocks
pcb_t pcb[PROC_SIZE];
// process runtime stacks
char proc_stack[PROC_SIZE][PROC_STACK_SIZE];
// intr table's DRAM location
struct i386_gate * intr_table;
// system time in centi-sec, intialize to 0
int sys_centi_sec;

mux_t mux[MUX_SIZE]; 
int vid_mux = 0;

term_t term[TERM_SIZE] = {
  { TRUE, TERM0_IO_BASE },
  { TRUE, TERM1_IO_BASE }
}; 

// Initialize Kernel Data
void InitKernelData(void) {
    int i;

    sys_centi_sec = 0;

    // Get Intrupt Table
    intr_table = get_idt_base();

    // Clear all queues
    Bzero((char *)&pid_q, sizeof(q_t));
    Bzero((char *)&ready_q, sizeof(q_t));
    Bzero((char *)&sleep_q, sizeof(q_t));
    Bzero((char *)&mux_q, sizeof(q_t));

    // Initialize ids for pid and mux queue
    for(i = 0; i < 20; i++) {
        EnQ(i, &pid_q);
        EnQ(i, &mux_q);
    }

    // Set the current running pid to NONE
    run_pid = NONE;
}

// init kernal control
void InitKernelControl(void) {
    // fill out intr table for timer
    // fill_gate();

    // Interupt table for Timer Interupt
    fill_gate(&intr_table[TIMER_INTR], 
            (int)TimerEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    // Interupt table for GetPidCall
    fill_gate(&intr_table[GETPID_CALL], 
            (int)GetPidEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    // Interupt table for ShowCharCall
    fill_gate(&intr_table[SHOWCHAR_CALL], 
            (int)ShowCharEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    // Interupt table for SleepCall
    fill_gate(&intr_table[SLEEP_CALL], 
            (int)SleepEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    fill_gate(&intr_table[MUX_CREATE_CALL], 
            (int)MuxCreateEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    fill_gate(&intr_table[MUX_OP_CALL], 
            (int)MuxOpEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    fill_gate(&intr_table[TERM0_INTR], 
            (int)TERM0Entry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    fill_gate(&intr_table[TERM1_INTR], 
            (int)TERM1Entry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    fill_gate(&intr_table[FORK_CALL], 
            (int)ForkEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    fill_gate(&intr_table[WAIT_CALL], 
            (int)WaitEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    fill_gate(&intr_table[EXIT_CALL], 
            (int)ExitEntry, get_cs(), 
            ACC_INTR_GATE, 
            0);

    // mask out PIC for timer
    // outportb();
    outportb(PIC_MASK, MASK);
}

// Choose run_pid
void Scheduler(void) {
    // Ok/picked
    if(run_pid > 0) {
        return;
    }

    // if ready_q is empty:
    // pick 0 as run_pid; pick InitProc
    // else
    // change state of PID 0 to ready
    // dequeue ready_q to set run_pid
    if(QisEmpty(&ready_q)) {
        // Process 0 to ready queue?
        run_pid = 0;
    } else {
        // Not sure if this 
        // pcb[0].state = READY;
        pcb[0].state = READY;
        run_pid = DeQ(&ready_q);
        // cons_printf("Scheduler: %d\n", run_pid);
    }

    // reset run_count of selected proc
    // upgrade its state to run
    pcb[run_pid].run_count = 0;
    pcb[run_pid].state = RUN;
}

int main(void) {
    InitKernelData();
    InitKernelControl();

    NewProcSR(InitProc);
    Scheduler();
    Loader(pcb[run_pid].trapframe_p);

    return 0;
}

void Kernel(trapframe_t * trapframe_p) {
    char ch;

    // save it
    pcb[run_pid].trapframe_p = trapframe_p; 


    // handle timer intr
    switch (trapframe_p->entry_id) {
        case TIMER_INTR:
            TimerSR();
            break;
        case GETPID_CALL:
            trapframe_p->eax = run_pid;
            break;
        case SHOWCHAR_CALL:
            ShowCharSR(trapframe_p->eax, trapframe_p->ebx, trapframe_p->ecx);
            break;
        case SLEEP_CALL:
            SleepSR(trapframe_p->eax);
            break;
        case MUX_CREATE_CALL:
            trapframe_p->eax = MuxCreateSR(trapframe_p->eax);
            break;
        case MUX_OP_CALL:
            MuxOpSR(trapframe_p->eax, trapframe_p->ebx);
            break;
        case TERM0_INTR:
            TermSR(0);
            outportb(PIC_CONTROL, TERM0_DONE_VAL);
            break;
        case TERM1_INTR:
            TermSR(1);
            outportb(PIC_CONTROL, TERM1_DONE_VAL);
            break;
        case FORK_CALL:
            trapframe_p->eax = ForkSR();
            break;
        case WAIT_CALL:
            trapframe_p->eax = WaitSR();
            break;
        case EXIT_CALL:
            ExitSR(trapframe_p->eax);
            break;
        default:
            cons_printf("Panic!: Should never be here\n");
    }

    // check if keyboard pressed
    if(cons_kbhit()) {
        ch = cons_getchar();
        // 'b' for breakpoint
        if(ch == 'b') {
            //let's g o GDB
            breakpoint();
        } 
        // 'n' for new process
        else if(ch == 'n') {
            // create a UserProc
            NewProcSR(UserProc);
        }
    }

    // May need t pick another proc
    Scheduler();
    Loader(pcb[run_pid].trapframe_p);
}
