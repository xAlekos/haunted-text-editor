#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MIN_BUF_SIZE 1024
#define MAX_BUF_SIZE 65535
#define HISTORY_MAX 2048
#define CURSPOS (gapbuf->cursor) 



typedef struct history_data{
    int operation;
    int ch; //il carattere sul quale è stata effettuata l'operazione
} HistoryData;


typedef struct gap_buf{
    HistoryData history[HISTORY_MAX];
    int historypointer; 
    int historypointermax;
    int totlines;
    int line;
    int col_mem; //ultima colonna nella quale ci si è spostati.
    char* buff; 
    int buff_size;
    int cursor; //first gapspace
    int gapend; //primo carattere dopo il gap
} GapBuf;


#define gap_back(buf) ((buf)->buff_size - (buf)->gapend) //dim testo dopo il gap
#define gap_front(buf) ((buf)->cursor) //dim testo prima del gap 
#define gap_used(buf)  (gap_back(buf) + gap_front(buf)) //dim totali testo


GapBuf* newbuffer(int initsize){
    initsize = initsize > MIN_BUF_SIZE ? initsize : MIN_BUF_SIZE;
    GapBuf* newgap_buf = (GapBuf*)malloc(sizeof(GapBuf));
    if (!newgap_buf)
        return NULL;
    char* newbuff = (char*)malloc(initsize);
    if (!newbuff)
        return NULL;
    newgap_buf->buff = newbuff; //riempo tutti i campi
    newgap_buf->buff_size = initsize;
    newgap_buf->cursor = 0;
    newgap_buf->totlines = 1;
    newgap_buf->line = 1;
    newgap_buf->col_mem = 1;
    newgap_buf->gapend = initsize;
    newgap_buf->historypointer = 0;
    newgap_buf->historypointermax = 0;
    return newgap_buf;
}

void freebuf(GapBuf* gapbuf){
    if(!gapbuf)
        return;
    free(gapbuf->buff);
    free(gapbuf);
}

void move_back(GapBuf* gapbuf, char* new_buf , int new_size){
    memmove(new_buf + new_size - gap_back(gapbuf), gapbuf->buff + gapbuf->gapend,gap_back(gapbuf));
            // nel back del nuovo buffer            //dal back del vecchio buffer , tutti gli elementi nel back


}

void shrink_buffer(GapBuf* gapbuf, int newsize){ //dimezza il buffer quando il gap è troppo grande rispetto ad i caratteri utilizzati    newsize = newsize >  MIN_BUF_SIZE ? newsize : MIN_BUF_SIZE;
    newsize = newsize < MIN_BUF_SIZE ? MIN_BUF_SIZE : newsize;
    if(newsize < gap_used(gapbuf))
        return;
    move_back(gapbuf, gapbuf->buff, newsize); //faccio moveback sullo stesso buffer con nuova size più piccola così che quello che era prima il back si avvicina al front,
    gapbuf->gapend = newsize-gap_back(gapbuf);//poi tutto ciò che rimane in fondo verrà cancellato quando si reallocherà
    gapbuf->buff_size = newsize;
    char* newbuff = (char*)realloc(gapbuf->buff, newsize);
    if(newbuff)
        gapbuf->buff = newbuff;
}

bool grow_buffer(GapBuf* gapbuf, int newsize){ //raddoppia il buffer quando il gap non è più sufficente
    newsize = newsize < SIZE_MAX ? newsize : SIZE_MAX;
    if(gapbuf->buff_size >= newsize)
        return false;
    char* newbuff = (char*)malloc(newsize); //alloca il nuovo buffer e copia il contenuto di quello vecchio così com'è
    memmove(newbuff,gapbuf->buff,gapbuf->buff_size); 
    if (!newbuff) return false;
    move_back(gapbuf,newbuff,newsize); //dopodichè sposta il back verso la fine
    free(gapbuf->buff);                 //del nuovo buf così da creare nuovo spazio in cui scrivere, poi libera la memoria del vecchio buffer 
    gapbuf->buff = newbuff;
    gapbuf->gapend = newsize - gap_back(gapbuf);
    gapbuf->buff_size = newsize;
    return true;
}

bool insert(GapBuf* gapbuf,char c){
    
    if(gapbuf->cursor == gapbuf->gapend){
        int newsize = gapbuf->buff_size < SIZE_MAX/2 ? 2* gapbuf->buff_size  : SIZE_MAX; //raddoppia le dimensioni del buffer se non c'è spazio 
        if(!grow_buffer(gapbuf,newsize))
            return false;
    }
    gapbuf->buff[gapbuf->cursor++] = c;
    gapbuf->col_mem+=1; //mantiene la memoria dell'ultimo spostamenoto orizzontale del cursore(in questo caso verso destra)
    if(c == '\n'){ //controlla se sei andato a capo, aggiungendo una riga
        gapbuf->totlines+=1;
        gapbuf->col_mem=1; //porta a 0 lo spostamento orizzontale del cursore da mentenere in memoria(sei andato a capo)
        gapbuf->line+=1;
    }
    return true;
}

int givecolumn(GapBuf* gapbuf){ //funzione che ti ritorna in quale colonna
                                 //si trova il cursore(serve a cursup e cursor down)
    int i = 1;
    while(gapbuf->cursor - i >= 0 && gapbuf->buff[gapbuf->cursor - i] != '\n')
        i++;
        return i;


}

bool cursor_left(GapBuf* gapbuf){
    if (gapbuf->cursor > 0){
        if(gapbuf->buff[gapbuf->cursor - 1] == '\n')
            gapbuf->line-=1; //controlla se sei salito di una riga
        gapbuf->buff[--gapbuf->gapend] = gapbuf->buff[--gapbuf->cursor];
        gapbuf->col_mem = givecolumn(gapbuf);//mantiene la memoria dell'ultimo spostamenoto orizzontale del cursore
        return true;
    }
    else 
        return false;
    

}

bool cursor_right(GapBuf* gapbuf){
    if(gapbuf->gapend == gapbuf->buff_size) 
        return false;

    if (gapbuf->cursor < gapbuf->buff_size){
        if(gapbuf->buff[gapbuf->gapend] == '\n') //controlla se sei sceso nella prossima riga
            gapbuf->line +=1;
        gapbuf->buff[gapbuf->cursor++] = gapbuf->buff[gapbuf->gapend++];
        gapbuf->col_mem = givecolumn(gapbuf);//mantiene la memoria dell'ultimo spostamenoto orizzontale del cursore
        return true;
    }
    else  
        return false;
    
}

bool backspace(GapBuf* gapbuf){ //elimina l'elemento a sinistra del cursore
    if(gapbuf->cursor > 0){
        gapbuf->cursor--;
        if(gapbuf->col_mem>1)
            gapbuf->col_mem-=1;//mantiene la memoria dell'ultimo spostamenoto orizzontale del cursore(verso sinsitra in questo caso)
        if(gapbuf->buff[gapbuf->cursor] == '\n'){
            gapbuf->line -=1;//controlla se sei salito di una riga
            gapbuf->totlines -= 1;
        }
    if (gap_used(gapbuf) < gapbuf->buff_size/4)
        shrink_buffer(gapbuf, gapbuf->buff_size/2);
    return true;
    }
    return false;
}

bool del(GapBuf* gapbuf){ //elimina l'elemento a destra del cursore
    if(gapbuf->gapend < gapbuf -> buff_size){
        if(gapbuf->buff[gapbuf->gapend] == '\n')//controlla se hai cancellato una line
            gapbuf->totlines-=1;
        gapbuf->gapend++;
    if (gap_used(gapbuf) < gapbuf->buff_size/4)
        shrink_buffer(gapbuf, gapbuf->buff_size/2);
    return true;
    }
    return false;
}


bool cursor_up(GapBuf* gapbuf){
    int col = gapbuf->col_mem; //mantiene in memoria l'ultimo spostamento di colonna per mantenerlo durante up e down
    int line = gapbuf->line; 
    if(gapbuf->line > 1){
        while((givecolumn(gapbuf) > col   || gapbuf->line != line - 1))
            cursor_left(gapbuf);
            
    
    gapbuf->col_mem=col; //mantiene in memoria l'ultimo spostamento di colonna per mantenerlo durante up e down
    return true;
    }
    else 
        return false;
}

bool cursor_down(GapBuf* gapbuf){
    int col = gapbuf->col_mem; //mantiene in memoria l'ultimo spostamento di colonna per mantenerlo durante up e down
    int col_now;
    int line = gapbuf->line; 
    if(gapbuf->line < gapbuf->totlines){
        while((col_now = givecolumn(gapbuf)) < col   || gapbuf->line < line + 1){
            if(!cursor_right(gapbuf))
                break;
            if(gapbuf->line == line + 1 && gapbuf->buff[gapbuf->gapend] == '\n')
                break;
        }
    
    gapbuf->col_mem=col; //mantiene in memoria l'ultimo spostamento di colonna per mantenerlo durante up e down
    return true;
    }
    else 
        return false;
} 

void memorizeinput(int op, int val, int setbr,  GapBuf* gapbuf){
//1 = ins char
// 2 = ins spazio
// 3 = ins newline

    gapbuf->historypointer %= HISTORY_MAX;

    switch(op){

    default: 
            if(setbr == 1){
                gapbuf->history[gapbuf->historypointer].operation=-1;
                gapbuf->historypointer++;  
            }
            gapbuf->history[gapbuf->historypointer].operation=op;
            gapbuf->history[gapbuf->historypointer].ch=val;
            gapbuf->historypointer++;  
            break;
    case 1:
            if(gapbuf->historypointer > 0 && (gapbuf->history[gapbuf->historypointer - 1 ]).operation == 1){
                gapbuf->history[gapbuf->historypointer].operation = 1; 
                gapbuf->history[gapbuf->historypointer].ch = val; 
                gapbuf->historypointer++;
                break;
            }
            else{
                gapbuf->history[gapbuf->historypointer++].operation = -1; //ad ogni primo char diverso da spazio mette un breakpoint
                gapbuf->history[gapbuf->historypointer].operation = 1;
                gapbuf->history[gapbuf->historypointer].ch = val; 
                gapbuf->historypointer++;
            }
                break;
    case 2:
            if(gapbuf->historypointer > 0 && (gapbuf->history[gapbuf->historypointer - 1 ]).operation == 2){
                gapbuf->history[gapbuf->historypointer].operation = 2; 
                gapbuf->history[gapbuf->historypointer].ch = val; 
                gapbuf->historypointer++;
                break;
            }
            else{
                gapbuf->history[gapbuf->historypointer++].operation = -1; //ad ogni primo spazio mette un breakpoint
                gapbuf->history[gapbuf->historypointer].operation = 1;
                gapbuf->history[gapbuf->historypointer].ch = val; 
                gapbuf->historypointer++;
            }
                break;
    case 3:     
                gapbuf->history[gapbuf->historypointer++].operation = -1; //ad ogni \n mette un breakpoint
                gapbuf->history[gapbuf->historypointer].operation = 3;
                gapbuf->history[gapbuf->historypointer].ch = val; 
                gapbuf->historypointer++;
                break;
    }
    gapbuf->historypointermax=gapbuf->historypointer;
}

void undo(GapBuf* gapbuf){//dalla pila delle azioni esce l'azione più recente e ne fa l'inverso.
    while(gapbuf->historypointer > 0 && gapbuf->history[gapbuf->historypointer - 1].operation != -1){
        switch(gapbuf->history[gapbuf->historypointer - 1].operation){
            default: 
                    backspace(gapbuf);
                    break;
            case 263: //backspace
                    insert(gapbuf,gapbuf->history[gapbuf->historypointer - 1].ch);
                    break;
            case 330: //canc
                    insert(gapbuf,gapbuf->history[gapbuf->historypointer - 1].ch);
                    cursor_left(gapbuf);
                    break;
            case 260: //left
                    cursor_right(gapbuf);
                    break;
            case 261: //right
                    cursor_left(gapbuf);
                    break;
            case 258 : //down
                    gapbuf->col_mem=gapbuf->history[gapbuf->historypointer - 1].ch;
                    cursor_up(gapbuf);
                    break;
            case 259 : //up
                    gapbuf->col_mem=gapbuf->history[gapbuf->historypointer - 1].ch;
                    cursor_down(gapbuf);
                    break;
            case -2: //growbuffer
                    break;
            case -3: //shrinkbuffer
                    break;

        }
        gapbuf->historypointer--;
    }
    if(gapbuf->historypointer>0)
        gapbuf->historypointer--;
}

void redo(GapBuf* gapbuf){//dalla pila delle azioni esce l'azione più recente e ne fa l'inverso.
    if(gapbuf->historypointer < gapbuf->historypointermax)
    gapbuf->historypointer++;
    while(gapbuf->historypointer < gapbuf->historypointermax && gapbuf->history[gapbuf->historypointer].operation != -1){
        switch(gapbuf->history[gapbuf->historypointer].operation){
            default: 
                    insert(gapbuf,gapbuf->history[gapbuf->historypointer].ch);
                    break;
            case 263: //backspace
                    backspace(gapbuf);
                    break;
            case 330: //canc
                    del(gapbuf);
                    break;
            case 260: //left
                    cursor_left(gapbuf);
                    break;
            case 261: //right
                    cursor_right(gapbuf);
                    break;
            case 258 : //down
                    gapbuf->col_mem=gapbuf->history[gapbuf->historypointer - 1].ch;
                    cursor_down(gapbuf);
                    break;
            case 259 : //up
                    gapbuf->col_mem=gapbuf->history[gapbuf->historypointer - 1].ch;
                    cursor_up(gapbuf);
                    break;
        }
        gapbuf->historypointer++;
    }

}







void printgapbuff(GapBuf* gapbuf){
    for(int i = 0; i<gap_front(gapbuf);i++){
        printf("%c",gapbuf->buff[i]);
            
    }
    
    printf("|");
    
    for(int i = gapbuf->gapend; i<gapbuf->buff_size;i++){
        printf("%c",gapbuf->buff[i]);
    }
    printf("\n");
}

/*int main(){
    
}*/