/*

Based on tape2wav.c: Convert tape files (tzx, tap, etc.) to .wav files
Copyright (c) 2007 Fredrick Meunier

*/

#include <tzx.h>
#include <libspectrum.h>
#include <coro.h>

/* ZX Spectrum clock */
#define MHZ 3500000ULL

typedef struct
{
  tape_t methods;
  coro_t coro;
  
  libspectrum_tape* tzx;
  short level; /* The last level output to this block */
  libspectrum_dword pulse_tstates;
  uint64_t balance_tstates;
  uint64_t tape_length;
  uint64_t pulse_length;
  int flags, step, is_playing;
}
state_t;

static void   tzx_close( tape_t* tape );
static size_t tzx_play( tape_t* tape, unsigned sample_rate, uint8_t* pcm_u8, size_t buffer_size );
static int    tzx_is_playing( tape_t* tape );
static void   tzx_rewind( tape_t* tape );

int tzx_init( void )
{
  /* The minimum version of libspectrum we need */
  static const char *LIBSPECTRUM_MIN_VERSION = "0.2.0";
  
  if ( !libspectrum_check_version( LIBSPECTRUM_MIN_VERSION ) )
  {
    return 0;
  }

  if ( libspectrum_init() != LIBSPECTRUM_ERROR_NONE )
  {
    return 0;
  }
  
  return 1;
}

int tzx_identify( const void* data, size_t size )
{
  static const char signature[] = { 'Z', 'X', 'T', 'a', 'p', 'e', '!', 0x1a };
  return size >= sizeof( signature ) && !memcmp( data, (const void*)signature, sizeof( signature ) );
}

tape_t* tzx_open( const void* data, size_t size )
{
  state_t* state = (state_t*)malloc( sizeof( state_t ) );
  
  if ( state == NULL )
  {
    return NULL;
  }
  
  state->tzx = libspectrum_tape_alloc();
  
  if ( state->tzx == NULL )
  {
    free( state );
    return NULL;
  }

  if ( libspectrum_tape_read( state->tzx, data, size, LIBSPECTRUM_ID_UNKNOWN, NULL ) != LIBSPECTRUM_ERROR_NONE )
  {
    free( state );
    return NULL;
  }
  
  state->methods.close = tzx_close;
  state->methods.play = tzx_play;
  state->methods.is_playing = tzx_is_playing;
  state->methods.rewind = tzx_rewind;
  
  tzx_rewind( (tape_t*)state );
  
  return (tape_t*)state;
}

static void tzx_close( tape_t* tape )
{
  state_t* state = (state_t*)tape;
  libspectrum_tape_free( state->tzx );
  free( state );
}

static size_t tzx_play( tape_t* tape, unsigned sample_rate, uint8_t* pcm_u8, size_t buffer_size )
{
  enum
  {
    OUTPUT = -1,
    END    = -2,
  };

  state_t* state = (state_t*)tape;
  coro_t* coro = &state->coro;
  uint8_t* begin = pcm_u8;
  libspectrum_error error;
  
  CORO_ENTER()
    state->is_playing = 1;
    
    while ( !( state->flags & LIBSPECTRUM_TAPE_FLAGS_TAPE ) )
    {
      error = libspectrum_tape_get_next_edge( &state->pulse_tstates, &state->flags, state->tzx );

      if ( error != LIBSPECTRUM_ERROR_NONE )
      {
        CORO_GOTO( END );
      }

      /* Invert the microphone state */
      if ( state->pulse_tstates ||
        ( state->flags & (LIBSPECTRUM_TAPE_FLAGS_STOP |
          LIBSPECTRUM_TAPE_FLAGS_LEVEL_LOW |
          LIBSPECTRUM_TAPE_FLAGS_LEVEL_HIGH ) ) )
      {
        if ( state->flags & LIBSPECTRUM_TAPE_FLAGS_NO_EDGE)
        {
          /* Do nothing */
        }
        else if ( state->flags & LIBSPECTRUM_TAPE_FLAGS_LEVEL_LOW )
        {
          state->level = 0;
        }
        else if ( state->flags & LIBSPECTRUM_TAPE_FLAGS_LEVEL_HIGH )
        {
          state->level = 1;
        }
        else
        {
          state->level = !state->level;
        }
      }

      state->balance_tstates += state->pulse_tstates;

      if ( state->flags & LIBSPECTRUM_TAPE_FLAGS_NO_EDGE )
      {
        continue;
      }

      state->pulse_length = state->balance_tstates * sample_rate / MHZ;
      state->balance_tstates -= state->pulse_length * MHZ / sample_rate;

      /* TZXs produced by snap2tzx have very tight tolerances, err on the side of
      producing a pulse that is too long rather than too short */
      /* HACK this makes the ZX81 wave end up bigger than it should */
      if ( state->balance_tstates > ( MHZ / sample_rate ) >> 1 )
      {
        state->pulse_length++;
        state->balance_tstates = 0;
      }
      
      CORO_J = state->level ? 0xff : 0x00;
      
      for ( CORO_I = 0; CORO_I < state->pulse_length; CORO_I++ )
      {
        CORO_GOSUB( OUTPUT, CORO_J, 0 );
      }
    }
    
    state->is_playing = 0;
    CORO_GOTO( END );
    
  CORO_LABEL( OUTPUT )
    if ( buffer_size == 0 )
    {
      CORO_YIELD( pcm_u8 - begin );
    }
    
    *pcm_u8++ = CORO_A;
    buffer_size--;
    CORO_RET();
    
  CORO_LABEL( END )
    CORO_YIELD( pcm_u8 - begin );
  
  CORO_LEAVE( 0 );
}

static int tzx_is_playing( tape_t* tape )
{
  state_t* state = (state_t*)tape;
  return state->is_playing;
}

static void tzx_rewind( tape_t* tape )
{
  state_t* state = (state_t*)tape;
  
  CORO_SETUP( &state->coro );
  
  state->level = 0;
  state->pulse_tstates = 0;
  state->balance_tstates = 0;
  state->tape_length = 0;
  state->pulse_length = 0;
  state->flags = 0;
  state->is_playing = 0;
  
  libspectrum_tape_nth_block( state->tzx, 0 );
}
