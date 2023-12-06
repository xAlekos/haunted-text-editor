#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/*
-1 : br
1 = insert qualsiasi cosa
2 = backspace
3 = canc
4 = cursr
5 = cursl
6 = insert \n
7 = insert spazio

*/





int main(){
int a = 0;
int storia[500];
int sp=0;
while(a != 8){  //INPUT 

scanf("%d",&a);
switch(a){
    default: break;
    case 1: printf("ho fatto un ins\n");
            if(sp > 0 && storia[sp-1] == 1){
                storia[sp++] = 1; 
                break;
            }
            else{
                storia[sp++] = -1;
                storia[sp++] = 1;
            }
                break;
    case 2: printf("ho fatto un backspace\n");storia[sp++] = -1; storia[sp++] = 2; break;
    case 3: printf("ho fatto un canc\n");storia[sp++] = -1; storia[sp++] = 3; break;
    case 4: printf("ho fatto un cursr\n");storia[sp++] = 4; break;
    case 5: printf("ho fatto un cursl\n");storia[sp++] = 5; break;
    case 6: printf("ho inserito una lain\n");storia[sp++] = -1; storia[sp++] = 6; break;
    case 7: printf("ho inserito uno spazio\n");
            if(sp > 0 && storia[sp-1] == 7){
                storia[sp++] = 7; 
                break;
            }
            else{
                storia[sp++] = -1;
                storia[sp++] = 7;
            }
                break;
}
while ((getchar()) != '\n'); 
}// SIMULA INPUT CHE RIEMPE LA PILA DELLE MODIFICHE


//PARTE CHE SIMULA IL CONTROL Z. A SECONDA DI QUANTI CTRL Z VOGLIAMO FARE RAGGIUGEREMO N BREAKPOINT.
    int ctrlz = 0;
    printf("\nquanti ctrl z vuoi fare?\n");
    sp-=1;
    scanf("%d",&ctrlz);
    while(sp > 0 && ctrlz > 0){
        while(storia[sp] != -1){
            switch(storia[sp]){
                case 1: printf("era stato fatto un ins, faccio un backspace\n"); sp--; break;
                case 2: printf("era stato fatto un backspace, faccio un ins\n"); sp--; break;
                case 3: printf("era stato fatto un canc, faccio un ins e un cur_left\n"); sp--; break;
                case 4: printf("era stato fatto un cursr, faccio un cursl\n"); sp--; break;
                case 5: printf("era stato fatto un cursl , faccio un cursr\n"); sp--; break;
                case 6: printf("era stato ins un \n, faccio backspace\n"); sp--; break;
                case 7: printf("era stato ins uno spazio, faccio backspace\n"); sp--; break;
            }
            
        }
        ctrlz-=1;
    }

}