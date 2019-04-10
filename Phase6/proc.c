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

// run demo to see what it does
void Aout(int device) {
    int i;
    // get my PID
    int pid = GetPidCall();

    // char str[] = "xx ( ) Hello, World!\n\r";
    char str[STR_SIZE] = "   ( ) Hello, World!\n\r\0";

    str[0] = '0' + pid / 10;
    str[1] = '0' + pid % 10;

    // in above str, replace xx with my PID, and a alphabet inside
    // the bracket (alphabet B if my PID is 2, C if 3, D if 4, etc.)
    str[4] = 'A' + pid - 1;

    // prompt to terminal the str    // use same device as parent
    WriteCall(device, str);

    // slide my alphabet across the Target PC display:
    // cycle thru columns 0 to 69 {
    //  use ShowCharCall( ...
    //  use SleepCall(10);
    //  use ShowCharCall( ...
    // }

    for(i = 0; i < 70; i++) {
        ShowCharCall(pid, i, str[4]);
        SleepCall(10);
        ShowCharCall(pid, i, ' ');
    }

    // call ExitCall with an exit code that is my_pid * 100
    ExitCall(pid * 100);
}

void UserProc(void) {
    int device;
    int ret;
    int pid = GetPidCall();

    char pid_str[STR_SIZE] = "PID    > ";
    char read_buffer[STR_SIZE];
    char buffer[6] = "\0\0\0\0\0\0";

    pid_str[4] = '0' + pid / 10;
    pid_str[5] = '0' + pid % 10;

    device = pid % 2 == 1 ? TERM0_INTR : TERM1_INTR;

    while(1) {
        // prompt for terminal input
        WriteCall(device, pid_str);
        // read terminal input
        ReadCall(device, read_buffer);

        // compare read_buffer and "fork", if not the same -> "continue;"
        if(StrCmp(read_buffer, "fork\0") == FALSE) {
            continue;
        }

        // make a ForkCall() and get its return
        ret = ForkCall();

        // if it returns NONE {
        //  prompt to terminal: "Couldn't fork!"
        //  continue;
        // }

        if(ret == NONE) {
            WriteCall(device, "Couldn't fork!\n\r\0");
            continue;
        }

        // if the return is == 0 --> call Aout(device);  // child code

        // prompt to terminal: the child PID (see demo)  // parent code
        // use Itoa() to convert # to a str and prompt it
        // prompt to terminal "\n\r" can be needed

        // make a WaitCall() and get an exit code from child

        // prompt to terminal: the child exit code (see demo)
        // use Itoa() to convert # to a str and prompt it
        // prompt to terminal "\n\r" can be needed
        if(ret == 0) {
            Aout(device);
        } else {
            WriteCall(device, "Child PID: ");

            Itoa(buffer, ret);
            WriteCall(device, buffer);
            WriteCall(device, "\n\r");

            ret = WaitCall();
            Itoa(buffer, ret);
            WriteCall(device, "Child exit code: ");
            WriteCall(device, buffer);
            WriteCall(device, "\n\r");
        }
    }
}
