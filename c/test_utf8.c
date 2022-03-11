#include "utf8.h"
#include <stdio.h>
#include <assert.h>

int main() {
    utf8* text = utf8_create("this is a new sentence");
    printf("%d %d %d %s\n", text->hash, text->byte_count, text->character_count, text->bytes);
    utf8_destroy(&text);

    utf8* text2 = utf8_create("\xe6\x97\xa5\xd1\x88");
    printf("%d %d %d %s\n", text2->hash, text2->byte_count, text2->character_count, text2->bytes);
    utf8_destroy(&text2);

    utf8* a = utf8_create("here");
    utf8* b = utf8_create("here");
    utf8* c = utf8_create("herE");
    assert(utf8_is_equal(a,b) == True);
    assert(utf8_is_equal(a,c) == False);
    utf8_destroy(&a);
    utf8_destroy(&b);
    utf8_destroy(&c);
    
    return 0;
}