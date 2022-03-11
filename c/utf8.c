#include "bool.h"
#include <stdlib.h>
#include <string.h>

typedef struct _utf8_string {
    int character_count;
    int byte_count;
    char* bytes;
    int hash;
} utf8;


// int hash = 7;
// for (int i = 0; i < strlen; i++) {
//     hash = hash*31 + charAt(i);
// }
int utf8_hash(char const * s) {
    int hash = 7;       // must be a prime for good performance
    while ( *s ) {
        hash = hash * 31 + *s;    // Java uses 31 as this gave the least collisions
        s++;
    }
    return hash;
}

int utf8_move_next(char const * bytes, int pos) {
    int ch = bytes[pos] & 0xFF;
    if ( ch == 0 ) {
        return 0;
    }
    if ( (ch & (1 << 7)) == 0 ) {
        return pos + 1;
    } else {
        if ( (ch & (1<<6)) == (1<<6) ) {
            if ( (ch & (1<<5)) == (1<<5) ) {
                if ( (ch & ( 1<<4)) == (1<<4) ) {
                    return pos + 4;
                } else {
                    return pos + 3;
                }
            } else {
                return pos + 2;
            }
        } else {
            return pos + 1;
        }
    }
}

int utf8_character_count(char const * bytes) {
    int p = 0;
    int count = 0;
    while ( (p = utf8_move_next(bytes,p)) > 0 ) {
        count++;
    }
    return count;
}

utf8* utf8_create(char const * str) {
    utf8* text = (utf8*) malloc(sizeof(utf8));
    text->byte_count = strlen(str);
    text->character_count = utf8_character_count(str);
    text->hash = utf8_hash(str);
    text->bytes = strdup(str);
    return text;
}

void utf8_destroy(utf8** text) {
    free((*text)->bytes);
    free(*text);
    *text = NULL;
}

char* utf8_c(utf8* text) {
    return text->bytes;
}

bool utf8_is_equal(utf8* a, utf8* b ) {
    if ( a->hash != b->hash ) {
        return False;
    }
    if ( a->byte_count != b->byte_count ) {
        return False;
    }
    if ( strcmp(a->bytes, b->bytes) == 0 ) {
        return True;
    }
    return False;
}
