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
    info->standard_max_x = x - 4;
    info->max_x = info->standard_max_x;
    info->min_x = 0;

    info->standard_max_y = y - 3;
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
attron(A_STANDOUT);
mvprintw(row - 1 , 0,"Ln: %d Col: %d-%d",gapbuf->line,givecolumn(gapbuf),gapbuf->col_mem);
attroff(A_STANDOUT);
move(y,x);
refresh();
}

void printtopbar(GapBuf* gapbuf){
    unsigned int  x=getmaxx(stdscr);
    attron(A_STANDOUT);
    unsigned int namelen = strlen(gapbuf->filename);
    int odd = namelen % 2 == 0 ? 0 : 1;
    int j=0;
    for (int i = 0;i<x;i++){
        if(odd == 0){
            if(i >= (x/2) - namelen/2 && i < (x/2) + namelen/2)
                addch(gapbuf->filename[j++]);
            else
                addch(' ');
        }
        else{
            if(i >= (x/2) - namelen/2 && i <= (x/2) + namelen/2)
                addch(gapbuf->filename[j++]);
            else
                addch(' ');
        }
    }
    attroff(A_STANDOUT);
    refresh();
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
    //visto CHE ciò che non entra nello schermo non viene stampato,
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

void checkybounds(GapBuf* gapbuf, PrintInfo* info){
    if(gapbuf->line > info->max_y){ //quando il cursore va sotto il limite dello schermo,
        info->min_y+=info->standard_max_y / 2; //si stampa di nuovo da sopra.
        info->max_y+=info->standard_max_y / 2;
    }
    if(gapbuf->line < info->min_y){
        info->min_y-=info->standard_max_y / 2;
        info->max_y-=info->standard_max_y / 2; 
    }
}

void checkxbounds(GapBuf* gapbuf, PrintInfo* info){
    int col = givecolumn(gapbuf);
    
    if(col > info->max_x){
         info->min_x+=info->standard_max_x; //si stampa di nuovo da sopra.
         info->max_x+=info->standard_max_x;
    }
    if(col < info->min_x){
         info->min_x-=info->standard_max_x; //si stampa di nuovo da sopra.
         info->max_x-=info->standard_max_x;
    }

}

void printgapbuftocursesfromto(GapBuf* gapbuf,PrintInfo* info){
    int old;
    int char_line = 1; //in che riga si trova il char che verrà stampato
    int char_col = 1; //in che colonna si trova il char che verrà stampato
    int exceeding_line = 1; //flag usata per decidere quando stampare un \n 
    for(int i = 0; i<gap_front(gapbuf);i++){
        checkxbounds(gapbuf,info);
        checkybounds(gapbuf,info);
        if(!islineoutofbound(char_line,info)){ //stampa solo le righe nel range contenibile nello schermo.
            old = gapbuf->line == char_line ? 0 : 1;
                if(!ischaroutofbound(char_col,info,old)) //stampa solo le colonne nel range dello schermo
                    addch(gapbuf->buff[i]);
                addlnifneeded(char_col,info,&exceeding_line,old);
        }
        updatelinecol(gapbuf->buff[i], &char_line, &char_col,&exceeding_line);
    }  
	addch('|');
    for(int i = gapbuf->gapend; i<gapbuf->buff_size;i++){
       checkxbounds(gapbuf,info);
       checkybounds(gapbuf,info);
       if(!islineoutofbound(char_line,info)){ //stampa solo le righe nel range contenibile nello schermo.
            old = gapbuf->line == char_line ? 0 : 1;
                if(!ischaroutofbound(char_col,info,old))
                    addch(gapbuf->buff[i]);
                addlnifneeded(char_col,info,&exceeding_line,old);
        }
        updatelinecol(gapbuf->buff[i], &char_line, &char_col,&exceeding_line);
    }
    refresh();
}

void printtextbox(char* input_message){
    int x,y;
    int x_max,y_max;
    getmaxyx(stdscr,y_max,x_max);
    getyx(stdscr,y,x);
    int input_message_len = strlen(input_message);
    move(y_max - 2,x_max/2 - strlen(input_message) / 2);
    attron(A_STANDOUT);
    for(int i = 0; i < input_message_len; i++)
        addch(input_message[i]);
    attroff(A_STANDOUT);
    refresh();
}

char* takeinput(){
    int x_max,y_max;
    getmaxyx(stdscr, y_max,x_max);
    echo();
    char* input = malloc(257);
    move(y_max - 1, 17);
    getnstr(input,256);
    noecho();
    return input;
}


void printgapbuftocurses(GapBuf* gapbuf,PrintInfo* info){
    erase(); 
    printtopbar(gapbuf);
    checkxbounds(gapbuf,info);
    checkybounds(gapbuf,info);
    printgapbuftocursesfromto(gapbuf,info);
    printcursorinfo(gapbuf);
}

void asktochangename(GapBuf* gapbuf,PrintInfo* info){
    char message[500];
    printgapbuftocurses(gapbuf,info);
    snprintf(message,500,"File name is : %s. Change File Name ? (y/n)",gapbuf->filename);
    printtextbox(message);
    int found = 0;
    while(found == 0){
        char* answer = takeinput();
        if (strcmp(answer,"y") == 0){
            found = 1;
            printgapbuftocurses(gapbuf,info);
            printtextbox(" -- SELECT NEW FILE NAME --");
            free(answer);
            answer=takeinput();
            strcpy(gapbuf->filename,answer);
            snprintf(message,500,"-- SAVING FILE AS %s -- ",gapbuf->filename);
            printgapbuftocurses(gapbuf,info);
            printtextbox(message);
            free(answer);
            save(gapbuf);

        }
        else if(strcmp(answer,"n") == 0){
            found = 1;
            snprintf(message,500,"-- FILE SAVED AS %s -- ",gapbuf->filename);
            printgapbuftocurses(gapbuf,info);
            printtextbox(message);
            free(answer);
            save(gapbuf);
        }
    }
}

int main(int argc, char* argv[])
{	
    setlocale(LC_ALL, "");
	initscr();			/* Start curses mode 		  */
	raw();
    noecho();
    curs_set(0);
    keypad(stdscr,true);
    GapBuf* nuovobuf = newbuffer(MAX_BUF_SIZE);
    PrintInfo* info = newprintinfo();
    int ch = 0;
    if(argc > 1){
       if(strlen(argv[1]) >= 256){
            printf("File name exceeding limit");
            return 0;
       }
       load(argv[1],nuovobuf);
       strcpy(nuovobuf->filename , argv[1]);
    }
    else{
        strcpy(nuovobuf->filename ,"New File");
    }

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
            case ctrl('s'):
                            asktochangename(nuovobuf,info);
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
