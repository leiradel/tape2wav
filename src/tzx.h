#ifndef TZX_H
#define TZX_H

#include <tape.h>
#include <stdint.h>
#include <stdlib.h>

int     tzx_init( void );
int     tzx_identify( const void* data, size_t size );
tape_t* tzx_open( const void* data, size_t size );

#endif /* TZX_H */
