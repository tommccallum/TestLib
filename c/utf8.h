#ifndef UTF8
#define UTF8

#include "bool.h"

typedef struct _utf8_string {
    int character_count;
    int byte_count;
    char* bytes;
    int hash;
} utf8;


int utf8_hash(char const * s);
int utf8_move_next(char const * bytes, int pos);
int utf8_character_count(char const * bytes);

utf8* utf8_create(char const * str);
void  utf8_destroy(utf8** text);
char* utf8_c(utf8* text);
bool  utf8_is_equal(utf8* a, utf8* b );

#endif