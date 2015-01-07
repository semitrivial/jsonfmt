/*
 * Json-Formatter: A JSON prettier in C.
 * Turns arbitrary JSON strings into beautiful properly-whitespaced equivalents.
 *
 * By Sam Alexander
 *
 * On github:  https://github.org/semitrivial/jsonfmt            
 */

char *json_format( const char *json, int indents, char **errptr );
