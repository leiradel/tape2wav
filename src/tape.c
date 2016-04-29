#include <tape.h>
#include <tzx.h>
#include <t81.h>
#include <string.h>

typedef struct
{
  int     (*init)( void );
  int     (*identify)( const void* data, size_t size );
  tape_t* (*open)( const void* data, size_t size );
}
functions_t;

static functions_t tapes[] =
{
  { tzx_init, tzx_identify, tzx_open },
  { t81_init, t81_identify, t81_open },
};

int tape_init( void )
{
  int i, res = 1;
  
  for ( i = 0; i < sizeof( tapes ) / sizeof( tapes[ 0 ] ); i++ )
  {
    res &= tapes[ i ].init();
  }
  
  return res;
}


tape_t* tape_open( const void* data, size_t size )
{
  int i;
  
  for ( i = 0; i < sizeof( tapes ) / sizeof( tapes[ 0 ] ); i++ )
  {
    if ( tapes[ i ].identify( data, size ) )
    {
      return tapes[ i ].open( data, size );
    }
  }
  
  return NULL;
}

void tape_close( tape_t* tape )
{
  tape->close( tape );
}

size_t tape_play( tape_t* tape, unsigned sample_rate, uint8_t* pcm_u8, size_t size )
{
  return tape->play( tape, sample_rate, pcm_u8, size );
}

int tape_is_playing( tape_t* tape )
{
  return tape->is_playing( tape );
}

void tape_rewind( tape_t* tape )
{
  return tape->rewind( tape );
}
