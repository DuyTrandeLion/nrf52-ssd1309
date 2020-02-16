#ifndef _MISCELLANEOUS_H_
#define _MISCELLANEOUS_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#define SIZE(value)                     (sizeof(value) / sizeof(value[0]))

#define ABSOLUTE(value)                 ((value) >= 0.0)? (value) : ((-1.0) * (value)) 

//#define MAX(value1, value2)             ((value1) >= (value2))? (value1) : (value2)

//#define MIN(value1, value2)             ((value1) < (value2))? (value1) : (value2)

#define CLEAR_BUFFER(buffer)		(memset(buffer, 0, SIZE(buffer)))

#define MERGE_2BYTES(buffer, value)     ((value) = (((buffer[0]) << 8) & 0xFFFF));  \
                                        ((value) |= ((buffer[1]) & 0xFFFF))

#define MERGE_4BYTES(buffer, value)     ((value) = (((buffer[0]) << 24) & 0xFFFFFFFF));   \
                                        ((value) |= (((buffer[1]) << 16) & 0xFFFFFFFF));  \
                                        ((value) |= (((buffer[2]) << 8) & 0xFFFFFFFF));  \
                                        ((value) |= ((buffer[3]) & 0xFFFFFFFF))

#define ERROR_CHECK(_function_)         while (HAL_OK != _function_) { }

#endif /* _MISCELLANEOUS_H_ */