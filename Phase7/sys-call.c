// sys-call.c
// calls to kernel services

#include "k-const.h"
#include "k-data.h"
#include "tools.h"

#include <spede/stddef.h>

int GetPidCall(void) {
    // interrupt!
    // after, copy eax to variable 'pid'
    // output
    // input
    // used registers
    int pid;
    asm("int %1;                
         movl %%eax, %0"         
        : "=g" (pid)           
        : "g" (GETPID_CALL)   
        : "eax"                      
    );

    return pid;
}

void ShowCharCall(int row, int col, int ch) {
    // send in row via eax
    // send in col via ebx
    // send in ch via ecx
    // initiate call, %3 gets entry_id
    asm("movl %0, %%eax;     
         movl %1, %%ebx;     
         movl %2, %%ecx;      
         int %3"            
        :                        
        : "g" (row),       
          "g" (col), 
          "g" (ch),
          "g" (SHOWCHAR_CALL)
        : "eax", "ebx", "ecx"
    );
}

// # of 1/100 of a second to sleep
void SleepCall(int centi_sec) {  
    asm("movl %0, %%eax;
         int %1"
        :
        : "g" (centi_sec),
          "g" (SLEEP_CALL)
        : "eax"
    );
}

int MuxCreateCall(int flag) {
    int id;
    asm("movl %1, %%eax;
         int %2;
         movl %%eax, %0"
        : "=g" (id)
        : "g" (flag),
          "g" (MUX_CREATE_CALL)
        : "eax"
    );

    return id;
}

void MuxOpCall(int mux_id, int opcode) {
    asm("movl %0, %%eax;
         movl %1, %%ebx;
         int %2"
        :
        : "g" (mux_id),
          "g" (opcode),
          "g" (MUX_OP_CALL)
        : "eax", "ebx"
    );
}

// composed differently
void WriteCall(int device, char * str) { 
    // get my PID by a service call
    // to set row number
    // column is set zero
    // if device is STDOUT {
    //    while what str points to is not a null character {
    //       Use an existing service call to show this character, at row and column
    //       increment the str pointer and the column position
    //    }
    // }
    int pid, row, col, term_no;
    pid = GetPidCall();
    row = pid;
    col = 0;

    if(device == STDOUT) {
        while(*str != '\0') {
            ShowCharCall(row, col++, *str);
            str++;
        }
    } else {                                 
        // device is a terminal

        // set 'int term_no' to 0 or 1 depending on the given argument
        // 'device' whether it is TERM0_INTR or TERM1_INTR
        if(device == TERM0_INTR)
            term_no = 0;
        else
            term_no = 1;

        // while what str points to is not a null character {
        //    lock the output mutex of the terminal interface data structure
        //    enqueue the character of the string to the output queue of the
        //    terminal interface data structure

        //    if the device is TERM0_INTR, issue asm("int $35");
        //    otherwise, issue: asm("int $36");

        //    advance pointer 'str'
        // }

        while(*str != '\0') {
           // if(QisFull(&term[term_no].out_q))
               // continue;
            MuxOpCall(term[term_no].out_mux, LOCK);
            EnQ((int)*str, &term[term_no].out_q);
            
            if(device == TERM0_INTR)
                asm("int $35");
            else if(device == TERM1_INTR)
                asm("int $36");

            str++;
        }
    }
}

void ReadCall(int device, char * str) {
    // determine which term_no (similar to how WriteCall did in the previous phase)
    // number of chars gathered so far = 0
    // loop forever {
    //  A. lock the in_mux of the terminal
    //  B. dequeue a char from out_q of the terminal
    //  C. set where the str points to to char
    //  D. if char is NUL -> return
    //  E. advance both str pointer and char count
    //  F. if char count is at the last available space of the given string:
    //     a. set where str points to to NUL
    //     b. return
    // }

    int term_no;
    int chars = 0;

    if(device == TERM0_INTR)
        term_no = 0;
    else
        term_no = 1;

    while(1) {
        int ch;

        MuxOpCall(term[term_no].in_mux, LOCK);
        ch = DeQ(&term[term_no].in_q);
        *str = (char)ch;

        if(ch == '\0')
            return;

        chars++;
        str++;

        if(chars == STR_SIZE - 1) {
            *str = '\0';
            return;
        }
    }
}

int ForkCall(void) {
    int ret;

    asm("int %1;
         mov %%eax, %0"
         : "=g" (ret)
         : "g" (FORK_CALL)
         : "eax"
    );

    return ret;
}

int WaitCall(void) {
    int ret;

    asm("int %1;
         mov %%eax, %0"
         : "=g" (ret)
         : "g" (WAIT_CALL)
         : "eax"
    );

    return ret;
}

void ExitCall(int exit_code) {
    asm("mov %0, %%eax;
         int %1"
         :
         : "g" (exit_code),
           "g" (EXIT_CALL)
         : "eax"
    );
}

void ExecCall(int code, int arg) {
    asm("mov %0, %%eax;
         mov %1, %%ebx;
         int %2"
         :
         : "g" (code),
           "g" (arg),
           "g" (EXEC_CALL)
         : "eax", "ebx"
    );
}

void SignalCall(int sig_num, int handler) {
    asm("mov %0, %%eax;
         mov %1, %%ebx;
         int %2"
         :
         : "g" (sig_num),
           "g" (handler),
           "g" (SIGNAL_CALL)
         : "eax", "ebx"
    );
}
