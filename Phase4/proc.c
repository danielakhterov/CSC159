#include "k-const.h"
#include "k-data.h"
#include "sys-call.h"

// Phase3: Request/alloc vid_mux using the new service MuxCreateCall().
void InitProc(void) {
    int i;

    // Creating vid_mux
    vid_mux = MuxCreateCall(1);

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

// Phase3: Experiment with MuxOpCall() calls to lock and unlock vid_mux
// in the while(1) loop.
void UserProc(void) {
    // Get my pid from sys call
    int pid = GetPidCall();
    int i;
    int _pid = pid;

    while(1) {
        int length;
        char chars[11] = "0000000000";

        _pid = pid;
        length = 0;

        MuxOpCall(vid_mux, LOCK);

        // number would be backwards in the array
        while(_pid / 10 > 0) {
            chars[length] = (char)((_pid % 10) + '0');
            _pid /= 10;
            length++;
        } 

        // Get the last digit in case the number < 10
        chars[length] = (char)((_pid % 10) + '0');
        length++;

        // Print the number one string at a time
        for(i = 0; i < length; i++) {
            ShowCharCall(pid + 1, length - i - 1, chars[i]);
        }

        SleepCall(50);

        // Overwrite the same locations with spaces to effectively "erase" the number
        for(i = 0; i < length; i++) {
            ShowCharCall(pid + 1, length - i - 1, ' ');
        }

        SleepCall(50);
        MuxOpCall(vid_mux, UNLOCK);
    }
}
