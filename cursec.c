#include <ncurses.h>
#include <stdio.h>
#include <locale.h>
#include "gap_buffer.h"


void printgapbuftocurses(GapBuf* gapbuf){
    erase();  
      for(int i = 0; i<gap_front(gapbuf);i++){
        addch(gapbuf->buff[i]);       
    }  
	addch('|');
    for(int i = gapbuf->gapend; i<gapbuf->buff_size;i++){
        addch(gapbuf->buff[i]);
    }
    refresh();
}

int main()
{	
    setlocale(LC_ALL, "it_IT.UTF-8");
	initscr();			/* Start curses mode 		  */
	cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr,true);
    GapBuf* nuovobuf = newbuffer(10);
    int ch = 0;
    while(ch != '-'){
        ch = getch();
        switch(ch){
            case KEY_BACKSPACE : backspace(nuovobuf); printgapbuftocurses(nuovobuf); break;
            case KEY_DC : del(nuovobuf); printgapbuftocurses(nuovobuf); break;
            case KEY_LEFT : cursor_left(nuovobuf); printgapbuftocurses(nuovobuf); break;
            case KEY_RIGHT : if(cursor_right(nuovobuf)); printgapbuftocurses(nuovobuf); break;
            default : insert(nuovobuf, ch); printgapbuftocurses(nuovobuf); break;
        }
        
    }
    endwin();
    printf("cazzo marmelalta");
    return 0;
}
