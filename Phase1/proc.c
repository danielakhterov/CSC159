#include "k-const.h"
#include "k-data.h"

// Delay CPU for half a second by 'inb $0x80'
void Delay(void) {
    int i;
    
    // Loop to try and delay CPU for about half a second
    for(i = 0; i <= LOOP/2; i++) {
        asm("inb $0x80");
    }
}

// Show ch at row, col
void ShowChar(int row, int col, char ch) {
    // Upper-left corner of display
    unsigned short * p = VID_HOME;
    p += 80 * row + col;
    *p = ch + VID_MASK;
}

void InitProc(void) {
    while(1) {
        // show a dot at upper-left corener of PC
        ShowChar(0, 0, '.');
        // wait for about half a second
        Delay();

        // erase dot
        ShowChar(0, 0, ' ');
        // wait for about half a second
        Delay();
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
    while(1) {
        // show 1st digit of my PID at row run_pid+1, 1st col
        // show 2nd digit of my PID at row run_pid+2, 2nd col
        int _pid;
        int length;
        char _chars[11] = "0000000000";
        int i;

        _pid = run_pid;
        length = 0;

        while(_pid / 10 > 0) {
            _chars[length] = (char)((_pid % 10) + '0');
            _pid /= 10;
            length++;
        } 

        _chars[length] = (char)((_pid % 10) + '0');
        length++;

        for(i = 0; i < length; i++) {
            ShowChar(run_pid + 1, length - i - 1, _chars[i]);
        }

        // wait for about half a second
        Delay();

        // erase above writing (one digit at a time)
        for(i = 0; i < length; i++) {
            ShowChar(run_pid + 1, length - i - 1, ' ');
        }

        // wait for about half a second
        Delay();
    }
}
