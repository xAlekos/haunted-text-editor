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
    int x = getmaxx(stdscr);
    info->standard_max_x = x - 3;
    info->max_x = info->standard_max_x;
    info->min_x = 0;

    info->standard_max_y = y - 2;
    info->min_y = 0;
    info->max_y = info->standard_max_y;
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


bool iscursoroutofbound(GapBuf* gapbuf, PrintInfo* info){
    if(givecolumn(gapbuf) > info->standard_max_x)
        return true;
    else
        return false;
}

bool ischaroutofbound(int col, PrintInfo* info,int old){
      int min,max;
    if(old == 1){
        max = info->standard_max_x;
        min = 0;
    }
    else{
        max = info->max_x;
        min = info->min_x;
    }   
    if(col > max || col < min)
        return true;
    else
        return false;
}

bool islineoutofbound(int line, PrintInfo* info){
     if(line > info->max_y || line < info->min_y)
        return true;
    else
        return false;
    
}

void updatelinecol(int ch,int* line, int* col, int* exceedingline){
    if(ch == '\n' ){
            *line += 1;
            *exceedingline = 1;
            *col = 1;
            }
    else
        *col+=1;

}

void addlnifneeded(int col , PrintInfo* info, int* exceeding_line, int old){
    //visto  ciò che non entra nello schermo non viene stampato,
    //questa funzione si assicura di far andare a capo anche se il \n presente nel buffer non viene stampato.
    //old indica se vuoi che il \n venga messo dopo che ogni carattere abbia raggiunto il limite standard o se 
    //invece prevedi che si scorrera orizzontalmente nel testo old utilizzerà il valore aggiornato del massimo.
    //PS: se non è chiaro lo so. è colpa mia. è stato un parto far funzionare questa cosa :(
    int max;
    if(old == 1)
        max = info->standard_max_x;
    else
        max = info->max_x;
    
    if(col > max && *exceeding_line == 1){
                    addch('#');
                    addch('\n');
                    *exceeding_line = 0;
                }
}

void printgapbuftocursesfromto(GapBuf* gapbuf,PrintInfo* info){

    int char_line = 1; //in che riga si trova il char che verrà stampato
    int char_col = 1; //in che colonna si trova il char che verrà stampato
    int exceeding_line = 1; 
    erase();  
    for(int i = 0; i<gap_front(gapbuf);i++){
        if(!islineoutofbound(char_line,info)){ //stampa solo le righe nel range contenibile nello schermo.
            if(gapbuf->line == char_line){
                if(!ischaroutofbound(char_col,info,0))
                    addch(gapbuf->buff[i]);
                addlnifneeded(char_col,info,&exceeding_line,0);
            }
            else{
                if(!ischaroutofbound(char_col,info,1))
                    addch(gapbuf->buff[i]);
                addlnifneeded(char_col,info,&exceeding_line,1);
            }

        }
        updatelinecol(gapbuf->buff[i], &char_line, &char_col,&exceeding_line);
    }  
	addch('|');
    for(int i = gapbuf->gapend; i<gapbuf->buff_size;i++){
       if(!islineoutofbound(char_line,info)){ //stampa solo le righe nel range contenibile nello schermo.
            if(gapbuf->line == char_line){
                if(!ischaroutofbound(char_col,info,0))
                    addch(gapbuf->buff[i]);
                addlnifneeded(char_col,info,&exceeding_line,0);
            }
            else{
                if(!ischaroutofbound(char_col,info,1))
                    addch(gapbuf->buff[i]);
                addlnifneeded(char_col,info,&exceeding_line,1);
            }

        }
        updatelinecol(gapbuf->buff[i], &char_line, &char_col,&exceeding_line);
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



    printgapbuftocursesfromto(gapbuf,info);
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
                        if(ch == 32 && iscursoroutofbound(nuovobuf,info))
                            addch('\n');
                        break;
        }
        
    }
    endwin();
    freebuf(nuovobuf);
    return 0;
}
