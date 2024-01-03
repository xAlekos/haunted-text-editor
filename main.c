#include "cursesprint.h"

bool namefile(GapBuf* nuovobuf,int count, char* argv[]){
    if(count > 1){
       if(strlen(argv[1]) >= 256){
            printf("File name exceeding limit");
            return false;
       }
       load(argv[1],nuovobuf);
       strcpy(nuovobuf->filename , argv[1]);
       return true;
    }
    else{
        strcpy(nuovobuf->filename ,"New File");
        return true;
    }
}

void initialize_curses(){
    setlocale(LC_ALL, "");
	initscr();			
	raw();
    noecho();
    curs_set(0);
    keypad(stdscr,true);
}

void writetoeditor(GapBuf* nuovobuf, PrintInfo* info){
    int ch;
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
}

int main(int argc, char* argv[])
{	
    initialize_curses();
    GapBuf* nuovobuf = newbuffer(MAX_BUF_SIZE);
    PrintInfo* info = newprintinfo();
    if(!namefile(nuovobuf,argc,argv))
        return 0;
    printgapbuftocurses(nuovobuf,info);
    writetoeditor(nuovobuf,info);
    endwin();
    freebuf(nuovobuf);
    free(info);
    return 0;
}
