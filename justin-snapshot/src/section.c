#include "../hdr/section.h"
#include "../hdr/byte_help.h"
#include <stdlib.h>
#include <stdio.h>

void add_to_section(section_t* sec, uint64 num){
    uint64 index = 0;
    piece_t** temp_array = NULL;
    piece_t* temp_piece = NULL;
    for(uint64 i=0; i<sec->numPieces; i++){
        if(sec->pieces[i]->highBound < num){
            index++;
        }
        if(sec->pieces[i]->lowBound <= num && sec->pieces[i]->highBound >= num){
            return;
        }else if(sec->pieces[i]->highBound+1 == num){
            sec->pieces[i]->highBound = num;
            // see if I can merge with the next
            if(i+1 < sec->numPieces){
                if(sec->pieces[i+1]->lowBound == sec->pieces[i]->highBound+1){
                    //merge this and next
                    sec->pieces[i+1]->lowBound = sec->pieces[i]->lowBound;
                    free(sec->pieces[i]);
                    temp_array = malloc(sizeof(piece_t*) * (sec->numPieces-1));
                    for(uint64 j=0; j<sec->numPieces; j++){
                        if(j < i){
                            temp_array[j] = sec->pieces[j];
                        }else if(i < j){
                            temp_array[j-1] = sec->pieces[j];
                        }else{ continue; }
                    }
                    free(sec->pieces);
                    sec->numPieces = sec->numPieces - 1;
                    sec->pieces = temp_array;
                }
            }
            return;
        }else if(sec->pieces[i]->lowBound!=0 && sec->pieces[i]->lowBound-1 == num){
            sec->pieces[i]->lowBound = num;
            if(i > 0){
                if(sec->pieces[i-1]->highBound == sec->pieces[i]->lowBound-1){
                    //merge last and this
                    sec->pieces[i-1]->highBound = sec->pieces[i]->highBound;
                    free(sec->pieces[i]);
                    temp_array = malloc(sizeof(piece_t*) * (sec->numPieces-1));
                    for(uint64 j=0; j<sec->numPieces; j++){
                        if(j < i){
                            temp_array[j] = sec->pieces[j];
                        }else if(i < j){
                            temp_array[j-1] = sec->pieces[j];
                        }else{ continue; }
                    }
                    free(sec->pieces);
                    sec->numPieces = sec->numPieces - 1;
                    sec->pieces = temp_array;
                }
            }
            return;
        }
    }
    // the piece needs to be added
    temp_piece = malloc(sizeof(piece_t));
    temp_piece->lowBound = num;
    temp_piece->highBound = num;
    if(sec->numPieces == 0){
        sec->numPieces = sec->numPieces + 1;
        sec->pieces = malloc(sizeof(piece_t*) * sec->numPieces);
        sec->pieces[0] = temp_piece;
    }else{
        temp_array = malloc(sizeof(piece_t*) * (sec->numPieces+1));
        for(uint64 j=0; j<sec->numPieces+1; j++){
            if(j < index){
                temp_array[j] = sec->pieces[j];
            }else if(index < j){
                temp_array[j] = sec->pieces[j-1];
            }else{
                temp_array[j] = temp_piece;
            }
        }
        free(sec->pieces);
        sec->numPieces = sec->numPieces + 1;
        sec->pieces = temp_array;
        temp_piece = NULL;
    }
}
void delete_from_section(section_t* sec, uint64 num){
    // here for completeness, no used
    piece_t* temp = NULL;
    piece_t** temp_array = NULL;
    for(uint64 i=0; i<sec->numPieces; i++){
        if(sec->pieces[i]->lowBound <= num && sec->pieces[i]->highBound >= num){
            if(sec->pieces[i]->lowBound < num && sec->pieces[i]->highBound > num){
                // split the piece into two
                temp = malloc(sizeof(piece_t));
                temp->lowBound = num + 1;
                temp->highBound = sec->pieces[i]->highBound;
                sec->pieces[i]->highBound = num - 1;
                temp_array = malloc(sizeof(piece_t*) * (sec->numPieces+1));
                for(uint64 j=0; j<sec->numPieces+1; j++){
                    if(j < i+1){
                        temp_array[j] = sec->pieces[j];
                    }else if(i+1 < j){
                        temp_array[j] = sec->pieces[j-1];
                    }else{ temp_array[j] = temp; continue; }
                }
                free(sec->pieces);
                sec->numPieces = sec->numPieces + 1;
                sec->pieces = temp_array;
            }else if(sec->pieces[i]->lowBound == num && sec->pieces[i]->highBound > num){
                // decrease the lower bound
                sec->pieces[i]->lowBound = num + 1;
            }else if(sec->pieces[i]->lowBound < num && sec->pieces[i]->highBound == num){
                // decrease the higher bound
                sec->pieces[i]->highBound = num - 1;
            }else{
                // it equals lower bound and higher bound
                free(sec->pieces[i]);
                if(sec->numPieces-1 > 0){
                    temp_array = malloc(sizeof(piece_t*) * (sec->numPieces-1));
                    for(uint64 j=0; j<sec->numPieces; j++){
                        if(j < i){
                            temp_array[j] = sec->pieces[j];
                        }else if(i < j){
                            temp_array[j-1] = sec->pieces[j];
                        }else{ continue; }
                        sec->pieces[i] = NULL;
                    }
                    free(sec->pieces);
                    sec->numPieces = sec->numPieces - 1;
                    sec->pieces = temp_array;
                }else{
                    sec->numPieces = sec->numPieces - 1;
                    free(sec->pieces);
                }
            }
            return;
        }
    }
}
uint64 last_continual_high_in_section(section_t* sec){
    if(sec->numPieces==0){ return 0; }
    return sec->pieces[0]->highBound;
}
uint64 first_low_in_section(section_t* sec){
    if(sec->numPieces==0){ return 0; }
    return sec->pieces[0]->lowBound;
}
piece_t* get_piece_in_section(section_t* sec, uint64 index){
    if(sec->numPieces <= index){ return NULL; }
    return sec->pieces[index];
}
void print_section(section_t* sec){
    printf("Section\n");
    printf("-------\n");
    printf("# pieces: %lu\n", sec->numPieces);
    printf("pieces:\n");
    if(sec->numPieces > 0){
        for(uint64 i=0; i<sec->numPieces; i++){
            printf("   %lu| l:%lu,h:%lu\n", i+1, sec->pieces[i]->lowBound, sec->pieces[i]->highBound);
        }
    }else{ printf("   NULL\n"); }
}
void free_section(section_t* sec){
    for(uint64 i=0; i<sec->numPieces; i++){
        free(sec->pieces[i]);
    }
    free(sec->pieces);
    free(sec);
}
section_t* create_section(void){
    section_t* sec = malloc(sizeof(section_t));
    sec->numPieces = 0;
    sec->pieces = NULL;
    return sec;
}