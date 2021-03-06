// sys-call.c
// calls to kernel services

#include "k-const.h"
#include "k-data.h"
#include "k-lib.h"

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
