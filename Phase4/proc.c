#include "k-const.h"
#include "k-data.h"
#include "sys-call.h"
#include "k-lib.h"
#include "k-include.h"

void InitTerm(int term_no) {
   int i, j;

   Bzero((char *)&term[term_no].out_q, sizeof(q_t));
   term[term_no].out_mux = MuxCreateCall(Q_SIZE);

    // CFCR_DLAB is 0x80
    outportb(term[term_no].io_base+CFCR, CFCR_DLAB);

    // period of each of 9600 bauds
    outportb(term[term_no].io_base+BAUDLO, LOBYTE(115200/9600));
    outportb(term[term_no].io_base+BAUDHI, HIBYTE(115200/9600));
    outportb(term[term_no].io_base+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);

    outportb(term[term_no].io_base+IER, 0);
    outportb(term[term_no].io_base+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);

    for(i = 0; i < LOOP/2; i++)
        asm("inb $0x80");

    // enable TX & RX intr
    outportb(term[term_no].io_base+IER, IER_ERXRDY|IER_ETXRDY);
    for(i = 0; i < LOOP/2; i++)
        asm("inb $0x80");

    // clear screen, sort of
    for(j = 0; j < 25; j++) {
        outportb(term[term_no].io_base+DATA, '\n');
        for(i = 0; i < LOOP/30; i++)
            asm("inb $0x80");

        outportb(term[term_no].io_base+DATA, '\r');
        for(i = 0; i < LOOP/30; i++)
            asm("inb $0x80");
    }

    /* clear key cleared PROCOMM screen
    inportb(term_term_no].io_base);
    for(i=0; i<LOOP/2; i++)asm("inb $0x80"); */
}

// Phase3: Request/alloc vid_mux using the new service MuxCreateCall().
void InitProc(void) {
    int i;

   InitTerm(0);
   InitTerm(1);

    // Creating vid_mux
    vid_mux = MuxCreateCall(1);

    while(1) {
        // show a dot at upper-left corener of PC
        ShowCharCall(0, 0, '.');
        // wait for about half a second
        for(i=0; i<LOOP/2; i++) 
            asm("inb $0x80");

        // erase dot
        ShowCharCall(0, 0, ' ');
        // wait for about half a second
        for(i=0; i<LOOP/2; i++) 
            asm("inb $0x80");
    }
}

// Phase3: Experiment with MuxOpCall() calls to lock and unlock vid_mux
// in the while(1) loop.
void UserProc(void) {
    // Get my pid from sys call
    // int i, length, 
    int which_term;
    int pid = GetPidCall();
    // char zeros[11] = "0000000000";
    // char * pid_str = &zeros[0];
    // char spaces[11] = "          ";

   char pid_str[STR_SIZE] = "PID    process is running exclusively using the video display...";
   char spaces[STR_SIZE] = "                                                                ";

   pid_str[4] = '0' + pid / 10;  // show my PID
   pid_str[5] = '0' + pid % 10;

    // length = 0;

    which_term = pid % 2 == 1 ? TERM0_INTR : TERM1_INTR;

    // number would be backwards in the array
    // while(pid / 10 > 0) {
    //     zeros[10 - length] = (char)((pid % 10) + '0');
    //     pid /= 10;
    //     length++;
    // } 

    // Get the last digit in case the number < 10
    // zeros[10 - length] = (char)((pid % 10) + '0');
    // length++;

    // Trim the beginning of the number by simply incrementing the pointer
    // for(i = 0; i < 10 - length; i++) {
    //     pid_str++;
    // } 

    while(1) {
        // MuxOpCall(vid_mux, LOCK);

        WriteCall(STDOUT, pid_str);
        WriteCall(which_term, pid_str);
        WriteCall(which_term, "\n\r");
        SleepCall(50);

        WriteCall(STDOUT, spaces);
        SleepCall(50);

        // MuxOpCall(vid_mux, UNLOCK);
    }
}
