//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_ENGINE_H
#define SCRATCH_FOP_ENGINE_H
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include "structs.h"
using namespace std;
void hazf_block(Block * block) {
    if (!block) return;
    switch (block->type) {
        case BLOCK_EVENT: cout<<"[EVENT] Script Started: "<<block->text<<endl;
            break;
            case BLOCK_MOTION: cout<<"[MOTION] Moving Object: "<<block->text<<endl;
            break;
        case BLOCK_LOOKS: cout<<"[LOOKS] Saying: "<<block->text<<endl;
            break;
            default: cout<<"[UNKNOWN] Block ID: "<<block->id<<endl;
    }
}
void run_script(Block * startBlock, Sprite * sprite) {
    Block * current =startBlock;
    while (current != nullptr) {
        switch (current->type) {
            case BLOCK_MOTION:sprite->x+=10;
                break;
            case BLOCK_LOOKS:
                cout<<"[LOOKS] "<<current->text<<endl;
                break;
            case BLOCK_EVENT:
                cout<<"[EVENT] "<<current->text<<endl;
                break;

        }
        SDL_Delay(100);
        current = current->next;
    }
}
void check_run_click( int mx, int my, const vector <Block*> & blocks) {
    for (Block*b:blocks) {
        if (b->type==BLOCK_EVENT && b->text=="When Flag Clicked" ) {
            cout<<"---Execution Started---\n";
            // run_script(b, &sprite);
            cout<<"---Execution Ended---\n";
        }
    }
}
#endif //SCRATCH_FOP_ENGINE_H