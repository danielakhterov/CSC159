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
    pcb[pid].main_table = kernel_main_table;
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
    pcb[pid].main_table = pcb[run_pid].main_table;

    p = (int *)pcb[pid].trapframe_p->ebp;

    while(*p != 0) {
       *p = *p+difference;
        p = (int *)*p;
    }

    return pid;
}

int WaitSR(void) {
    int i,j;
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

    set_cr3(pcb[i].main_table);

    exit_code = pcb[i].trapframe_p->eax;

    set_cr3(pcb[run_pid].main_table);

    for(j = 0;j<PAGE_NUM;j++){
        if(page_user[j] == i){
            page_user[j] = NONE;
        }
    }

    pcb[i].state = UNUSED;
    EnQ(i, &pid_q);

    return exit_code;
}

void ExitSR(int exit_code) {
    int i;

    if(pcb[pcb[run_pid].ppid].state != WAIT) {
        pcb[run_pid].state = ZOMBIE;
        run_pid = NONE;
        return;
    }

    // Free memory pages
    for(i = 0; i < PAGE_NUM; i++) {
        if(page_user[i] == run_pid) {
            page_user[i] = NONE;
        }
    }

    pcb[pcb[run_pid].ppid].state = READY;
    EnQ(pcb[run_pid].ppid, &ready_q);
    pcb[pcb[run_pid].ppid].trapframe_p->eax = exit_code;

    pcb[run_pid].state = UNUSED;
    EnQ(run_pid, &pid_q);

    set_cr3(kernel_main_table);

    run_pid = NONE;
}

void ExecSR(int code, int arg) {
    int i, j, page_indices[5], *p, entry;
    int * pages[5];
    trapframe_t *q;
    enum {MAIN_TABLE, CODE_TABLE, STACK_TABLE, CODE_PAGE, STACK_PAGE};

    for(i = 0; i < 5; i++) {
        if(i == 0) {
            page_indices[i] = AllocatePage(0);
        } else {
            page_indices[i] = AllocatePage(page_indices[i - 1] + 1);
        }
    }

    for(i = 0; i < 5; i++) {
        if(page_indices[i] == NONE) {
            cons_printf("Panic: Ran out of memory page_indices\n");
            return;
        }
    }

    for(i = 0; i < 5; i++) {
        page_user[page_indices[i]] = run_pid;
    }

    for(i = 0; i < 5; i++) {
        pages[i] = (int *)(page_indices[i] * PAGE_SIZE + RAM);
    }

    MemCpy((char *)pages[CODE_PAGE], (char *)code, PAGE_SIZE);
    Bzero((char *)pages[STACK_PAGE], PAGE_SIZE);

    (char *)p = (char *)pages[STACK_PAGE] + PAGE_SIZE;

    p--;
    *p = arg;

    p--;

    q = (trapframe_t *)p;
    q--;

    pcb[run_pid].trapframe_p = q;

    pcb[run_pid].trapframe_p->efl = EF_DEFAULT_VALUE|EF_INTR;
    pcb[run_pid].trapframe_p->cs = get_cs();
    pcb[run_pid].trapframe_p->eip = (int)pages[CODE_PAGE];

    Bzero((char *)pages[MAIN_TABLE], PAGE_SIZE);
    MemCpy((char *)pages[MAIN_TABLE], 
            (char *)kernel_main_table, 
            sizeof(int) * 4);

    // 10 left-most bits
    entry = M256 >> 22;
    ((int *)pages[MAIN_TABLE])[entry] = (int)pages[CODE_TABLE] | PRESENT | RW;

    entry = G1_1 >> 22;
    ((int *)pages[MAIN_TABLE])[entry] = (int)pages[STACK_TABLE] | PRESENT | RW;

    Bzero((char *)pages[CODE_TABLE], PAGE_SIZE);
    entry = M256 & MASK10 >> 12;
    ((int *)pages[CODE_TABLE])[entry] = (int)pages[CODE_PAGE] | PRESENT | RW;

    Bzero((char *)pages[STACK_TABLE], PAGE_SIZE);
    entry = G1_1 & MASK10 >> 12;
    ((int *)pages[STACK_TABLE])[entry] = (int)pages[STACK_PAGE] | PRESENT | RW;

    pcb[run_pid].main_table = (int)pages[MAIN_TABLE];
    pcb[run_pid].trapframe_p = (trapframe_t *)V_TF;
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

void PauseSR(void) {
    // a. alter the state of the running process
    // b. the running process is now NONE
    pcb[run_pid].state = PAUSE;
    run_pid = NONE;
}

void KillSR(int pid, int sig_num) {
    int i;

    for(i = 0; i < PROC_SIZE; i++) {
        if(pcb[i].ppid == pid && pcb[i].state == PAUSE) {
            pcb[i].state = READY;
            EnQ(i, &ready_q);
        }
    }
}

unsigned RandSR(void) {
    rand = run_pid * rand + A_PRIME;
    rand %= G2;
    return rand;
}
