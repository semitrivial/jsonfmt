 Json-Formatter: A JSON prettifier function in C.
 Turns arbitrary JSON strings into beautiful properly-whitespaced equivalents.
 Also contains a JSON escape function.
 
 By Sam Alexander
 
 On github:  https://github.org/semitrivial/jsonfmt            

-----------------------------------------------------------
USAGE
-----------------------------------------------------------

* Compile jsonfmt.c along with your project.
* #include jsonfmt.h in any file where you want to use Json-Formatter.
* To minimize name conflicts, you do not need to #include jsonfmt_internal.h anywhere
  (only jsonfmt.c #includes it).
* Call the json_format function to beautify your JSON.

Here is the documentation for json_format:
 
Main function: given a string of json, prettify it with
beautiful whitespace.  "indents" is how many spaces to
add for each level of braces.  On error, returns NULL and
writes an error message to *errptr (if errptr != NULL).
(The error message is not malloc'd and doesn't need freed.)
On success, returns a string containing beautified JSON.
To free this string (and all others allocated by the library),
call json_gc().

  char *json_format( const char *json, int indents, char **errptr )

-----------------------
json_escape
-----------------------

This library also contains a JSON escape function:

  char *json_escape( const char *txt );

This function takes a non-NULL string, txt, and inserts an escape
character \ before any double-quotes and any backslashes.

Returns a string, or NULL in case of insufficient RAM.
In order to free the string (along with all other strings allocated
by the library), call json_gc().

-----------------------
json_gc
-----------------------
Call this function to free all memory that as allocated for
the library so far.

void json_gc( void );


-----------------------------------------------------------
EXAMPLE FILE
-----------------------------------------------------------

A very simple test file, test.c, is included for demo purposes.

To compile it with gcc: gcc -g -Wall test.c jsonfmt.c -o test

-----------------------------------------------------------
LICENSE
-----------------------------------------------------------

This code is released under the MIT license
