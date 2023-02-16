#ifndef FILE_HANDLE_H
#define FILE_HANDLE_H

#include <stdio.h>
#include <pthread.h>
#include "./stdtypes.h"

typedef struct{
  FILE* pointer;
  bool8 pointerStatus;
  uint64 pathLength;
  uchar8* path;
  uint64 fileNameLength;
  uchar8* fileName;
  uint64 fileSize;
  pthread_mutex_t mutex;
} file_handle_t;

typedef struct{
  uint64 numHandles;
  file_handle_t** handles;
  uint64 dirLength;
  uchar8* dir;
} dir_handle_t;

extern file_handle_t* open_file_handle(uint64 pathLength, uchar8* path);
extern void ready_file_handle(file_handle_t* fhandle);
extern void print_file_handle(file_handle_t* fhandle);
extern void close_file_handle(file_handle_t* fhandle);

extern dir_handle_t* open_dir_handle(uint64 dirLength, uchar8* dir);
extern void print_dir_handle(dir_handle_t* dhandle);
extern void close_dir_handle(dir_handle_t* dhandle);

extern uint64 num_segments_in_file_handle(file_handle_t* fhandle, uint64 segmentSize);
extern file_handle_t* file_handle_for_file(dir_handle_t* dhandle, uchar8* file, uint64 fileLength);
extern uchar8* segment_in_file_handle(file_handle_t* fhandle, uint64 segmentNum, uint64 segmentSize, uint64* numBytes);

#endif//FILE_HANDLE_H