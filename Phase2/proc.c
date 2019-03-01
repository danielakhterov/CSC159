#include "k-const.h"
#include "k-data.h"
#include "sys-call.h"

void InitProc(void) {
    int i;
    while(1) {
        // show a dot at upper-left corener of PC
        ShowCharCall(0, 0, '.');
        // wait for about half a second
        for(i=0; i<LOOP/2; i++) asm("inb $0x80");

        // erase dot
        ShowCharCall(0, 0, ' ');
        // wait for about half a second
        for(i=0; i<LOOP/2; i++) asm("inb $0x80");
    }
}

void UserProc(void) {
    /* while(1) {
      Delay();
      ShowChar(run_pid, 0, run_pid/10 + '0');
      ShowChar(run_pid, 1, run_pid%10 + '0');

      Delay();
      ShowChar(run_pid, 0, ' ');
      ShowChar(run_pid, 1, ' ');
    } */

    // Get my pid from sys call
    int pid = GetPidCall();
    while(1) {
        // show 1st digit of my PID at row run_pid+1, 1st col
        // show 2nd digit of my PID at row run_pid+2, 2nd col
        int _pid;
        int length;
        char _chars[11] = "0000000000";
        int i;

        _pid = pid;
        length = 0;

        while(_pid / 10 > 0) {
            _chars[length] = (char)((_pid % 10) + '0');
            _pid /= 10;
            length++;
        } 

        _chars[length] = (char)((_pid % 10) + '0');
        length++;

        for(i = 0; i < length; i++) {
            ShowCharCall(pid + 1, length - i - 1, _chars[i]);
        }

        // wait for about half a second
        SleepCall(50);

        // erase above writing (one digit at a time)
        for(i = 0; i < length; i++) {
            ShowCharCall(pid + 1, length - i - 1, ' ');
        }

        // wait for about half a second
        SleepCall(50);
    }
}
