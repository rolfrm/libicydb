//requires:
// <stdint.h>

//Undefined behaviour will be undefined.

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
//typedef int128_t i128;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
//typedef uint128_t u128;
typedef struct{
  u64 a,b;
}u128;

typedef struct{
  u128 a, b;
}u256;

/*typedef struct{
  i64 a,b;
  }i128;*/
typedef char * str;

typedef float f32;
typedef double f64;


