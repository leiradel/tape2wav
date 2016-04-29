#include <common.h>
#include <stdio.h>
#include <sys/stat.h>

void* read_file( const char* filename, size_t* length )
{
  struct stat file_info;
  FILE* file = NULL;
  void* buffer = NULL;
  
  if ( stat( filename, &file_info ) == -1 )
  {
    return NULL;
  }

  *length = file_info.st_size;
  buffer = malloc( *length );

  if ( buffer == NULL )
  {
    return NULL;
  }

  file = fopen(filename, "rb");

  if ( file == NULL )
  {
    free( buffer );
    return NULL;
  }

  if ( fread( buffer, 1, *length, file ) != *length )
  {
    fclose( file );
    free( buffer );
    return NULL;
  }

  if ( fclose( file ) != 0 )
  {
    free( buffer );
    return NULL;
  }

  return buffer;
}
