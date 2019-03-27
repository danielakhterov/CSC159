#include "k-include.h"
#include "k-type.h"
#include "k-data.h"
#include "k-lib.h"
#include "k-sr.h"
#include "sys-call.h"

// To create a process: alloc PID, PCB, and process stack
// bulid trapframe, initialize PCB, record PID to ready_q
void NewProcSR(func_p_t p) {
    // arg: Where process code starts
    int pid;

    if(QisEmpty(&pid_q)) {
        cons_printf("Panic: no more processes!\n");
        // Cannot continue, alternative: breakpoint();
        breakpoint();
        return;
    }

    // alloc PID (1st is 0)
    pid = DeQ(&pid_q);
    // clear PCB
    Bzero((char *)&pcb[pid], sizeof(pcb_t));
    // clear stack
    Bzero((char *)&proc_stack[pid], PROC_STACK_SIZE);
    // change process state
    pcb[pid].state = READY;

    // Queue to ready_q if > 0
    if(pid > 0) {
        EnQ(pid, &ready_q);
    }

    // point trapframe_p to stack & fill it out
    // point to stack top
    pcb[pid].trapframe_p = (trapframe_t *)&proc_stack[pid][PROC_STACK_SIZE];
    // lower by trapframe size
    pcb[pid].trapframe_p--; 
    // enables intr
    pcb[pid].trapframe_p->efl = EF_DEFAULT_VALUE|EF_INTR;
    // dupl from CPU
    pcb[pid].trapframe_p->cs = get_cs();
    // set to code
    pcb[pid].trapframe_p->eip = (unsigned)p;
}

// Count run_count and switch if hitting time slice
void TimerSR(void) {
    int pid, i, length;
    // notify PIC timer done
    outportb(PIC_CONTROL, TIMER_DONE);

    // count up to run_count
    pcb[run_pid].run_count++;
    // count up to total_count
    pcb[run_pid].total_count++;

    sys_centi_sec++;

    // Check wake times of processes

    // if runs long enough
    length = sleep_q.tail;

    for(i = 0; i < length; i++) {
        pid = DeQ(&sleep_q);
        if(pcb[pid].wake_centi_sec <= sys_centi_sec) {
            pcb[pid].state = READY;
            EnQ(pid, &ready_q);
        } else {
            EnQ(pid, &sleep_q);
        }
    }

    // move it to ready_q
    // change its state
    if(pcb[run_pid].run_count >= TIME_SLICE && run_pid != 0) {
        pcb[run_pid].state = READY;
        EnQ(run_pid, &ready_q);
        // running proc = NONE
        run_pid = NONE;
    }
}

// Gets the pid of the current running process
int GetPidSR(void) {
    return run_pid;
}

// Show ch at row, col
void ShowCharSR(int row, int col, int ch) {
    unsigned short * p;

    p = VID_HOME;
    p += 80 * row + col;
    *p = (char)ch + VID_MASK;
}

// Delays for i amount of centi seconds
void SleepSR(int sec) {
    pcb[run_pid].wake_centi_sec = sys_centi_sec + sec;
    pcb[run_pid].state = SLEEP;
    EnQ(run_pid, &sleep_q);
    run_pid = NONE;
}

// details described in 3.html
int MuxCreateSR(int flag) {
    // Get next id for mux
    int id = DeQ(&mux_q);

    // Zero out that mux
    // This  would also zero out the suspend_q
    Bzero((char *)&mux[id], sizeof(mux_t));

    mux[id].flag = flag;
    mux[id].creator = run_pid;

    return id;
}

// details described in 3.html
void MuxOpSR(int id, int opcode) {
    switch (opcode) {
        case LOCK:
            // decrement the flag in the mutex by 1 if it is greater than 0, otherwise
            // Enqueue the running process because
            // the  current mutex is locked already
            if(mux[id].flag > 0) {
                mux[id].flag--;
            } else {
                EnQ(run_pid, &mux[id].suspend_q);
                pcb[run_pid].state = SUSPEND;
                run_pid = NONE;
            }
            break;
        case UNLOCK:
            // if no suspended process in the suspension queue of the mutex, just increment the flag of the mutex by 1
            // otherwise, release the 1st PID in the suspension queue
            if(QisEmpty(&mux[id].suspend_q)) {
                mux[id].flag++;
            } else {
                int pid = DeQ(&mux[id].suspend_q);
                EnQ(pid, &ready_q);
                pcb[pid].state = READY;
            }
            break;
        default:
            cons_printf("Invalid opcode received by MuxOpSR");
    }
}

void TermSR(int term_no) {
    // read the type of event from IIR of the terminal port
    int term_readiness;
    term_readiness = inportb(term[term_no].io_base + IIR);

    // if it's TXRDY, call TermTxSR(term_no)
    // else if it's RXRDY, call TermRxSR(term_no) which does nothing but 'return;'

    if(term_readiness == TXRDY) {
        TermTxSR(term_no);
    } else if(term_readiness == RXRDY) {
        TermRxSR(term_no);
    }

    // if the tx_missed flag is TRUE, also call TermTxSR(term_no)
    if(term[term_no].tx_missed == TRUE)
        TermTxSR(term_no);
}


void TermTxSR(int term_no) {
    // if the out_q in terminal interface data structure is empty:
    //  1. set the tx_missed flag to TRUE
    //  2. return
    // (otherwise)
    //  1. get 1st char from out_q
    //  2. use outportb() to send it to the DATA register of the terminal port
    //  3. set the tx_missed flag to FALSE
    //  4. unlock the out_mux of the terminal interface data structure

    if(QisEmpty(&term[term_no].echo_q) && QisEmpty(&term[term_no].out_q)) {
        term[term_no].tx_missed = TRUE;
        return;
    } else {
        int ch;
        
        if(!QisEmpty(&term[term_no].echo_q)) {
            ch = DeQ(&term[term_no].echo_q);
        } else {
            ch = DeQ(&term[term_no].out_q);
        }

        outportb(term[term_no].io_base + DATA, (char)ch);
        term[term_no].tx_missed = FALSE;
        MuxOpSR(term[term_no].out_mux, UNLOCK);
    }
}

void TermRxSR(int term_no) {
    // read a char from the terminal io_base+DATA
    int ch = inportb(term[term_no].io_base + DATA);

    // enqueue char to the terminal echo_q
    EnQ(ch, &term[term_no].echo_q);

    // if char is CR -> also enqueue NL to the terminal echo_q
    // if char is CR -> enqueue NUL to the terminal in_q
    // else -> enqueue char to the terminal in_q
    if(ch == '\r') {
        EnQ('\n', &term[term_no].echo_q);
        EnQ('\0', &term[term_no].in_q);
    } else {
        EnQ(ch, &term[term_no].in_q);
    }

    // unlock the terminal in_mux
    MuxOpSR(term[term_no].in_mux, UNLOCK);
}
