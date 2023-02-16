#include "../hdr/server_help.h"
#include "../hdr/socket_msg_help.h"
#include "../hdr/byte_help.h"
#include "../hdr/conn_chan.h"
#include "../hdr/file_handle.h"
#include "../hdr/section.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

void find_existing_channel(connection_t*** conns, uint64 conn_index, uint64 sendID, uint64 recvID, channel_t** rchan, uint64* rindex){
    for(uint64 i=0; i<(*conns)[conn_index]->numChans; i++){
        if((*conns)[conn_index]->chans[i]->selfID == recvID){
            *rindex = i;
            *rchan = (*conns)[conn_index]->chans[i];
            return;
        }
    }
    for(uint64 i=0; i<(*conns)[conn_index]->numChans; i++){
        if((*conns)[conn_index]->chans[i]->otherID == sendID){
            *rindex = i;
            *rchan = (*conns)[conn_index]->chans[i];
            return;
        }
    }
    *rindex = 0;
    *rchan = NULL;
}

void* fork_server_channel(void* ptr){
    if(!ptr){ pthread_exit(0); }
    channel_t* chan = (channel_t *)ptr;
    printf("* Forked channel %lu for ", chan->selfID);
    print_string(chan->handle->fileName, chan->handle->fileNameLength);
    printf(".\n");

    uchar8* bytes = NULL;
    uint64 temp_int = 0;
    msg_t* tempmsg = NULL;
    bool8 finished_ft = false;

    section_t* send_sec = create_section();
    add_to_section(send_sec, 0);
    section_t* retrans_sec = create_section();

    while(!chan->ready && !chan->kill){ sleep(0); }
    if(chan->ready){ printf("* Channel %lu is Ready and Active\n", chan->selfID); }

    while(!chan->kill){
        //relay messages to thread
        if(chan->data){
            add_to_section(retrans_sec, chan->value->head->segmentNum);
            free_msg(chan->value);
            chan->data = false;
            printf("* Channel %lu received retransmission.\n", chan->selfID);
        }
        //current progress and what to do
        //flip
        if(finished_ft){
            if(get_piece_in_section(retrans_sec, 0) != NULL){
                bytes = segment_in_file_handle(chan->handle, first_low_in_section(retrans_sec), chan->maxSize, &temp_int);
                tempmsg = form_msg(chan->selfID, chan->otherID, DATA, SYN, first_low_in_section(retrans_sec), bytes, (uint32)temp_int);
                send_msg(chan->sock, tempmsg);
                free_msg(tempmsg);
                delete_from_section(retrans_sec, first_low_in_section(retrans_sec));
                free(bytes);
            }
        }else{
            if(last_continual_high_in_section(send_sec) < num_segments_in_file_handle(chan->handle, chan->maxSize) + 1){
                bytes = segment_in_file_handle(chan->handle, last_continual_high_in_section(send_sec)+1, chan->maxSize, &temp_int);
                tempmsg = form_msg(chan->selfID, chan->otherID, DATA, SYN, last_continual_high_in_section(send_sec)+1, bytes, (uint32)temp_int);
                send_msg(chan->sock, tempmsg);
                free_msg(tempmsg);
                add_to_section(send_sec, last_continual_high_in_section(send_sec)+1);
                free(bytes);
            }else{
                finished_ft = true;
            }
        }
    }
    printf("* Channel %lu terminated, converging.\n", chan->selfID);
    if(chan->data){ free_msg(chan->value); }
    free_section(send_sec);
    free_section(retrans_sec);
    pthread_exit(0);
    return NULL;
}