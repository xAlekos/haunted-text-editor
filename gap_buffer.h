#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MIN_BUF_SIZE 5
#define MAX_BUF_SIZE 65535
#define CURSPOS (gapbuf->cursor)

typedef struct gap_buf{
    int totlines;
    int line;
    char* buff;
    int buff_size;
    int cursor; //first gapspace
    int gapend; //last gapspace
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
    newgap_buf->buff = newbuff;
    newgap_buf->buff_size = initsize;
    newgap_buf->cursor = 0;
    newgap_buf->totlines = 1;
    newgap_buf->line = 1;
    newgap_buf->gapend = initsize;
    return newgap_buf;
}

void freebuf(GapBuf* gapbuf){
    if(!gapbuf)
        return;
    free(gapbuf->buff);
    free(gapbuf);
}

void move_back(GapBuf* gapbuf, char* new_buf , int new_size){
    memmove(new_buf + new_size - gap_back(gapbuf), gapbuf->buff + gap_back(gapbuf),gap_back(gapbuf));
            // nel back del nuovo buffer            //dal back del vecchio buffer , tutti gli elementi nel back


}

void shrink_buffer(GapBuf* gapbuf, int newsize){ //dimezza il buffer quando il gap è troppo grande rispetto ad i caratteri utilizzati
    newsize = newsize >  MIN_BUF_SIZE ? newsize : MIN_BUF_SIZE;
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
    char* newbuff = (char*)realloc(gapbuf->buff, newsize);
    move_back(gapbuf,newbuff,newsize);
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
    if(c == '\n'){ //controlla se sei andato a capo, aggiungendo una riga
        gapbuf->totlines+=1;
        gapbuf->line+=1;
    }
    return true;
}

void cursor_left(GapBuf* gapbuf){
    if (gapbuf->cursor > 0){
        if(gapbuf->buff[gapbuf->cursor - 1] == '\n')
            gapbuf->line-=1; //controlla se sei salito di una riga
        gapbuf->buff[--gapbuf->gapend] = gapbuf->buff[--gapbuf->cursor];
        
    }

}

bool cursor_right(GapBuf* gapbuf){
    if(gapbuf->gapend == gapbuf->buff_size) 
        return false;

    if (gapbuf->cursor < gapbuf->buff_size){
        if(gapbuf->buff[gapbuf->gapend] == '\n') //controlla se sei sceso nella prossima riga
            gapbuf->line +=1;
        gapbuf->buff[gapbuf->cursor++] = gapbuf->buff[gapbuf->gapend++];
        return true;
    }
    else  
        return false;
    
}

void backspace(GapBuf* gapbuf){ //elimina l'elemento a sinistra del cursore
    if(gapbuf->cursor > 0){
        gapbuf->cursor--;
        if(gapbuf->buff[gapbuf->cursor] == '\n'){
            gapbuf->line -=1;//controlla se sei salito di una riga
            gapbuf->totlines -= 1;
        }
    }
    if (gap_used(gapbuf) < gapbuf->buff_size/4)
        shrink_buffer(gapbuf, gapbuf->buff_size/2);
}

void del(GapBuf* gapbuf){ //elimina l'elemento a destra del cursore
    if(gapbuf->gapend < gapbuf -> buff_size){
        if(gapbuf->buff[gapbuf->gapend] == '\n')//controlla se hai cancellato una line
            gapbuf->totlines-=1;
        gapbuf->gapend++;
    }
    if (gap_used(gapbuf) < gapbuf->buff_size/4)
        shrink_buffer(gapbuf, gapbuf->buff_size/2);


}



int givecolumn(GapBuf* gapbuf){
    int i = 0;
    while(gapbuf->cursor - i != 0 && gapbuf->buff[gapbuf->cursor - i] != '\n')
        i++;
    if(gapbuf->cursor == 0)
        return 1;
    else
        return i;


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