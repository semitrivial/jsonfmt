/*
 * Json-Formatter: A JSON prettier in C.
 * Turns arbitrary JSON strings into beautiful properly-whitespaced equivalents.
 *
 * Also contains a JSON escape function.
 *
 * By Sam Alexander
 *
 * On github:  https://github.org/semitrivial/jsonfmt
 */

#include <string.h>
#include <stdlib.h>
#include "jsonfmt_internal.h"

/*
 * Main function: given a string of json, prettify it with
 * beautiful whitespace.  "indents" is how many spaces to
 * add for each level of braces.  On error, returns NULL and
 * writes an error message to *errptr (if errptr != NULL).
 * (The error message is not malloc'd and doesn't need freed.)
 * On success, returns the beautified json.
 *
 * Memory management: To free the strings output by json_format
 * (and other functions in this library), call json_gc().
 */
char *json_format( const char *json, int indents, char **errptr )
{
  int len=0, indent=0, last_was_nonspace=0, fQuote=0, fContent=0;
  const char *ptr;
  char *buf, *bptr;

  if ( indents > MAX_INDENT_SIZE )
    JSONFMT_ERR( "Indents too large" );

  if ( indents < 0 )
    JSONFMT_ERR( "Negative indents" );

  if ( !json )
    JSONFMT_ERR( "Null input" );

  /*
   * First pass: calculate the length of the result, for memory allocation purpose
   */
  for ( ptr = json; *ptr; ptr++ )
  {
    /*
     * Escaped sequences get copied verbatim
     */
    if ( *ptr == '\\' )
    {
      if ( !ptr[1] )
        JSONFMT_ERR( "Last input character is backslash" );

      len += 2;
      ptr++;
      last_was_nonspace = 1;
      fContent = 1;
      continue;
    }

    /*
     * Anything between quotes goes unchanged
     */
    if ( fQuote )
    {
      if ( *ptr == '"' )
      {
        fQuote = 0;
        last_was_nonspace = 1;
      }

      len += 2; // Shortcut: Don't count length from opening quote until closing quote found
      continue;
    }

    /*
     * If not in quotes, what to do depends on which character we're reading.
     */
    switch( *ptr )
    {
      /*
       * Ignore spaces unless the previous character (if any) was a non-space
       */
      case ' ':
      case '\t':
        if ( last_was_nonspace )
        {
          len++;
          last_was_nonspace = 0;
        }
        break;

      /*
       * Ignore linebreaks
       */
      case '\n':
      case '\r':
        break;

      /*
       * Put opening brace on new line (same indent), then increase indent and add another new line
       */
      case '{':
      case '[':
        if ( fContent )
          len += 3 + 2*indent + indents;
        else
        {
          len += 2 + 2*indent + indents;
          fContent = 1;
        }

        indent += indents;
        last_was_nonspace = 0;
        break;

      /*
       * On closing brace, decrease indent and then add newline
       */
      case '}':
      case ']':
        if ( !indent )
          JSONFMT_ERR( (*ptr == '}') ? "Mismatched closing brace" : "Mismatched closing bracket" );

        indent -= indents;
        len += 2 + indent;
        last_was_nonspace = 1;
        break;

      /*
       * Quote?  Make a note that subsequent characters are enquoted.
       */
      case '"':
        fQuote = 1;
        //len++;  Shortcut: Don't count the length until the quote ends
        fContent = 1;
        break;

      /*
       * Comma? Add newline and indent
       */
      case ',':
        len += 2 + indent;
        last_was_nonspace = 0;
        fContent = 1;
        break;

      /*
       * Any other case: copy verbatim
       */
      default:
        len++;
        last_was_nonspace = 1;
        fContent = 1;
        break;
    }
  }

  if ( fQuote )
    JSONFMT_ERR( "Non-terminated quote detected" );

  if ( indent )
    JSONFMT_ERR( "Non-terminated brace or bracket detected" );

  if ( !fContent )
  {
    buf = malloc( sizeof(char) );
    *buf = '\0';
    return buf;
  }

  /*
   * Now that we know how much space is needed for the prettified json, allocate it.
   */
  buf = malloc( (len+1) * sizeof(char) );

  if ( !buf )
    JSONFMT_ERR( "Could not allocate enough memory" );

  last_was_nonspace = fContent = 0;

  /*
   * Initiate spaces buffer (if necessary)
   */
  add_spaces( NULL, 0 );

  /*
   * Second pass: Populate our new buffer with prettified json
   */
  for ( bptr = buf, ptr = json; *ptr; ptr++ )
  {
    /*
     * Escaped sequences get copied verbatim
     */
    if ( *ptr == '\\' )
    {
      *bptr++ = '\\';
      ptr++;
      *bptr++ = *ptr;
      last_was_nonspace = 1;
      fContent = 1;
      continue;
    }

    /*
     * Characters enclosed in quotes get copied verbatim
     */
    if ( fQuote )
    {
      if ( *ptr == '"' )
      {
        fQuote = 0;
        last_was_nonspace = 1;
      }

      *bptr++ = *ptr;
      continue;
    }

    /*
     * Outside of quotes: proceed depending on which character we see
     */
    switch( *ptr )
    {
      /*
       * Ignore spaces unless previous character was non-space
       */
      case ' ':
      case '\t':
        if ( last_was_nonspace )
        {
          *bptr++ = ' ';
          last_was_nonspace = 0;
        }
        break;

      /*
       * Ignore newlines altogether
       */
      case '\n':
      case '\r':
        break;

      /*
       * Put opening braces on new line, followed by another newline and indent
       */
      case '{':
      case '[':
        /*
         * Exception 1: If the list is empty, write it as [], with no newline preceding
         */
        {
          char what = (*ptr == '{' ? '}' : ']');
          char const *where;

          if ( next_nonwhitespace_is( ptr, what, &where ) )
          {
            *bptr++ = *ptr;
            *bptr++ = what;
            ptr = where;
            fContent = 1;
            last_was_nonspace = 1;
            continue;
          }
        }

        /*
         * Exception 2: Omit the initial newline if this brace is the first content we see,
         * or if the last nonspace we wrote was a newline.
         */
        if ( fContent )
        {
          if ( !last_nonspace_was_newline( bptr, buf ) )
          {
            *bptr++ = '\n';
            add_spaces( &bptr, indent );
          }
        }
        else
          fContent = 1;

        *bptr++ = *ptr;
        *bptr++ = '\n';
        indent += indents;
        add_spaces( &bptr, indent );
        last_was_nonspace = 0;
        break;

      /*
       * Put closing braces on newline
       */
      case '}':
      case ']':
        indent -= indents;
        *bptr++ = '\n';
        add_spaces( &bptr, indent );
        *bptr++ = *ptr;
        last_was_nonspace = 1;
        break;

      /*
       * Quote?  Make a note that subsequent characters are enquoted
       */
      case '"':
        *bptr++ = '"';
        fQuote = 1;
        fContent = 1;
        break;

      /*
       * Comma? Add a newline.
       */
      case ',':
        *bptr++ = ',';
        *bptr++ = '\n';
        add_spaces( &bptr, indent );
        last_was_nonspace = 0;
        fContent = 1;
        break;

      /*
       * Any other case? Copy verbatim
       */
      default:
        *bptr++ = *ptr;
        last_was_nonspace = 1;
        fContent = 1;
        break;
    }
  }

  /*
   * Nul-terminate our buffer and return it.
   */
  *bptr = '\0';
  return prep_for_json_gc(buf);
}

char *json_escape( const char *txt )
{
  return json_escape_( txt, 1 );
}

char *json_escape_no_gc( const char *txt )
{
  return json_escape_( txt, 0 );
}

char *json_escape_( const char *txt, int gc )
{
  int len;
  const char *ptr;
  char *buf, *bptr;

  for ( len=0, ptr = txt; *ptr; ptr++ )
  {
    switch( *ptr )
    {
      case '"':
      case '\\':
        len++;
      default:
        continue;
    }
  }

  len += (ptr - txt);

  buf = malloc( (len+1) * sizeof(char) );

  if ( !buf )
    return NULL;

  for ( ptr = txt, bptr = buf; *ptr; ptr++ )
  {
    switch( *ptr )
    {
      case '"':
      case '\\':
        *bptr++ = '\\';
      default:
        *bptr++ = *ptr;
    }
  }

  *bptr = '\0';

  return gc ? prep_for_json_gc(buf) : buf;
}

/*
 *  Add some spaces at a (pointer to a) location, then increase the pointer by that much.
 *
 *  Requires initialization via calling add_spaces( NULL, 0 ).
 *  Repeated initializations are idempotent.
 */
void add_spaces( char **ptr, int count )
{
  static char *spaces;

  if ( !count )
  {
    /*
     * If uninitialized, allocate a big buffer of spaces
     * (so we can exploit the fast performance of memcpy)
     */
    if ( !spaces )
    {
      char *left, *right;
      int len = (MAX_LEVEL * MAX_INDENT_SIZE + 1) * sizeof(char);

      spaces = malloc( len );

      /*
       * Fill the spaces buffer with spaces
       */
      for ( left = spaces, right = &spaces[len]; left < right; left++ )
        *left = ' ';
    }

    /*
     * If 0 spaces are to be added, then do nothing.
     */
    return;
  }

  /*
   * Add all the spaces at once, rather than one-by-one
   */
  memcpy( *ptr, spaces, count * sizeof(char) );

  *ptr += count;
}

int next_nonwhitespace_is( const char *ptr, char c, const char **where )
{
  const char *rightptr = &ptr[1];

  for ( rightptr = &ptr[1]; *rightptr && *rightptr != c; rightptr++ )
  {
    switch( *rightptr )
    {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        continue;
      default:
        break;
    }
    break;
  }

  if ( *rightptr == c )
  {
    *where = rightptr;
    return 1;
  }

  return 0;
}

int last_nonspace_was_newline( char *ptr, char *buf )
{
  for ( ; ; )
  {
    if ( ptr <= buf )
      return 0;

    ptr--;

    if ( *ptr == ' ' )
      continue;

    if ( *ptr == '\n' )
      return 1;

    return 0;
  }
}

/*
 * Experimental material follows
 */
json_str *first_js_str[JSON_HASH];
json_str *last_js_str[JSON_HASH];

int is_json( const char *str )
{
  json_str *x;
  unsigned char hash = get_js_hash( str );

  for ( x = first_js_str[hash]; x; x = x->next )
    if ( x->str == str )
      return 1;

  return 0;
}

unsigned char get_js_hash( char const *str )
{
  while ( *str == '\"' || *str == '{' || *str == '[' )
    str++;

  return (unsigned char) *str;
}

char *prep_for_json_gc( char *str )
{
  json_str *x;
  int hash;

  x = malloc( sizeof( json_str ) );

  if ( !x )
    return NULL;

  x->str = str;

  hash = get_js_hash( str );

  JSONFMT_LINK( x, first_js_str[hash], last_js_str[hash], next );

  return str;
}

void json_gc( void )
{
  json_str *x, *x_next;
  int hash;

  for ( hash = 0; hash < JSON_HASH; hash++ )
  {
    for ( x = first_js_str[hash]; x; x = x_next )
    {
      x_next = x->next;

      free( x->str );
      free( x );
    }
    first_js_str[hash] = NULL;
    last_js_str[hash] = NULL;
  }
}

char *json_c_adapter( int paircnt, ... )
{
  va_list vargs;
  char **args, **argspt, *ch, *buf, *bptr;
  int i, len, rawcnt;

  rawcnt = paircnt * 2;

  if ( (args = malloc(sizeof(char*) * (1+rawcnt))) == NULL )
    return NULL;

  va_start( vargs, paircnt );

  for ( i = 0, len = 0; i < rawcnt; i++ )
  {
    ch = va_arg( vargs, char * );

    if ( is_json( ch ) )
    {
      args[i] = ch;
      len += strlen( ch );
    }
    else
    {
      char *escaped = json_escape_no_gc( ch );
      args[i] = json_enquote( escaped );
      free( escaped );
      len += strlen( args[i] );
    }
  }
  args[rawcnt] = NULL;

  va_end( vargs );

  len += strlen("{}") + ( strlen(":,") * paircnt ) - strlen(",");

  if ( (buf = malloc(len+1)) == NULL )
  {
    free( args );
    return NULL;
  }

  bptr = buf;
  *bptr = '{';

  for ( argspt = args; *argspt; argspt += 2 )
  {
    ++bptr;
    strcpy( bptr, *argspt );
    bptr = &bptr[strlen(bptr)];

    *bptr++ = ':';

    strcpy( bptr, argspt[1] );
    bptr = &bptr[strlen(bptr)];
    *bptr = ',';
  }

  *bptr = '}';
  bptr[1] = '\0';

  free( args );

  return prep_for_json_gc( buf );
}

char *json_enquote( const char *str )
{
  char *buf = malloc( strlen(str) + strlen("\"\"") + 1 );

  sprintf( buf, "\"%s\"", str );

  return prep_for_json_gc( buf );
}

char *json_array_worker( char * (*fnc) (void *), void **array )
{
  char **results, **rptr, *buf, *bptr;
  void **ptr;
  int cnt, len;

  for ( ptr = array; *ptr; ptr++ )
    ;

  cnt = ptr - array;

  if ( !cnt )
    return prep_for_json_gc( strdup( "[]" ) );

  if ( (results = malloc( sizeof( char *) * (cnt+1) )) == NULL )
    return NULL;

  rptr = results;

  for ( ptr = array, len = 0; *ptr; ptr++ )
  {
    if ( (*rptr = (*fnc) (*ptr) ) == NULL )
    {
      free( results );
      return NULL;
    }
    len += strlen( *rptr );
    rptr++;
  }

  *rptr = NULL;

  len += strlen("[]") + ( strlen(",") * (cnt-1) );

  if ( (buf = malloc( len + 1 )) == NULL )
  {
    free( results );
    return NULL;
  }

  bptr = buf;
  *bptr = '[';

  for (rptr = results; *rptr; rptr++)
  {
    bptr++;
    strcpy( bptr, *rptr );
    bptr = &bptr[strlen(bptr)];
    *bptr = ',';
  }
  bptr[0] = ']';
  bptr[1] = '\0';

  return prep_for_json_gc(buf);
}
