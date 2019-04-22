#include "k-include.h"
#include "k-type.h"
#include "k-data.h"
#include "tools.h"
#include "k-sr.h"
#include "sys-call.h"
#include "proc.h"

void NewProcSR(func_p_t p) {
    int pid;

    if(QisEmpty(&pid_q)) {
        cons_printf("Panic: no more processes!\n");
        breakpoint();
        return;
    }

    pid = DeQ(&pid_q);
    Bzero((char *)&pcb[pid], sizeof(pcb_t));
    Bzero((char *)&proc_stack[pid], PROC_STACK_SIZE);
    pcb[pid].state = READY;

    if(pid > 0) {
        EnQ(pid, &ready_q);
    }

    pcb[pid].trapframe_p = (trapframe_t *)&proc_stack[pid][PROC_STACK_SIZE];
    pcb[pid].trapframe_p--; 
    pcb[pid].trapframe_p->efl = EF_DEFAULT_VALUE|EF_INTR;
    pcb[pid].trapframe_p->cs = get_cs();
    pcb[pid].trapframe_p->eip = (unsigned)p;
}

void TimerSR(void) {
    int pid, i, length;
    outportb(PIC_CONTROL, TIMER_DONE);

    pcb[run_pid].run_count++;
    pcb[run_pid].total_count++;

    sys_centi_sec++;

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

    if(pcb[run_pid].run_count >= TIME_SLICE && run_pid != 0) {
        pcb[run_pid].state = READY;
        EnQ(run_pid, &ready_q);
        // running proc = NONE
        run_pid = NONE;
    }
}

int GetPidSR(void) {
    return run_pid;
}

void ShowCharSR(int row, int col, int ch) {
    unsigned short * p;

    p = VID_HOME;
    p += 80 * row + col;
    *p = (char)ch + VID_MASK;
}

void SleepSR(int sec) {
    pcb[run_pid].wake_centi_sec = sys_centi_sec + sec;
    pcb[run_pid].state = SLEEP;
    EnQ(run_pid, &sleep_q);
    run_pid = NONE;
}

int MuxCreateSR(int flag) {
    int id = DeQ(&mux_q);

    Bzero((char *)&mux[id], sizeof(mux_t));

    mux[id].flag = flag;
    mux[id].creator = run_pid;

    return id;
}

void MuxOpSR(int id, int opcode) {
    switch (opcode) {
        case LOCK:
            if(mux[id].flag > 0) {
                mux[id].flag--;
            } else {
                EnQ(run_pid, &mux[id].suspend_q);
                pcb[run_pid].state = SUSPEND;
                run_pid = NONE;
            }
            break;
        case UNLOCK:
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
    int term_readiness;
    term_readiness = inportb(term[term_no].io_base + IIR);

    if(term_readiness == TXRDY) {
        TermTxSR(term_no);
    } else if(term_readiness == RXRDY) {
        TermRxSR(term_no);
    }

    if(term[term_no].tx_missed == TRUE)
        TermTxSR(term_no);
}


void TermTxSR(int term_no) {
    if(QisEmpty(&term[term_no].echo_q) && QisEmpty(&term[term_no].out_q)) {
        term[term_no].tx_missed = TRUE;
        return;
    } else {
        int ch;

        if(!QisEmpty(&term[term_no].echo_q)) {
            ch = DeQ(&term[term_no].echo_q);
        } else {
            ch = DeQ(&term[term_no].out_q);
            MuxOpSR(term[term_no].out_mux, UNLOCK);
        }

        outportb(term[term_no].io_base + DATA, (char)ch);
        term[term_no].tx_missed = FALSE;
    }
}

void TermRxSR(int term_no) {
    int suspended_pid, device;
    int ch = inportb(term[term_no].io_base + DATA);

    if(term_no == 0) {
        device = TERM0_INTR;
    } else {
        device = TERM1_INTR;
    }

    EnQ(ch, &term[term_no].echo_q);

    if(ch == '\r') {
        EnQ('\n', &term[term_no].echo_q);
        EnQ('\0', &term[term_no].in_q);
    } else {
        EnQ(ch, &term[term_no].in_q);
    }

    suspended_pid = QPeek(&mux[term[term_no].in_mux].suspend_q);

    if(ch == SIGINT && !QisEmpty(&mux[term[term_no].in_mux].suspend_q) && pcb[suspended_pid].sigint_handler != 0) {
        breakpoint();
        WrapperSR(suspended_pid, pcb[suspended_pid].sigint_handler, device);
    }

    MuxOpSR(term[term_no].in_mux, UNLOCK);
}

int ForkSR(void) {
    int pid;
    int difference;
    int * p;

    difference = 0;

    if(QisEmpty(&pid_q)) {
        cons_printf("Panic: no more processes!\n");
        return NONE;
    }
    
    pid = DeQ(&pid_q);

    Bzero((char *)&pcb[pid], sizeof(pcb_t));
    Bzero((char *)&proc_stack[pid], PROC_STACK_SIZE);

    pcb[pid].state = READY;
    pcb[pid].ppid = run_pid;

    EnQ(pid, &ready_q);

    difference = PROC_STACK_SIZE*(pid-run_pid);
    pcb[pid].trapframe_p = (trapframe_t *)((int)pcb[run_pid].trapframe_p + difference);

    MemCpy((char *)&proc_stack[pid], (char *)&proc_stack[run_pid], PROC_STACK_SIZE);

    pcb[pid].trapframe_p->eax = 0;
    pcb[pid].trapframe_p->esp = pcb[run_pid].trapframe_p->esp + difference;
    pcb[pid].trapframe_p->ebp = pcb[run_pid].trapframe_p->ebp + difference;
    pcb[pid].trapframe_p->esi = pcb[run_pid].trapframe_p->esi + difference;
    pcb[pid].trapframe_p->edi = pcb[run_pid].trapframe_p->edi + difference;

    p = (int *)pcb[pid].trapframe_p->ebp;

    while(*p != 0) {
       *p = *p+difference;
        p = (int *)*p;
    }

    return pid;
}

int WaitSR(void) {
    int i;
    int exit_code;

    for(i = 0; i < PROC_SIZE; i++) {
        if(pcb[i].ppid == run_pid && pcb[i].state == ZOMBIE) {
            break;
        }
    }

    if(i == PROC_SIZE) {
        pcb[run_pid].state = WAIT;
        run_pid = NONE;
        return NONE;
    }

    exit_code = pcb[i].trapframe_p->eax;

    pcb[i].state = UNUSED;
    EnQ(i, &pid_q);

    return exit_code;
}

void ExitSR(int exit_code) {

    if(pcb[pcb[run_pid].ppid].state != WAIT) {
        pcb[run_pid].state = ZOMBIE;
        run_pid = NONE;
        return;
    }

    pcb[pcb[run_pid].ppid].state = READY;
    EnQ(pcb[run_pid].ppid, &ready_q);
    pcb[pcb[run_pid].ppid].trapframe_p->eax = exit_code;

    pcb[run_pid].state = UNUSED;
    EnQ(run_pid, &pid_q);
    run_pid = NONE;
}

void ExecSR(int code, int arg) {
    int code_page_index = NONE;
    int stack_page_index = NONE;
    char * code_page = 0;
    char * stack_page = 0;
    char * stack_page_top = 0;

    code_page_index = AllocatePage(0);
    stack_page_index = AllocatePage(code_page_index + 1);

    if(code_page_index == NONE || stack_page_index == NONE){ 
        cons_printf("Panic: Ran out of memory pages\n");
        return;
    }

    page_user[code_page_index] = run_pid;
    page_user[stack_page_index] = run_pid;

    code_page = (char *)(code_page_index * PAGE_SIZE + RAM);
    stack_page = (char *)(stack_page_index * PAGE_SIZE + RAM);

    MemCpy(code_page, (char *)code, PAGE_SIZE);
    Bzero(stack_page, PAGE_SIZE);

    stack_page_top = (char *)(stack_page + PAGE_SIZE);

    stack_page_top -= sizeof(int);
    *(int *)stack_page_top = arg;

    stack_page_top -= sizeof(int);
    stack_page_top -= sizeof(trapframe_t);

    pcb[run_pid].trapframe_p = (trapframe_t *)stack_page_top;

    pcb[run_pid].trapframe_p->efl = EF_DEFAULT_VALUE|EF_INTR;
    pcb[run_pid].trapframe_p->cs = get_cs();
    pcb[run_pid].trapframe_p->eip = (int)code_page;
}

void SignalSR(int sig_num, int handler) {
    pcb[run_pid].sigint_handler = handler;
}

void WrapperSR(int pid, int handler, int arg) {
    trapframe_t * temp = pcb[pid].trapframe_p;
    (char *)temp -= 3 * sizeof(int);

    MemCpy((char *)temp, (char *)pcb[pid].trapframe_p, sizeof(trapframe_t));
    pcb[pid].trapframe_p = temp;
    temp++;

    *(int *)temp = pcb[pid].trapframe_p->eip;
    (int *)temp += 1;

    *(int *)temp = handler;
    (int *)temp += 1;

    *(int *)temp = arg;
    (int *)temp += 1;

    pcb[pid].trapframe_p->eip = (int)Wrapper;
}
