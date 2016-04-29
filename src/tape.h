#ifndef TAPE_H
#define TAPE_H

#include <stdint.h>
#include <stdlib.h>

typedef struct tape_t tape_t;

struct tape_t
{
  void   (*close)( tape_t* tape );
  size_t (*play)( tape_t* tape, unsigned sample_rate, uint8_t* pcm_buffer, size_t buffer_size );
  int    (*is_playing)( tape_t* tape );
  void   (*rewind)( tape_t* tape );
};

int tape_init( void );

tape_t* tape_open( const void* data, size_t size );
void    tape_close( tape_t* tape );

size_t tape_play( tape_t* tape, unsigned sample_rate, uint8_t* pcm_u8, size_t size );
int    tape_is_playing( tape_t* tape );
void   tape_rewind( tape_t* tape );

#endif /* TAPE_H */
