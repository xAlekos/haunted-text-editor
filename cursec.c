#include <ncurses.h>
#include <stdio.h>
#include <locale.h>
#include "gap_buffer.h"
#define ctrl(x)           ((x) & 0x1f)


typedef struct printinfo{
    int min_x;
    int max_x;
    int standard_max_x;
    int min_y;
    int max_y;
    int standard_max_y;
} PrintInfo;

PrintInfo* newprintinfo(){
    PrintInfo* info = malloc(sizeof(PrintInfo));
    int y = getmaxy(stdscr);
    info->standard_max_x = 5 ;
    info->max_x = 5;
    info->min_x = 5;

    info->standard_max_y = y - 2;
    info->min_y = 0;
    info->max_y = y - 2;
    return info;
}

void printcursorinfo(GapBuf* gapbuf){
int row;
int x;
int y;
getyx(stdscr, y, x);
row = getmaxy(stdscr);
mvprintw(row - 1 , 0,"Ln: %d Col: %d",gapbuf->line,givecolumn(gapbuf));
move(y,x);
refresh();
}



void printgapbuftocursesfromto(GapBuf* gapbuf,int lns,int lnend,int cs, int ce){

    int char_line = 1; //in che riga si trova il char che verrà stampato
    int char_col = 1; //in che colonna si trova il char che verrà stampato
    erase();  
    for(int i = 0; i<gap_front(gapbuf);i++){
            if(char_line  >= lns && char_line  <= lnend){ //stampa solo le righe nel range contenibile nello schermo.
                addch(gapbuf->buff[i]);     
            }
            if(gapbuf->buff[i] == '\n' ){
                char_line += 1;
                char_col = 0;
            }
            
    }  
	addch('|');
    for(int i = gapbuf->gapend; i<gapbuf->buff_size;i++){
        if(char_line  >= lns && char_line  <= lnend){ //stampa solo le righe nel range contenibile nello schermo.
                addch(gapbuf->buff[i]);     
            }
            if(gapbuf->buff[i] == '\n' ){
                char_line += 1;
                char_col = 0;
            }
    }
    refresh();
}

void printgapbuftocurses(GapBuf* gapbuf,PrintInfo* info){
    //facciamo finta che lo schermo sia largo 5

    if(gapbuf->line > info->max_y){ //quando il cursore va sotto il limite dello schermo,
        info->min_y+=info->standard_max_y / 2; //si stampa di nuovo da sopra.
        info->max_y+=info->standard_max_y / 2;
    }
    if(gapbuf->line < info->min_y){
        info->min_y-=info->standard_max_y / 2;
        info->max_y-=info->standard_max_y / 2;
    }

    if(givecolumn(gapbuf) > info->max_x){
         info->min_x+=info->standard_max_x; //si stampa di nuovo da sopra.
         info->max_x+=info->standard_max_x;
    }
    if(givecolumn(gapbuf) < info->min_x){
         info->min_x-=info->standard_max_x; //si stampa di nuovo da sopra.
         info->max_x-=info->standard_max_x;
    }



    printgapbuftocursesfromto(gapbuf, info->min_y, info->max_y,info->min_x,info->max_x);
    printcursorinfo(gapbuf);
}

int main()
{	
    setlocale(LC_ALL, "");
	initscr();			/* Start curses mode 		  */
	raw();
    noecho();
    curs_set(0);
    keypad(stdscr,true);
    GapBuf* nuovobuf = newbuffer(1024);
    PrintInfo* info = newprintinfo();
    int ch = 0;
    printgapbuftocurses(nuovobuf,info);
    while(ch != ctrl('x')){
        ch = getch();
        switch(ch){
            case KEY_BACKSPACE : 
                                if(backspace(nuovobuf))
                                    memorizeinput(KEY_BACKSPACE,nuovobuf->buff[nuovobuf->cursor],1,nuovobuf);
                                printgapbuftocurses(nuovobuf,info);
                                break;
            case KEY_DC : 
                            if(del(nuovobuf))
                                memorizeinput(KEY_DC,nuovobuf->buff[nuovobuf->gapend - 1],1,nuovobuf);        
                            printgapbuftocurses(nuovobuf,info);
                            break;
            case KEY_LEFT :                             
                            if(cursor_left(nuovobuf))
                                memorizeinput(KEY_LEFT,0,0,nuovobuf);
                            printgapbuftocurses(nuovobuf,info);
                            break;
            case KEY_RIGHT :                            
                            if(cursor_right(nuovobuf))
                                memorizeinput(KEY_RIGHT,0,0,nuovobuf);
                            printgapbuftocurses(nuovobuf,info);
                            break;
            case KEY_UP : 
                            if(cursor_up(nuovobuf))
                                memorizeinput(KEY_UP,nuovobuf->col_mem,0,nuovobuf);
                            printgapbuftocurses(nuovobuf,info);
                            break; 
            case KEY_DOWN : 
                            if(cursor_down(nuovobuf))
                                memorizeinput(KEY_DOWN,nuovobuf->col_mem,0,nuovobuf);
                            printgapbuftocurses(nuovobuf,info);
                            break;
            case ctrl('z'):
                            undo(nuovobuf);
                            printgapbuftocurses(nuovobuf,info);
                            break;
            case ctrl('y'):
                            redo(nuovobuf);
                            printgapbuftocurses(nuovobuf,info);
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
                        printgapbuftocurses(nuovobuf,info);
                        break;
        }
        
    }
    endwin();
    freebuf(nuovobuf);
    return 0;
}
