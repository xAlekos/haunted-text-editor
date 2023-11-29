#include <ncurses.h>
#include <stdio.h>
#include <locale.h>
#include "gap_buffer.h"

#define ctrl(x)           ((x) & 0x1f)

void printgapbuftocurses(GapBuf* gapbuf){

    int row,col ;
    int x;
    int y;
    getyx(stdscr, y, x);
    getmaxyx(stdscr,row,col);
    erase();  
      for(int i = 0; i<gap_front(gapbuf);i++){
        addch(gapbuf->buff[i]);       
    }  
	addch('|');
    for(int i = gapbuf->gapend; i<gapbuf->buff_size;i++){
        addch(gapbuf->buff[i]);
    }
    mvprintw(row/2 , 0,"Line: %d\nCol: %d\nTotlines:%d",gapbuf->line,givecolumn(gapbuf),gapbuf->totlines);
    move(y,x+1);
    refresh();
}

int main()
{	
    setlocale(LC_ALL, "it_IT.UTF-8");
	initscr();			/* Start curses mode 		  */
	raw();
    noecho();
    curs_set(0);
    keypad(stdscr,true);
    GapBuf* nuovobuf = newbuffer(10);
    int ch = 0;
    printgapbuftocurses(nuovobuf);
    while(ch != ctrl('x')){
        ch = getch();
        switch(ch){
            case KEY_BACKSPACE : backspace(nuovobuf); printgapbuftocurses(nuovobuf); break;
            case KEY_DC : del(nuovobuf); printgapbuftocurses(nuovobuf); break;
            case KEY_LEFT : cursor_left(nuovobuf); printgapbuftocurses(nuovobuf); break;
            case KEY_RIGHT : if(cursor_right(nuovobuf)); printgapbuftocurses(nuovobuf); break;
            case KEY_UP : cursor_up(nuovobuf); printgapbuftocurses(nuovobuf); break; 
            case KEY_DOWN : cursor_down(nuovobuf); printgapbuftocurses(nuovobuf); break; 
            default : insert(nuovobuf, ch); printgapbuftocurses(nuovobuf); break;
        }
        
    }
    endwin();
    printf("\nLINES : %d\n",nuovobuf->totlines);
    freebuf(nuovobuf);
    return 0;
}
