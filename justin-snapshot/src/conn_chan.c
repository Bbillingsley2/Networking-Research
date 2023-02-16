#include "../hdr/conn_chan.h"
#include "../hdr/byte_help.h"
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

connection_t* new_connection(connection_t*** conns, uint64* totalConnections){
    *totalConnections = *totalConnections + 1;
    if(*totalConnections == 1){
        *conns = malloc(sizeof(connection_t*) * (*totalConnections));
        (*conns)[0] = malloc(sizeof(connection_t));
        (*conns)[0]->numChans = 0;
        (*conns)[0]->sock = 0;
        (*conns)[0]->chans = NULL;
        return (*conns)[0];
    }

    connection_t** tempconns = malloc(sizeof(connection_t*) * ((*totalConnections) - 1));
    for(uint64 i=0; i<((*totalConnections) - 1); i++){
        tempconns[i] = (*conns)[i];
        (*conns)[i] = NULL;
    }
    free(*conns);
    *conns = tempconns;

    (*conns)[(*totalConnections)-1] = malloc(sizeof(connection_t));
    (*conns)[(*totalConnections)-1]->numChans = 0;
    (*conns)[(*totalConnections)-1]->sock = 0;
    (*conns)[(*totalConnections)-1]->chans = NULL;
    return (*conns)[(*totalConnections)-1];
}
void end_connection(connection_t*** conns, uint64* totalConnections, uint64* totalChannels, uint64 index){
    while((*conns)[index]->numChans != 0){
        end_channel(conns, totalChannels, index, 0);
    }
    //close socket and clean
    close((*conns)[index]->sock);
    free((*conns)[index]);
    (*conns)[index] = NULL;
    *totalConnections = *totalConnections - 1;
    if(*totalConnections == 0){
        free(*conns);
    }else{
        connection_t** tempconnects = malloc(sizeof(connection_t*) * (*totalConnections));
        for(uint64 i=0; i<((*totalConnections)+1); i++){
            if(i < index){
                tempconnects[i] = (*conns)[i];
                (*conns)[i] = NULL;
            }else if(i > index){
                tempconnects[i-1] = (*conns)[i];
                (*conns)[i] = NULL;
            }
        }
        free(*conns);
        *conns = tempconnects;
    }
}
void print_connection(connection_t*** conns, uint64* totalConnections, uint64 index){
    printf("Connection %lu\n", index);
    if((*totalConnections)-1 < index){
        printf("  NULL | Does not Exist\n");
        return;
    }
    struct sockaddr_in* caddr = (struct sockaddr_in*)&((*conns)[index]->address);
    printf("  sock:%d\n", (*conns)[index]->sock);
    printf("  address:%s | port:%d\n", inet_ntoa(caddr->sin_addr), ntohs(caddr->sin_port));
    printf("  channels amount:%lu\n", (*conns)[index]->numChans);
    printf("  channels:\n");
    if((*conns)[index]->numChans > 0){
        for(uint64 i=0; i<(*conns)[index]->numChans; i++){
            printf("   * channel %lu\n", i);
            printf("     thread:%ld\n", (int64)(*conns)[index]->chans[i]->thread);
            printf("     selfID:%lu | otherID:%lu\n",  (*conns)[index]->chans[i]->selfID, (*conns)[index]->chans[i]->otherID);
            printf("     maxSize:%lu\n", (*conns)[index]->chans[i]->maxSize);
            printf("     file:");
            if((*conns)[index]->chans[i]->handle != NULL){
                print_string((*conns)[index]->chans[i]->handle->fileName, (*conns)[index]->chans[i]->handle->fileNameLength);
            }else{
                printf("none");
            }
            printf("\n");
        }
    }else{
        printf("   NULL\n");
    }
}
void print_all_connections(connection_t*** conns, uint64* totalConnections){
    printf("All Channels:\n");
    if(*totalConnections == 0){
        printf("NULL\n");
    }else{
        for(uint64 i=0; i<(*totalConnections); i++){
            print_connection(conns, totalConnections, i);
        }
    }
}
void end_all_connections(connection_t*** conns, uint64* totalConnections, uint64* totalChannels){
    while(*totalConnections != 0){
        end_connection(conns, totalConnections, totalChannels, 0);
    }
}

channel_t* new_channel(connection_t*** conns, uint64* totalChannels, uint64 conn_index){
    *totalChannels = *totalChannels + 1;
    (*conns)[conn_index]->numChans = (*conns)[conn_index]->numChans + 1;

    if((*conns)[conn_index]->numChans == 1){
        (*conns)[conn_index]->chans = malloc(sizeof(channel_t*) * ((*conns)[conn_index]->numChans));
        (*conns)[conn_index]->chans[0] = malloc(sizeof(channel_t));
        (*conns)[conn_index]->chans[0]->selfID = 0;
        (*conns)[conn_index]->chans[0]->otherID = 0;
        (*conns)[conn_index]->chans[0]->maxSize = 0;
        (*conns)[conn_index]->chans[0]->handle = NULL;
        (*conns)[conn_index]->chans[0]->data = false;
        (*conns)[conn_index]->chans[0]->kill = false;
        (*conns)[conn_index]->chans[0]->ready = false;
        (*conns)[conn_index]->chans[0]->value = NULL;
        (*conns)[conn_index]->chans[0]->thread = 0;
        return (*conns)[conn_index]->chans[0];
    }

    channel_t** tempchans = malloc(sizeof(channel_t*) * ((*conns)[conn_index]->numChans - 1));
    for(uint64 i=0; i<((*conns)[conn_index]->numChans - 1); i++){
        tempchans[i] = (*conns)[conn_index]->chans[i];
        (*conns)[conn_index]->chans[i] = NULL;
    }
    free((*conns)[conn_index]->chans);
    (*conns)[conn_index]->chans = tempchans;

    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1] = malloc(sizeof(channel_t));
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->selfID = 0;
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->otherID = 0;
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->maxSize = 0;
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->handle = NULL;
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->data = false;
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->kill = false;
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->ready = false;
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->value = NULL;
    (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1]->thread = 0;
    return (*conns)[conn_index]->chans[(*conns)[conn_index]->numChans - 1];
}
void end_channel(connection_t*** conns, uint64* totalChannels, uint64 conn_index, uint64 chan_index){
    *totalChannels = *totalChannels - 1;
    (*conns)[conn_index]->numChans = (*conns)[conn_index]->numChans - 1;
    (*conns)[conn_index]->chans[chan_index]->kill = true;
    pthread_join((*conns)[conn_index]->chans[chan_index]->thread,NULL);
    free((*conns)[conn_index]->chans[chan_index]);
    (*conns)[conn_index]->chans[chan_index] = NULL;

    if((*conns)[conn_index]->numChans==0){
        free((*conns)[conn_index]->chans);
    }else{
        channel_t** tempchans = malloc(sizeof(channel_t*) * ((*conns)[conn_index]->numChans));
        for(uint64 i=0; i<((*conns)[conn_index]->numChans+1); i++){
            if(i < chan_index){
                tempchans[i] = (*conns)[conn_index]->chans[i];
                (*conns)[conn_index]->chans[i] = NULL;
            }else if(i > chan_index){
                tempchans[i-1] = (*conns)[conn_index]->chans[i];
                (*conns)[conn_index]->chans[i] = NULL;
            }
        }
        free((*conns)[conn_index]->chans);
        (*conns)[conn_index]->chans = tempchans;
    }
}