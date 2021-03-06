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
    int pid;
    // notify PIC timer done
    outportb(PIC_CONTROL, TIMER_DONE);

    // count up to run_count
    pcb[run_pid].run_count++;
    // count up to total_count
    pcb[run_pid].total_count++;

    sys_centi_sec++;

    // Check wake times of processes

    // if runs long enough

    while(pcb[sleep_q.q[0]].wake_centi_sec <= sys_centi_sec && sleep_q.tail > 0) {
        pid = PriorityDequeue(&sleep_q);
        pcb[pid].state = READY;
        EnQ(pid, &ready_q);
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
    PriorityEnqueue(&sleep_q, run_pid);
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
