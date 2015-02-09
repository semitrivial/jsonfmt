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

char *json_format( const char *json, int indents, char **errptr );
char *json_escape( const char *txt );

/*
 * Experimental functions
 */
char *json_c_adapter( int paircnt, ... );
void json_gc( void );


/*
* FOR_EACH macro thanks to Gregory Pakosz.
*
* FOR_EACH( what, ... ) applies what to all following arguments.
* Unfortunately, the max number of arguments must be hard-coded,
* due to limitations in the preprocessor. For our purposes
* here, eight arguments will suffice.
*/
#define FOR_EACH_MAX_ARGS 8
#define FOR_EACH_1(what, x, ...) what(x)
#define FOR_EACH_2(what, x, ...)\
what(x)\
FOR_EACH_1(what, __VA_ARGS__)
#define FOR_EACH_3(what, x, ...)\
what(x)\
FOR_EACH_2(what, __VA_ARGS__)
#define FOR_EACH_4(what, x, ...)\
what(x)\
FOR_EACH_3(what, __VA_ARGS__)
#define FOR_EACH_5(what, x, ...)\
what(x)\
FOR_EACH_4(what, __VA_ARGS__)
#define FOR_EACH_6(what, x, ...)\
what(x)\
FOR_EACH_5(what, __VA_ARGS__)
#define FOR_EACH_7(what, x, ...)\
what(x)\
FOR_EACH_6(what, __VA_ARGS__)
#define FOR_EACH_8(what, x, ...)\
what(x)\
FOR_EACH_7(what, __VA_ARGS__)
#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
#define CONCATENATE(arg1, arg2) CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2) CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2) arg1##arg2
#define STRINGIZE(arg) STRINGIZE1(arg)
#define STRINGIZE1(arg) STRINGIZE2(arg)
#define STRINGIZE2(arg) #arg
#define FOR_EACH_(N, what, x, ...) CONCATENATE(FOR_EACH_, N)(what, x, __VA_ARGS__)
#define FOR_EACH(what, x, ...) FOR_EACH_(FOR_EACH_NARG(x, __VA_ARGS__), what, x, __VA_ARGS__)
/*
* Magical JSON macros (using the FOR_EACH technology above)
*
* The novel discovery is the MAGIC_SPLIT macro which turns
* a single argument, (key: val), into two arguments: (key), (val).
* This is accomplished by hijacking the ternary operator.
*
* Example:
* The expression
* JSON( "id": "1", "name": get_name() )
* expands to
* jsonf( 2, "id", "1", "name", get_name() )
* or rather, to be more specific, to
* jsonf( 0+1+1, "id", "1", "name", get_name() )
*/
#define MAGIC_SPLIT( x ) , 1 ? x, 0 ? x
#define ADD_ONE( x ) + 1
#define JSON(...)\
json_c_adapter\
(\
0 FOR_EACH( ADD_ONE, __VA_ARGS__ )\
FOR_EACH( MAGIC_SPLIT, __VA_ARGS__ )\
)
