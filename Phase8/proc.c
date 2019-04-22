#include "k-const.h"
#include "k-data.h"
#include "sys-call.h"
#include "tools.h"
#include "k-include.h"

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

void InitProc(void) {
    int i;

   InitTerm(0);
   InitTerm(1);

    vid_mux = MuxCreateCall(1);

    while(1) {
        ShowCharCall(0, 0, '.');
        for(i=0; i<LOOP/2; i++) 
            asm("inb $0x80");

        ShowCharCall(0, 0, ' ');
        for(i=0; i<LOOP/2; i++) 
            asm("inb $0x80");
    }
}

// args implanted in stack
void Wrapper(int handler, int arg) {
    func_p_t2 func = (func_p_t2)handler;

    // save regs
    asm("pushal");

    // call signal handler with arg
    func(arg);

    // restore regs
    asm("popal");

    // skip handler & arg
    asm("movl %%ebp, %%esp; popl %%ebp; ret $8"::);
}

void Ouch(int device) {
    WriteCall(device, "Can't touch that!\n\r");
}

void Aout(int device) {
    int i;
    unsigned sleep_time;
    int pid = GetPidCall();
    char str[] = "   ( ) Hello, World!\n\r\0";

    str[0] = '0' + pid / 10;
    str[1] = '0' + pid % 10;
    str[4] = 'A' + pid - 1;

    WriteCall(device, str);

    PauseCall();

    for(i = 0; i < 70; i++) {
        ShowCharCall(pid, i, str[4]);

        sleep_time = RandCall() % 20 + 5;
        SleepCall((int)sleep_time);

        ShowCharCall(pid, i, ' ');
    }

    ExitCall(pid * 100);
}

void UserProc(void) {
    int i;
    int device;
    int ret = NONE;
    int pid = GetPidCall();

    char pid_str[STR_SIZE] = "PID    > ";
    char read_buffer[STR_SIZE];
    char buffer[6] = "\0\0\0\0\0\0";
    char letter[2] = "#\0";

    pid_str[4] = '0' + pid / 10;
    pid_str[5] = '0' + pid % 10;

    device = pid % 2 == 1 ? TERM0_INTR : TERM1_INTR;

    SignalCall(SIGINT, (int)Ouch);

    while(1) {
        WriteCall(device, pid_str);
        ReadCall(device, read_buffer);

        if(StrCmp(read_buffer, "race\0") == FALSE) {
            continue;
        }

        for(i = 0; i < 5 && ret != 0; i++) {
            ret = ForkCall();
        }

        if(ret == NONE) {
            WriteCall(device, "Couldn't fork!\n\r\0");
            continue;
        }

        if(ret == 0) {
            ExecCall((int)Aout, device);
        } else {
            SleepCall(300);

            KillCall(pid, SIGGO);

            // WriteCall(device, "Child PID: ");

            // Itoa(buffer, ret);
            // WriteCall(device, buffer);
            // WriteCall(device, "\n\r");

            for(i = 0; i < 5; i++) {
                ret = WaitCall();
                Itoa(buffer, ret);

                WriteCall(device, "Child exit code: ");
                WriteCall(device, buffer);
                WriteCall(device, " ... ");

                letter[0] = 'A' + (ret / 100) - 1;
                WriteCall(device, letter);
                WriteCall(device, " Arrives!");
                WriteCall(device, "\n\r");
            }
        }
    }
}
