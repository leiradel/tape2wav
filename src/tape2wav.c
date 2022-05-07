#include <tape.h>
#include <common.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 4096

typedef struct
{
  /* header */
  char     chunkid[4];
  uint32_t chunksize;
  char     format[4];
  /* fmt */
  char     subchunk1id[4];
  uint32_t subchunk1size;
  uint16_t audioformat;
  uint16_t numchannels;
  uint32_t samplerate;
  uint32_t byterate;
  uint16_t blockalign;
  uint16_t bitspersample;
  /* data */
  char     subchunk2id[4];
  uint32_t subchunk2size;
  uint8_t  data[0];

}
wave_t;

static void usage( FILE* out )
{
  fprintf( out, "Usage: tape2wav options...\n\n" );
  fprintf( out, "-i filename        Input file name\n" );
  fprintf( out, "-o filename        Output file name (default is tape2wav.wav)\n" );
  fprintf( out, "-r sample_rate     Output sample rate (default is 44100)\n" );
  fprintf( out, "-s                 Stereo output (default is mono)\n" );
  fprintf( out, "-8                 Unsigned 8-bits per sample (default signed 16-bits)\n" );
  fprintf( out, "-h                 This page\n\n" );
}

int main( int argc, const char* argv[] )
{
  const char* input_name = NULL;
  const char* output_name = "tape2wav.wav";
  int sample_rate = 44100;
  int stereo = 0;
  int bits = 16;
  
  int i;
  void* data;
  size_t size, j, total;
  tape_t* tape;
  uint8_t pcm[ BUFFER_SIZE ];
  uint8_t conv[ BUFFER_SIZE * 4 ];
  uint8_t* u8;
  uint16_t* u16;
  FILE* out;
  wave_t wav;
  
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
    else if ( !strcmp( argv[ i ], "-o" ) )
    {
      if ( ++i == argc )
      {
        fprintf( stderr, "Error: missing argument to -o\n" );
        return 1;
      }
      
      output_name = argv[ i ];
    }
    else if ( !strcmp( argv[ i ], "-r" ) )
    {
      if ( ++i == argc )
      {
        fprintf( stderr, "Error: missing argument to -r\n" );
        return 1;
      }
      
      sample_rate = atoi( argv[ i ] );
      
      if ( sample_rate < 11025 || sample_rate > 1000000 )
      {
        fprintf( stderr, "Error: sample rate outside reasonable bounds\n" );
        return 1;
      }
    }
    else if ( !strcmp( argv[ i ], "-s" ) )
    {
      stereo = 1;
    }
    else if ( !strcmp( argv[ i ], "-8" ) )
    {
      bits = 8;
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
  
  tape = tape_open( data, size );
  
  if ( tape == NULL )
  {
    fprintf( stderr, "Error: input file is not a tape, or file is corrupted\n" );
    free( data );
    return 1;
  }
  
  free( data );
  
  total = 0;
  
  for ( ;; )
  {
    size = tape_play( tape, sample_rate, pcm, sizeof( pcm ) );
    
    if ( size ==  0)
    {
      break;
    }
    
    if ( !stereo )
    {
      if ( bits == 8 )
      {
        total += size;
      }
      else
      {
        total += size * 2;
      }
    }
    else
    {
      if ( bits == 8 )
      {
        total += size * 2;
      }
      else
      {
        total += size * 4;
      }
    }
  }
  
  tape_rewind( tape );

  out = fopen( output_name, "wb" );
  
  if ( out == NULL )
  {
    fprintf( stderr, "Error: could not open '%s' for writing: %s\n", output_name, strerror( errno ) );
    tape_close( tape );
    return 1;
  }
  
  /* header */
  wav.chunkid[0] = 'R';
  wav.chunkid[1] = 'I';
  wav.chunkid[2] = 'F';
  wav.chunkid[3] = 'F';
  wav.chunksize = sizeof(wave_t) + total - 8; /* totalsize - 8 (chunkid and chunksize) */
  wav.format[0] = 'W';
  wav.format[1] = 'A';
  wav.format[2] = 'V';
  wav.format[3] = 'E';
  /* fmt */
  wav.subchunk1id[0] = 'f';
  wav.subchunk1id[1] = 'm';
  wav.subchunk1id[2] = 't';
  wav.subchunk1id[3] = ' ';
  wav.subchunk1size = 16;
  wav.audioformat = 1;
  wav.numchannels = stereo ? 2 : 1;
  wav.samplerate = sample_rate;
  wav.byterate = sample_rate * wav.numchannels * bits / 8;
  wav.blockalign = wav.numchannels * bits / 8;
  wav.bitspersample = bits;
  /* data */
  wav.subchunk2id[0] = 'd';
  wav.subchunk2id[1] = 'a';
  wav.subchunk2id[2] = 't';
  wav.subchunk2id[3] = 'a';
  wav.subchunk2size = total; /* sample data size */
  
  if ( fwrite( &wav, 1, sizeof( wav ), out ) != sizeof( wav ) )
  {
  write_error:
    fprintf( stderr, "Error: could not write to '%s': %s", output_name, strerror( errno ) );
    fclose( out );
    unlink( output_name );
    tape_close( tape );
    return 1;
  }
  
  total = 0;
  
  for ( ;; )
  {
    size = tape_play( tape, sample_rate, pcm, sizeof( pcm ) );
    
    if ( size ==  0)
    {
      break;
    }
    
    if ( !stereo )
    {
      if ( bits == 8 )
      {
        memcpy( conv, pcm, size );
      }
      else
      {
        u16 = (uint16_t*)conv;
        
        for ( j = 0; j < size; j++ )
        {
          i = pcm[ j ];
          i = i * 65535 / 255 - 32768;
          *u16++ = i;
        }
        
        size *= 2;
      }
    }
    else
    {
      if ( bits == 8 )
      {
        u8 = conv;
        
        for ( j = 0; j < size; j++ )
        {
          i = pcm[ j ];
          *u8++ = i;
          *u8++ = i;
        }
        
        size *= 2;
      }
      else
      {
        u16 = (uint16_t*)conv;
        
        for ( j = 0; j < size; j++ )
        {
          i = pcm[ j ];
          i = i * 65535 / 255 - 32768;
          *u16++ = i;
          *u16++ = i;
        }
        
        size *= 4;
      }
    }
    
    if ( fwrite( conv, 1, size, out ) != size )
    {
      goto write_error;
    }
    
    total += size;
  }
  
  if ( fclose( out ) != 0 )
  {
    fprintf( stderr, "Error: could not close '%s': %s\n", output_name, strerror( errno ) );
    unlink( output_name );
    return 1;
  }
  
  tape_close( tape );
  return 0;
}
