#ifndef STRINGS_01081969_H
#define STRINGS_01081969_H

#define STR_CAPACITY 256
#define STR_MAX_LEN STR_CAPACITY-1
#define MAX_STR_LEN STR_MAX_LEN

/* Calling strlen, letting it step through a possibly long string,
   and then comparing the result to 0 would waste CPU cycles.
   Checking the first character for a null byte is very fast. */
#define IS_EMPTY_STRING(s) (s!=NULL && s[0]=='\0')

/* Checks for not NULL and not empty string. Do not use this on an
   uninitialized string. */
#define CONTAINS_A_VALUE(s) (s!=NULL && s[0]!='\0')

/* Removes any data, setting it to a zero-length, property terminated
   string. */
#define CLEAR_STRING(s) s[0]='\0'

#endif
