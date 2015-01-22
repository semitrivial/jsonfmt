/*
 * Json-Formatter: A JSON prettier in C.
 * Turns arbitrary JSON strings into beautiful properly-whitespaced equivalents.
 *
 * This example test program can safely be deleted.
 * Its purpose is to show how to use JSON-Formatter.
 *
 * By Sam Alexander
 *
 * On github:  https://github.org/semitrivial/jsonfmt
 */

#include <stdio.h>
#include <stdlib.h>
#include "jsonfmt.h"

int main(void)
{
  char *x = "{\"lol\": xxx, \"empty\": [  ], \"whatever\":[a,b,c,\"\\t\"]}";
  char *err = NULL;
  char *y = json_format( x, 2, &err );
  char *dialog, *escaped;

  if ( err )
  {
    printf( "Error: %s\n", err );
    return 0;
  }

  printf( "%s\n\n", y );

  free( y );

  dialog = "\"Hello, \\ World,\" said the poet.";
  escaped = json_escape( dialog );

  printf( "Dangerous dialog: %s\n", dialog );
  printf( "Escaped JSON:  {\"dialog\": \"%s\"}\n\n", escaped );

  free( escaped );

  return 1;
}
