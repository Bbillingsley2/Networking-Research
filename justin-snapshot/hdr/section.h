#ifndef SECTION_H
#define SECTION_H

#include "./stdtypes.h"

//Purpose: to hold a low and high bound in one struct
typedef struct{
  uint64 lowBound;
  uint64 highBound;
} piece_t;

//Purpose: hold many pieces in one struct, pieces are ordered
typedef struct{
  uint64 numPieces; // how many piece pointers there are
  piece_t** pieces; // an array of piece pointers
} section_t;

//Purpose: simply allocate and setup a section struct
//Parameters: none
//Returns: a usable section pointer
//Notes: none
extern section_t* create_section(void);

//Purpose: add a number to a section pointer
//Parameters: a section to add to and a number to add
//Returns: none
//Notes: none
extern void add_to_section(section_t* sec, uint64 num);

//Purpose: delete a number from a section pointer
//Parameters: a section to delete from and a number to delete
//Returns: none
//Notes: none
extern void delete_from_section(section_t* sec, uint64 num);

//Purpose: get a piece according to an index
//Parameters: a section to search and an index
//Returns: a piece pointer (could be NULL)
//Notes: none
extern piece_t* get_piece_in_section(section_t* sec, uint64 index);

//Purpose: get the last continual high in a section
//Parameters: a section to query from
//Returns: a int corresponding to the last continual high
//Notes: none
extern uint64 last_continual_high_in_section(section_t* sec);

//Purpose: get the first low number in a section
//Parameters: a section to query from
//Returns: a int corresponding to the first low
//Notes: returns 0 if the section is NULL or empty
extern uint64 first_low_in_section(section_t* sec);

//Purpose: print the details of a section
//Parameters: a section to get info from
//Returns: none
//Notes: none
extern void print_section(section_t* sec);

//Purpose: frees a section pointer (including its variables)
//Parameters: a section to free
//Returns: none
//Notes: none
extern void free_section(section_t* sec);

#endif//SECTION_H