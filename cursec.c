#include <ncurses.h>
#include <stdio.h>
#include <locale.h>
#include "gap_buffer.h"
#define ctrl(x)           ((x) & 0x1f)

void printgapbuftocurses(GapBuf* gapbuf){

    int row,col;
    int x;
    int y;
    getyx(stdscr, y, x);
    getmaxyx(stdscr,row,col);
    erase();  
    for(int i = 0; i<gap_front(gapbuf);i++){
        addch(gapbuf->buff[i]);       
    }  
	addch('|');
    /*for(int i = gap_front(gapbuf); i<gapbuf->gapend;i++){
        addch('_');
    }
    */
    for(int i = gapbuf->gapend; i<gapbuf->buff_size;i++){
        addch(gapbuf->buff[i]);
    }
    mvprintw(row - 1 , 0,"Ln: %d Col; %d",gapbuf->line,givecolumn(gapbuf));
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
    GapBuf* nuovobuf = newbuffer(1024);
    int ch = 0;

    printgapbuftocurses(nuovobuf);
    while(ch != ctrl('x')){
        int cleft; //usate per contare quante volte facendo up si è andati a sinistra ed a destra,
        int cright; //per assicuarsi che in fase di undo non si sminchia 
        ch = getch();
        switch(ch){
            case KEY_BACKSPACE : 
                                if(backspace(nuovobuf))
                                    memorizeinput(KEY_BACKSPACE,nuovobuf->buff[nuovobuf->cursor],1,nuovobuf);
                                printgapbuftocurses(nuovobuf);
                                break;
            case KEY_DC : 
                            if(del(nuovobuf))
                                memorizeinput(KEY_DC,nuovobuf->buff[nuovobuf->gapend - 1],1,nuovobuf);        
                            printgapbuftocurses(nuovobuf);
                            break;
            case KEY_LEFT :                             
                            if(cursor_left(nuovobuf))
                                memorizeinput(KEY_LEFT,0,0,nuovobuf);
                            printgapbuftocurses(nuovobuf);
                            break;
            case KEY_RIGHT :                            
                            if(cursor_right(nuovobuf))
                                memorizeinput(KEY_RIGHT,0,0,nuovobuf);
                            printgapbuftocurses(nuovobuf);
                            break;
            case KEY_UP : 
                            if(cleft = cursor_up(nuovobuf))
                                memorizeinput(KEY_UP,cleft,0,nuovobuf);
                            printgapbuftocurses(nuovobuf);
                            break; 
            case KEY_DOWN : 
                            if(cright = cursor_down(nuovobuf))
                                memorizeinput(KEY_DOWN,cright,0,nuovobuf);
                            printgapbuftocurses(nuovobuf);
                            break;
            case ctrl('z'):
                            undo(nuovobuf);
                            printgapbuftocurses(nuovobuf);
                            break;
            case ctrl('y'):
                            redo(nuovobuf);
                            printgapbuftocurses(nuovobuf);
                            break;
            default :  
                        if(ch != 32 && ch != 10) //se è un char qualsiasi l'operazione è 1, se uno spazio è 2, se è enter l'operazione è 3
                            memorizeinput(1,ch,0,nuovobuf);
                        else{
                            if(ch == 32)
                               memorizeinput(3,ch,0,nuovobuf);
                            else
                               memorizeinput(2,ch,0,nuovobuf);
                        }
                        insert(nuovobuf, ch);
                        printgapbuftocurses(nuovobuf);
                        break;
        }
        
    }
    endwin();
    printf("\nLINES : %d\n",nuovobuf->totlines);
    freebuf(nuovobuf);
    return 0;
}
