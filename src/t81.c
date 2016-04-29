#include <tape.h>
#include <t81.h>
#include <coro.h>
#include <string.h>

enum
{
  OPTZEROBEFOREONE = 29,
  OPTZEROBEFORENUL = 29,
  OPTEXTRAONE      = 76,
  OPTEXTRANUL      = 76,
  OPTLEADIN        = 22,
  OPTLEADOUT       = 22,
};

typedef struct
{
  char    name[ 32 ];
  char    size[ 16 ];
  uint8_t data[ 1 ];
}
block_t;

typedef struct
{
  tape_t methods;
  coro_t coro;
  
  const uint8_t* t81;
  
  const block_t* block;
  const block_t* end;
  size_t size;
  int borrow0, borrow1;
  int is_playing;
}
state_t;

static const char signature[] = { 'E', 'O', '8', '1' };

static const uint8_t ascii2zx81[] = {
  0x00, 0x16, 0x0b, 0x16, 0x0d, 0x16, 0x16, 0x16, 0x10, 0x11, 0x17, 0x15, 0x1a, 0x16, 0x1b, 0x18,
  0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x0e, 0x19, 0x13, 0x14, 0x12, 0x0f,
  0x16, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,
  0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x16, 0x16, 0x16, 0x16, 0x16,
  0x16, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34,
  0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x16, 0x16, 0x16, 0x16, 0x16,
};

static uint8_t sine[ 256 ] =
{
  0x80, 0x83, 0x86, 0x89, 0x8c, 0x8f, 0x92, 0x95, 0x98, 0x9c, 0x9f, 0xa2, 0xa5, 0xa8, 0xab, 0xae,
  0xb0, 0xb3, 0xb6, 0xb9, 0xbc, 0xbf, 0xc1, 0xc4, 0xc7, 0xc9, 0xcc, 0xce, 0xd1, 0xd3, 0xd5, 0xd8,
  0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xed, 0xef, 0xf0, 0xf2, 0xf3, 0xf5,
  0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfc, 0xfd, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfd, 0xfc, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7,
  0xf6, 0xf5, 0xf3, 0xf2, 0xf0, 0xef, 0xed, 0xec, 0xea, 0xe8, 0xe6, 0xe4, 0xe2, 0xe0, 0xde, 0xdc,
  0xda, 0xd8, 0xd5, 0xd3, 0xd1, 0xce, 0xcc, 0xc9, 0xc7, 0xc4, 0xc1, 0xbf, 0xbc, 0xb9, 0xb6, 0xb3,
  0xb0, 0xae, 0xab, 0xa8, 0xa5, 0xa2, 0x9f, 0x9c, 0x98, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x83,
  0x80, 0x7c, 0x79, 0x76, 0x73, 0x70, 0x6d, 0x6a, 0x67, 0x63, 0x60, 0x5d, 0x5a, 0x57, 0x54, 0x51,
  0x4f, 0x4c, 0x49, 0x46, 0x43, 0x40, 0x3e, 0x3b, 0x38, 0x36, 0x33, 0x31, 0x2e, 0x2c, 0x2a, 0x27,
  0x25, 0x23, 0x21, 0x1f, 0x1d, 0x1b, 0x19, 0x17, 0x15, 0x13, 0x12, 0x10, 0x0f, 0x0d, 0x0c, 0x0a,
  0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x09, 0x0a, 0x0c, 0x0d, 0x0f, 0x10, 0x12, 0x13, 0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x23,
  0x25, 0x27, 0x2a, 0x2c, 0x2e, 0x31, 0x33, 0x36, 0x38, 0x3b, 0x3e, 0x40, 0x43, 0x46, 0x49, 0x4c,
  0x4f, 0x51, 0x54, 0x57, 0x5a, 0x5d, 0x60, 0x63, 0x67, 0x6a, 0x6d, 0x70, 0x73, 0x76, 0x79, 0x7c
};

static void   t81_close( tape_t* tape );
static size_t t81_play( tape_t* tape, unsigned sample_rate, uint8_t* pcm_u8, size_t buffer_size );
static int    t81_is_playing( tape_t* tape );
static void   t81_rewind( tape_t* tape );

int t81_init( void )
{
  return 1;
}

int t81_identify( const void* data, size_t size )
{
  return size >= sizeof( signature ) && !memcmp( data, (const void*)signature, sizeof( signature ) );
}

tape_t* t81_open( const void* data, size_t size )
{
  state_t* state = (state_t*)malloc( sizeof( state_t ) );
  
  if ( state == NULL )
  {
    return NULL;
  }
  
  state->t81 = (const uint8_t*)malloc( size );
  
  if ( state->t81 == NULL )
  {
    free( state );
    return NULL;
  }
  
  state->methods.close = t81_close;
  state->methods.play = t81_play;
  state->methods.is_playing = t81_is_playing;
  state->methods.rewind = t81_rewind;
  
  memcpy( (void*)state->t81, data, size );
  state->end = (const block_t*)( state->t81 + size );
  
  t81_rewind( (tape_t*)state );
  
  return (tape_t*)state;
}

static void t81_close( tape_t* tape )
{
  state_t* state = (state_t*)tape;
  free( (void*)state->t81 );
  free( state );
}

static size_t t81_play( tape_t* tape, unsigned sample_rate, uint8_t* pcm_u8, size_t buffer_size )
{
  enum
  {
    ZEROES = -1,
    BYTE   = -2,
    ZERO   = -3,
    ONE    = -4,
    OUTPUT = -5,
    END    = -6,
  };

  state_t* state = (state_t*)tape;
  coro_t* coro = &state->coro;
  uint8_t* begin = pcm_u8;
  
  CORO_ENTER()
    state->is_playing = 1;
    
    while ( state->block < state->end )
    {
      state->size = atol( state->block->size );
      
      if ( !strcmp( state->block->name, "<Silence>" ) )
      {
        /* Silence */
        CORO_GOSUB( ZEROES, state->size * sample_rate / 1000, 0 );
        state->block = (block_t*)( (uint8_t*)state->block + 48 );
        continue;
      }
      
      /* Lead in */
      CORO_GOSUB( ZEROES, OPTLEADIN * 1000ULL * sample_rate / 22050ULL, 0 );
      
      /* File name */
      CORO_J = state->block->data[ 0 ];
      
      if ( CORO_J == 0 || CORO_J == 255 || CORO_J == 1 )
      {
        /* Use the block name */
        for ( CORO_I = 0; ( CORO_J = state->block->name[ CORO_I ] ) != 0; CORO_I++ )
        {
          CORO_J = CORO_J >= 32 && CORO_J <= 127 ? ascii2zx81[ CORO_J - 32 ] : 0x16;
          CORO_GOSUB( BYTE, CORO_J, 0 );
        }
      }
      
      /* Data */
      for ( CORO_I = 0; CORO_I < state->size; CORO_I++ )
      {
        CORO_GOSUB( BYTE, state->block->data[ CORO_I ], 0 );
      }
      
      /* Lead out */
      CORO_GOSUB( ZEROES, OPTLEADOUT * 1000ULL * sample_rate / 22050ULL, 0 );
      
      state->block = (block_t*)( (uint8_t*)state->block + 48 + state->size );
    }
    
    state->is_playing = 0;
    CORO_GOTO( END );
    
  CORO_LABEL( ZEROES )
    for ( CORO_I = 0; CORO_I < CORO_A; CORO_I++ )
    {
      CORO_GOSUB( OUTPUT, 128, 0 );
    }

    CORO_RET();
    
  CORO_LABEL( BYTE )
    for ( CORO_I = 128; CORO_I != 0; CORO_I >>= 1 )
    {
      if ( ( CORO_A & CORO_I ) == 0 )
      {
        CORO_GOSUB( ZERO, 0, 0 );
      }
      else
      {
        CORO_GOSUB( ONE, 0, 0 );
      }
    }
    
    CORO_RET();
    
  CORO_LABEL( ZERO )
    CORO_GOSUB( ZEROES, OPTZEROBEFORENUL * sample_rate / 22050, 0 );
    CORO_J = 26 * sample_rate / 22050;
    
    for ( CORO_I = 0; CORO_I < CORO_J; CORO_I++ )
    {
      CORO_K = CORO_I * 4 * 256 / CORO_J;
      CORO_GOSUB( OUTPUT, sine[ CORO_K & 255 ], 0 );
    }
    
 		if ( OPTEXTRANUL != 0 )
		{
			CORO_I = state->borrow0 / 100;
			state->borrow0 += OPTEXTRANUL * sample_rate / 22050;

			if ( ( state->borrow0 / 100 ) > CORO_I )
			{
				state->borrow0 -= CORO_I * 100;
        CORO_GOSUB( OUTPUT, 128, 0 );
			}
		}
    
    CORO_RET();
    
  CORO_LABEL( ONE )
    CORO_GOSUB( ZEROES, OPTZEROBEFOREONE * sample_rate / 22050, 0 );
    CORO_J = 58 * sample_rate / 22050;
    
		for ( CORO_I = 0; CORO_I < CORO_J; CORO_I++ )
    {
      CORO_K = CORO_I * 9 * 256 / CORO_J;
      CORO_GOSUB( OUTPUT, sine[ CORO_K & 255 ], 0 );
    }

 		if ( OPTEXTRAONE != 0 )
		{
			CORO_I = state->borrow1 / 100;
			state->borrow1 += OPTEXTRAONE * sample_rate / 22050;

			if ( ( state->borrow1 / 100 ) > CORO_I )
			{
				state->borrow1 -= CORO_I * 100;
        CORO_GOSUB( OUTPUT, 128, 0 );
			}
		}
    
    CORO_RET();
    
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

static int t81_is_playing( tape_t* tape )
{
  state_t* state = (state_t*)tape;
  return state->is_playing;
}

static void t81_rewind( tape_t* tape )
{
  state_t* state = (state_t*)tape;
  
  CORO_SETUP( &state->coro );
  
  state->block = (const block_t*)( state->t81 + sizeof( signature ) );
  state->borrow0 = state->borrow1 = 0;
  state->is_playing = 0;
}
