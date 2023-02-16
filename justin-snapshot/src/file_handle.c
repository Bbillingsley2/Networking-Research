#include "../hdr/file_handle.h"
#include "../hdr/byte_help.h"
#include <stdlib.h>
#include <sys/dir.h>
#include <string.h>
#include <unistd.h>

uint64 ffsize(FILE* stream){
    fseek(stream, 0L, SEEK_END); // check fseek return
    return (uint64)ftell(stream); // check return, handle -1
}

FILE* ffopen(uchar8* path, uint64 plength){
    // documentation
    uchar8* opener = add_to_void(path, plength, 1);
    opener[plength] = '\0';
    FILE* fhandle = fopen((char8*)opener, "rb"); // check return
    free(opener);
    return fhandle;
}

void getFileNameFromPath(uint64 plength, uchar8* p, uint64* flength, uchar8** f){
    // make f a malloc, potential segment fault otherwise, basename(), dirname()
    *f = p;
    *flength = plength;
    for(uint64 i=plength; i>0; --i){
        if(p[i-1] == '/'){
            *f = &p[i];
            *flength = plength - i;
            return;
        }
    }
}

// create instead of open
file_handle_t* open_file_handle(uint64 plength, uchar8* p){
    file_handle_t* fhandle = malloc(sizeof(file_handle_t));
    fhandle->pointerStatus = false;
    pthread_mutex_init(&fhandle->mutex, NULL);
    fhandle->pathLength = plength;
    fhandle->path = copy_void(p, plength);
    getFileNameFromPath(fhandle->pathLength, fhandle->path, &fhandle->fileNameLength, &fhandle->fileName);
    fhandle->fileSize = 0;
    return fhandle;
}

void ready_file_handle(file_handle_t* fhandle){
    fhandle->pointer = ffopen(fhandle->path, fhandle->pathLength); // return
    fhandle->fileSize = ffsize(fhandle->pointer);
    fhandle->pointerStatus = true;
}

void print_file_handle(file_handle_t* fhandle){
    // file handle value
    printf("File Handle\n");
    printf("\tfilename: ");
    print_string(fhandle->fileName, fhandle->fileNameLength);
    printf(" | %lu\n", fhandle->fileNameLength);
    printf("\tfullpath: ");
    print_string(fhandle->path, fhandle->pathLength);
    printf(" | %lu\n", fhandle->pathLength);
    printf("\tsize: %lu\n", fhandle->fileSize); // unintalized
    printf("\tstatus: %s\n", fhandle->pointerStatus?"valid":"corrupted");
}

void close_file_handle(file_handle_t* fhandle){
    // non null fhandle
    if(fhandle->pointerStatus){ fclose(fhandle->pointer); }
    free(fhandle->path);
    free(fhandle);
}

DIR* ddopen(uchar8* path, uint64 plength){
    uchar8* opener = add_to_void(path, plength, 1);
    opener[plength] = '\0';
    DIR* dhandle = opendir((char8*)opener);
    free(opener);
    return dhandle;
}

dir_handle_t* open_dir_handle(uint64 dlength, uchar8* d){
    dir_handle_t* dhandle = malloc(sizeof(dir_handle_t));
    dhandle->dir = add_to_void(d, dlength, 1);
    dhandle->dir[dlength] = '/';
    dhandle->dirLength = dlength+1;
    dhandle->numHandles = 0;

    DIR* temp_dir;
    struct dirent *dirp;
    uint64 temp_int = 0;
    uchar8* fpath = NULL;

    temp_dir = ddopen(dhandle->dir, dhandle->dirLength);
    while((dirp = readdir(temp_dir)) != NULL){ if(dirp->d_type!=4){ dhandle->numHandles++; } }
    closedir(temp_dir);

    dhandle->handles = malloc(sizeof(file_handle_t*) * dhandle->numHandles);

    temp_dir = ddopen(dhandle->dir, dhandle->dirLength);
    while((dirp = readdir(temp_dir)) != NULL){
        if(dirp->d_type!=4){
            fpath = combine_voids(dhandle->dir, dhandle->dirLength, dirp->d_name, strlen(dirp->d_name));
            dhandle->handles[temp_int] = open_file_handle(dhandle->dirLength + strlen(dirp->d_name), fpath);
            free(fpath);
            temp_int++;
        }
    }
    closedir(temp_dir);

    for(uint64 i=0; i<dhandle->numHandles; i++){
        ready_file_handle(dhandle->handles[i]);
    }

    return dhandle;
}

void print_dir_handle(dir_handle_t* dhandle){
    printf("Directory '");
    print_string(dhandle->dir, dhandle->dirLength);
    printf("'\n");
    for(uint64 i=0; i<dhandle->numHandles; i++){
        printf("  - ");
        print_string(dhandle->handles[i]->fileName, dhandle->handles[i]->fileNameLength);
        printf(" | %lu\n", dhandle->handles[i]->fileSize);
    }
}

void close_dir_handle(dir_handle_t* dhandle){
    free(dhandle->dir);
    for(uint64 i=0; i<dhandle->numHandles; i++){
        close_file_handle(dhandle->handles[i]);
    }
    free(dhandle->handles);
    free(dhandle);
}

uint64 num_segments_in_file_handle(file_handle_t* fhandle, uint64 segmentSize){
    return ( (fhandle->fileSize%segmentSize)==0 ? (fhandle->fileSize/segmentSize) : ((fhandle->fileSize/segmentSize) + 1) );
}

file_handle_t* file_handle_for_file(dir_handle_t* dhandle, uchar8* file, uint64 fileLength){
    file_handle_t* res = NULL;
    for(uint64 i=0; i<dhandle->numHandles; i++){
        if(compare_bytes(file, fileLength, dhandle->handles[i]->fileName, dhandle->handles[i]->fileNameLength)){
            res = dhandle->handles[i];
            break;
        }
    }
    return res;
}

uchar8* segment_in_file_handle(file_handle_t* fhandle, uint64 segmentNum, uint64 segmentSize, uint64* numBytes){
    if(segmentNum == 0 || segmentNum > num_segments_in_file_handle(fhandle, segmentSize)){ *numBytes = 0; return NULL; }
    uint64 start = (segmentNum - 1) * segmentSize;
    *numBytes = (start + segmentSize > fhandle->fileSize) ? (fhandle->fileSize-start) : (segmentSize);

    uchar8* ret = malloc(sizeof(uchar8) * (*numBytes));
    pthread_mutex_lock(&fhandle->mutex);
    fseek(fhandle->pointer, (uint32)start, SEEK_SET);
    int res = (int)fread(ret, sizeof(uchar8), (*numBytes), fhandle->pointer);
    printf("%d", res);
    pthread_mutex_unlock(&fhandle->mutex);
    return ret;
}