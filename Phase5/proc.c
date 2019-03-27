#include "k-const.h"
#include "k-data.h"
#include "sys-call.h"
#include "k-lib.h"
#include "k-include.h"

// Phase 5
void InitTerm(int term_no) {
    int i, j;

    Bzero((char *)&term[term_no].out_q, sizeof(q_t));
    Bzero((char *)&term[term_no].in_q, sizeof(q_t));
    Bzero((char *)&term[term_no].echo_q, sizeof(q_t));
    term[term_no].out_mux = MuxCreateCall(Q_SIZE);
    term[term_no].in_mux = MuxCreateCall(0);

    // CFCR_DLAB is 0x80
    outportb(term[term_no].io_base+CFCR, CFCR_DLAB);
    // period of each of 9600 bauds
    outportb(term[term_no].io_base+BAUDLO, LOBYTE(115200/9600));
    outportb(term[term_no].io_base+BAUDHI, HIBYTE(115200/9600));
    outportb(term[term_no].io_base+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);

    outportb(term[term_no].io_base+IER, 0);
    outportb(term[term_no].io_base+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
    for(i=0; i<LOOP/2; i++)asm("inb $0x80");

    // enable TX & RX intr
    outportb(term[term_no].io_base+IER, IER_ERXRDY|IER_ETXRDY);
    for(i=0; i<LOOP/2; i++)asm("inb $0x80");

    // Clear the screen
    for(j=0; j<25; j++) {
      outportb(term[term_no].io_base+DATA, '\n');
      for(i=0; i<LOOP/30; i++)asm("inb $0x80");
      outportb(term[term_no].io_base+DATA, '\r');
      for(i=0; i<LOOP/30; i++)asm("inb $0x80");
    }

    /*  
    // uncomment this part for VM (Procomm logo needs a key pressed, here reads it off)
    inportb(term[term_no].io_base); // clear key cleared PROCOMM screen
    for(i=0; i<LOOP/2; i++)asm("inb $0x80");
    */
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
    int device;
    int pid = GetPidCall();

    // char pid_str[STR_SIZE] = "PID    process is running exclusively using the video display...";
    // char spaces[STR_SIZE] = "                                                                ";
    char pid_str[STR_SIZE] = "PID    > ";
    char spaces[STR_SIZE];

    // show my PID
    pid_str[4] = '0' + pid / 10;
    pid_str[5] = '0' + pid % 10;

    device = pid % 2 == 1 ? TERM0_INTR : TERM1_INTR;

    while(1) {
        // prompt for terminal input
        WriteCall(device, pid_str);

        // read terminal input
        ReadCall(device, spaces);

        // show what input was to PC
        WriteCall(STDOUT, spaces);
    }
}
