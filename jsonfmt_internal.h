/*
 * Json-Formatter: A JSON prettier in C.
 * Turns arbitrary JSON strings into beautiful properly-whitespaced equivalents.
 *
 * By Sam Alexander
 *
 * On github:  https://github.org/semitrivial/jsonfmt            
 */

#define JSONFMT_ERR( txt )\
  do\
  {\
    if ( errptr )\
      *errptr = (txt);\
    return NULL;\
  }\
  while(0)

#define MAX_LEVEL 128
#define MAX_INDENT_SIZE 32

void add_spaces( char **ptr, int count );
int next_nonwhitespace_is( const char *ptr, char c, const char **where );
int last_nonspace_was_newline( char *ptr, char *buf );
