#include "cursesprint.h"
#include <pthread.h> 
#include <unistd.h>
#include <pthread.h>

#define MAX_SKILLS 5

enum ghost_skills{
    MESS_INPUT,
    WRITE_MESSAGE,
    CHANGE_HISTORY,
    CHANGE_FILENAME,
    MESSAGE_BOX
};

typedef struct ghost_skill{
    int id;
    int required_rage; 
    bool cooling;
    int cooldown_time;
    pthread_t cooldown; 
} GhostSkill;

typedef struct ghost{
    char* name;
    GhostSkill* active_skill_list;
    GhostSkill* total_skill_list;
    int active_skills_num;
    int rage;
} Ghost;

void initialize_skill_list(Ghost* ghost){
    GhostSkill* total_skills = (GhostSkill* )malloc(sizeof(GhostSkill) * MAX_SKILLS);
    GhostSkill* active_skills = (GhostSkill* )malloc(sizeof(GhostSkill) * MAX_SKILLS);
    
    total_skills[0].id = MESS_INPUT; 
    total_skills[0].required_rage = 0;
    total_skills[0].cooling = false;
    total_skills[0].cooldown_time = 10;

    total_skills[1].id = WRITE_MESSAGE;
    total_skills[1].required_rage = 0;
    total_skills[1].cooling = false;
    total_skills[1].cooldown_time = 10;

    total_skills[2].id = CHANGE_HISTORY;
    total_skills[2].required_rage = 0;
    total_skills[2].cooling = false;
    total_skills[2].cooldown_time = 10;

    total_skills[3].id = CHANGE_FILENAME;
    total_skills[3].required_rage = 0;
    total_skills[3].cooling = false;
    total_skills[3].cooldown_time = 10;

    total_skills[4].id = MESSAGE_BOX;
    total_skills[4].required_rage = 0;
    total_skills[4].cooling = false;
    total_skills[4].cooldown_time = 10;

    ghost->total_skill_list= total_skills;
    ghost->active_skill_list= active_skills;
}

void update_active_skills(Ghost* ghost){
    for(int i = 0; i < MAX_SKILLS; i++){
        if(ghost->total_skill_list[i].required_rage <= ghost->rage){
            ghost->active_skill_list[ghost->active_skills_num].id = ghost->total_skill_list[i].id;
            ghost->active_skill_list[ghost->active_skills_num].cooldown_time = ghost->total_skill_list[i].cooldown_time;
            ghost->active_skill_list[ghost->active_skills_num].cooling = false;
            ghost->active_skill_list[ghost->active_skills_num++].required_rage = ghost->total_skill_list[i].required_rage;
            ghost->total_skill_list[i].required_rage = 1000; 
        }
    }
}

void printskills(Ghost* ghost){
    printf("Total skil list:\n");
    for(int i = 0; i< MAX_SKILLS ; i++){
        printf("ID : %d\nRR : %d\n",ghost->total_skill_list[i].id, ghost->total_skill_list[i].required_rage);
    }
    printf("Active skill list:\n");
    for(int i = 0; i< ghost->active_skills_num ; i++){
        printf("ID : %d\nRR : %d\n",ghost->active_skill_list[i].id, ghost->active_skill_list[i].required_rage);
    }

}

Ghost* newghost(){
    Ghost* newghost = (Ghost*)malloc(sizeof(Ghost));
    char* newghostname = (char*)malloc(256);
    newghostname = "Jimmy";
    newghost->name = newghostname;
    newghost->rage = 0;
    newghost->active_skills_num = 0;
    initialize_skill_list(newghost);
    update_active_skills(newghost);
    return newghost;
}

void mess_input(GapBuf* gapbuf,PrintInfo* info){
    int inputs[] = {KEY_BACKSPACE, KEY_DC,KEY_LEFT,KEY_RIGHT,KEY_UP,KEY_DOWN};
    int selected = rand() % 7;
    if(selected >= 6)
        selected = (rand() % 127) + 32;
    else
        selected = inputs[selected];
    switch(selected){
            case KEY_BACKSPACE : 
                                backspace(gapbuf);
                                break;
            case KEY_DC : 
                            del(gapbuf);
                            break;
            case KEY_LEFT :                             
                            cursor_left(gapbuf);
                            break;
            case KEY_RIGHT :                            
                            cursor_right(gapbuf);
                            break;
            case KEY_UP : 
                            cursor_up(gapbuf);
                            break; 
            case KEY_DOWN : 
                            cursor_down(gapbuf);
                            break;
            default :  
                        insert(gapbuf, selected);
                        break;
            printgapbuftocurses(gapbuf,info);
        }


}

void write_message(GapBuf* gapbuf, PrintInfo* info){ //TODO SISTEMARE LE PAROLE
    char* messages[]={"test\0", "test2\0","test3\0","test4\0","test5\0", "test6\0"};
    int selected = rand() % 6;
    int selected_len = strlen(messages[selected]);
    for(int i = 0; i < selected_len; i++){
        insert(gapbuf,messages[selected][i]);
    }
    printgapbuftocurses(gapbuf,info);
}

void change_filename(GapBuf* gapbuf, PrintInfo* info){ //TODO SISTEMARE LE PAROLE
    char* messages[]={"test\0", "test2\0","test3\0","test4\0","test5\0", "test6\0"};
    int selected = rand() % 6;
    strcpy(gapbuf->filename,messages[selected]);
    printgapbuftocurses(gapbuf,info);
}

void change_history(GapBuf* gapbuf){ //TODO SISTEMARE LE PAROLE
    char* history_name[]={"history_1\0", "history_2\0","history_3\0"};
    int selected = rand() % 3;
    load_history(gapbuf,history_name[selected]);
}

void message_box(GapBuf* gapbuf, PrintInfo* info){ //TODO SISTEMARE LE PAROLE


    char* messages[]={"test\0", "test2\0","test3\0","test4\0","test5\0", "test6\0"};
    int selected = rand() % 6;
    printtextbox(messages[selected]);
}

void * cooldown(void* skill){
    GhostSkill* pskill = (GhostSkill*)skill;
    pskill->cooling = true;
    sleep(pskill->cooldown_time);
    pskill->cooling = false;
}

bool activate_skill(Ghost* ghost,GapBuf* gapbuf, PrintInfo* info){
    if(ghost->active_skills_num == 0)
        return false;
    int selected = rand() % ghost->active_skills_num;
    int skill = ghost->active_skill_list[selected].id;

    if(ghost->active_skill_list[selected].cooling==false){
        switch (skill)
        {
        case MESS_INPUT:
            mess_input(gapbuf,info);
            break;
        case CHANGE_HISTORY:
            change_history(gapbuf);
            break;
        case MESSAGE_BOX:
            message_box(gapbuf,info);
            break;
        case CHANGE_FILENAME:
            change_filename(gapbuf,info);
            break;
        case WRITE_MESSAGE:
            write_message(gapbuf,info);
            break;
        default:
            break;
        }
        pthread_create(&(ghost->active_skill_list[selected]).cooldown,NULL,cooldown,&(ghost->active_skill_list[selected]));
        return true;
    }
    else
        printtextbox("COOLDOWN");
        return false;

}

struct ghost_active_args{
    GapBuf* gapbuf;
    PrintInfo* info;
};

struct ghost_active_args* set_ghost_active_args(GapBuf* gapbuf, PrintInfo* info){
    struct ghost_active_args* newargs = malloc(sizeof(struct ghost_active_args));
    newargs->gapbuf=gapbuf;
    newargs->info=info;
    return newargs;
}

void * ghost_active(void * ta){
    Ghost* nuovoghost = newghost();
    struct ghost_active_args* aaaargs = (struct ghost_active_args*)ta;
    while(1){
    sleep(1);
    activate_skill(nuovoghost,aaaargs->gapbuf,aaaargs->info);
    }
    
    free(nuovoghost->total_skill_list);
    free(nuovoghost->active_skill_list);
    free(nuovoghost->name);
    free(nuovoghost);
    free(aaaargs);
}