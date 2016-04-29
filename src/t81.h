#ifndef T81_H
#define T81_H

#include <stdint.h>
#include <stdlib.h>

int     t81_init( void );
int     t81_identify( const void* data, size_t size );
tape_t* t81_open( const void* data, size_t size );

#endif /* T81_H */
