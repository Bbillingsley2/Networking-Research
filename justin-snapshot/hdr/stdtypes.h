#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// ---------------------------
//          Types
// ---------------------------
// 8 bits == 1 byte
// use sizeof() to check the size of any data type

// ints
typedef int8_t          int8;      // signed    - 8   bits | 1   bytes | -128 <-> 127
typedef uint8_t        uint8;      // unsigned  - 8   bits | 1   bytes | 0 <-> 255
typedef int16_t         int16;     // signed    - 16  bits | 2   bytes | -32768 <-> 32767
typedef uint16_t       uint16;     // unsigned  - 16  bits | 2   bytes | 0 <-> 65535
typedef int32_t         int32;     // signed    - 32  bits | 4   bytes | -2,147,483,648 <-> 2,147,483,647
typedef uint32_t       uint32;     // unsigned  - 32  bits | 4   bytes | 0 <-> 4,294,967,295
typedef int64_t         int64;     // signed    - 64  bits | 8   bytes | -9,223,372,036,854,775,808 <-> 9,223,372,036,854,775,807
typedef uint64_t       uint64;     // unsigned  - 64  bits | 8   bytes | 0 <-> 18,446,744,073,709,551,615
typedef intmax_t        int0;      // signed    - max bits | max bytes | same as int64
typedef uintmax_t      uint0;      // unsigned  - max bits | max bytes | same as uint64

// bools
typedef bool            bool8;      // true / false - 8 bits | 1 bytes | true or false

//floats
typedef float           float32;    // 32  bits | 4  bytes
typedef double          float64;    // 64  bits | 8  bytes
typedef long double     float128;   // 128 bits | 16 bytes

//chars
typedef char            char8;  // signed             - 8 bits | 1  byte | -128 <-> 127
typedef unsigned char  uchar8;  // unsigned           - 8 bits | 1  byte | 0 <-> 255

//randoms

//Purpose: generate a random unsigned 64bit int within bounds
//Parameters: takes a uint64 for min and max
//Returns: a uint64
//Notes: typecast the input from what you have and output to whatever is needed
extern uint64 generate_random_uint(uint64 min, uint64 max);

#endif//STD_TYPES_H