#include <SDL2/SDL.h>
#include <tape.h>
#include <common.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef struct
{
  tape_t*       tape;
  SDL_AudioSpec have;
}
userdata_t;

static void usage( FILE* out )
{
  fprintf( out, "Usage: playtape options...\n\n" );
  fprintf( out, "-i filename        Input file name\n" );
  fprintf( out, "-h                 This page\n\n" );
}

static void generate( void* userdata, Uint8* stream, int len )
{
  userdata_t* ud = (userdata_t*)userdata;
  size_t size;
  
  size = tape_play( ud->tape, ud->have.freq, stream, len );
  
  if ( size < len )
  {
    memset( stream + size, 0, len - size );
  }
}

int main( int argc, char* argv[] )
{
  const char* input_name = NULL;

  int i;
  void* data;
  size_t size;
  SDL_AudioSpec want;
  SDL_AudioDeviceID devid;
  userdata_t ud;
  
  for ( i = 1; i < argc; i++ )
  {
    if ( !strcmp( argv[ i ], "-i" ) )
    {
      if ( ++i == argc )
      {
        fprintf( stderr, "Error: missing argument to -i\n" );
        return 1;
      }
      
      input_name = argv[ i ];
    }
    else if ( !strcmp( argv[ i ], "-h" ) )
    {
      usage( stdout );
      return 1;
    }
    else
    {
      fprintf( stderr, "Error: unknown option %s\n\n", argv[ i ] );
      usage( stderr );
      return 1;
    }
  }
  
  if ( input_name == NULL )
  {
    fprintf( stderr, "Error: input file not specified\n" );
    return 1;
  }
  
  tape_init();
  
  data = read_file( input_name, &size );
  
  if ( data == NULL )
  {
    fprintf( stderr, "Error: could not read input file: %s\n", strerror( errno ) );
    return 1;
  }
  
  ud.tape = tape_open( data, size );
  
  if ( ud.tape == NULL )
  {
    fprintf( stderr, "Error: input file is not a tape, or file is corrupted\n" );
    free( data );
    return 1;
  }
  
  free( data );
  
  if ( SDL_Init( SDL_INIT_AUDIO ) != 0 )
  {
    fprintf( stderr, "Error: couldn't initialize the audio output: %s\n", SDL_GetError() );
    tape_close( ud.tape );
    return 1;
  }
  
  want.freq = 22050;
  want.format = AUDIO_U8;
  want.channels = 1;
  want.samples = 4096;
  want.callback = generate;
  want.userdata = &ud;
  
  devid = SDL_OpenAudioDevice( NULL, 0, &want, &ud.have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE );
  
  if ( devid == 0 )
  {
    fprintf( stderr, "Error: couldn't initialize the audio device: %s\n", SDL_GetError() );
    SDL_Quit();
    tape_close( ud.tape );
    return 1;
  }
  
  SDL_PauseAudioDevice( devid, 0 );
  SDL_Delay( 1000 );
  
  while ( tape_is_playing( ud.tape ) )
  {
    SDL_Delay( 10 );
  }
  
  SDL_CloseAudioDevice( devid );
  SDL_Quit();
  tape_close( ud.tape );
  return 0;
}
