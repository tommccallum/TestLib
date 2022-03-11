/**
 * @file universe_date.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-10
 * 
 * @copyright Copyright (c) 2022
 * 
 * A date library for the universe
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

const double PLANK_CONSTANT = 5.43e-44;


// A number is following series:
// N=a(127) + b
typedef struct _date {
  unsigned char a;
  signed char b;
} UniverseDate;

const unsigned  char  UNSIGNED_CHAR_MAXIMUM = (char) ((1 << 8) - 1);
const unsigned  char  UNSIGNED_CHAR_MINIMUM = (char) 0;
const signed    char  SIGNED_CHAR_MAXIMUM   = (char) ((1 << 7) - 1);
const signed    char  SIGNED_CHAR_MINIMUM   = (char) (1 << 7);
const int16_t SIGNED_SHORT_MINIMUM = (int16_t) (1<<15);

UniverseDate UniverseDate_create() {
  UniverseDate a = {0};
  return a;
}

void UniverseDate_print(UniverseDate a) {
  printf("{%u,%d}\n", a.a, a.b);
}

UniverseDate UniverseDate_add(UniverseDate date, int increment) {
  if ( (char)(date.a + increment) < date.a ) {
    // overflow so we carry one
    if ( (date.b + 1) < date.b ) {
      // overflow
      printf("too much\n");
    } else {
      date.b += 1;
      date.a = date.a + increment;
    }
  } else {
    date.a += increment;
  }
  return date;
}

char* addStrings(char* num1, char* num2) {
    char buffer[255];
    memset(buffer, 0, 255);
    int num1_neg = 0;
    int num2_neg = 0;
    int num1_it = 0, num2_it = 0;
    if ( num1[0] == '-' ) {
      num1_neg = 1;
    }
    if ( num2[0] == '-' ) {
      num2_neg = 1;
    }
    int sz1 = strlen(num1);
    int sz2 = strlen(num2);
    int carry = 0;
    int buffer_it=0;
    int v = 0;
    while (num1_it < sz1 || num2_it < sz2 || carry > 0) {
        int t = 0;
        int dirty = 0;
        if ( carry ) {
          t = carry;
          dirty = 1;
        }
        if ( num1[sz1 - num1_it - 1] != '-' ) {
          // if ( num1_neg > 0 && sz1 - num1_it - 1 == 1 ) {
          //   v = num1_it < sz1 ? num1[sz1 - num1_it - 1] - 48 : 0;
          //   t += -1 * v;
          // } else {
            t += num1_it < sz1 ? num1[sz1 - num1_it - 1] - 48 : 0;
            dirty = 1;
          // }
        }
        if ( num2[sz2 - num2_it - 1] != '-' ) {
          // if ( num2_neg > 0 && sz2 - num2_it - 1 == 1 ) {
          //   v =  num2_it < sz2 ? num2[sz2 - num2_it - 1] - 48 : 0;
          //   t += -1 * v;
          // } else {
            t += num2_it < sz2 ? num2[sz2 - num2_it - 1] - 48 : 0;
            dirty = 1;
          // }
        }
        if ( dirty ) {
          buffer[buffer_it] = t % 10 + 48;
          buffer_it++;
        }
        carry = t / 10; // carry over next digit
        num1_it ++;
        num2_it ++;
    }
    if ( num1_neg || num2_neg ) {
      buffer[buffer_it] = '-';
      buffer_it++;
    }
    // number is currently in reverse
    int n = strlen(buffer);
    char* ans = NULL;
    if ( n > 0 ) {
      ans = (char*) malloc(n * sizeof(char));
      memset(ans, 0, n);
      for( int ii=n-1, jj=0; ii >= 0; ii--, jj++ ) {
        ans[jj] = buffer[ii];
      }
    }
    return ans;
}

char* UniverseDate_toString(UniverseDate date) {
  char *buffer = (char*) malloc(sizeof(char)*255);
  memset(buffer, 0, 255);
  int index = 0;
  if ( date.b == 0 && date.a == 0 ) {
    buffer[index] = 0;
  } else {
    char* base = (char*) malloc(sizeof(char)*2);
    base[0] = '1';
    base[1] = '\0';
    char mask = 0;
    for( int bit=0; bit < 16; bit++) {
      if ( bit > 7 ) {
        if ( bit == 15 ) {
          // take account of 2's complement so this will be -128
          char* old_base = base;
          int nth = strlen(base)+1;
          base = (char*) malloc(sizeof(char)*strlen(base)+2);
          base[0] = '-';
          strncpy(&base[1], old_base, nth-1);
          base[nth] = '\0';
          free(old_base);
        }
        mask = 1 << (bit-8);
        if ( (date.b & mask) == mask ) {
          // then we have a 1
          buffer = addStrings(buffer, base);
        }
      } else {
        mask = 1 << bit;
        if ( (date.a & mask) == mask ) {
          // then we have a 1
          buffer = addStrings(buffer, base);
        }
      }
      // we double this each time e.g. 2^N
      char* newbase = addStrings(base, base);
      free(base);
      base = newbase;
    }
  }
  return buffer;
}

int main() {
  printf("Min unsigned char: %u\n", UNSIGNED_CHAR_MINIMUM);
  printf("Max unsigned char: %u\n", UNSIGNED_CHAR_MAXIMUM);
  printf("Min signed char: %d\n", SIGNED_CHAR_MINIMUM);
  printf("Max signed char: %d\n", SIGNED_CHAR_MAXIMUM);
  printf("Min signed short: %d\n", SIGNED_SHORT_MINIMUM);


  UniverseDate a = UniverseDate_create();
  // UniverseDate_print(a);

  // // test one will our second value increment when we overflow
  // printf("testing when a=255 step=1\n");
  // a.a = 255;
  // int step = 1;
  // UniverseDate_print(a);
  // a = UniverseDate_add(a, step);
  // UniverseDate_print(a);

  // printf("testing when a=255 step=6\n");
  // a.b = 0;
  // a.a = 255;
  // int step2 = 6;
  // UniverseDate_print(a);
  // a = UniverseDate_add(a, step2);
  // UniverseDate_print(a);

  // char* ans = addStrings("127", "123");
  // printf("%s\n", ans);
  // free(ans);
  
  // char* result = UniverseDate_toString(a);
  // printf("%s\n", result);
  // free(result);
  // result = NULL;

  a.b = SIGNED_CHAR_MINIMUM;
  a.a = 0;
  char* result = UniverseDate_toString(a);
  printf("%s\n", result);

  // for( int ii=0; ii < SIGNED_CHAR_MAXIMUM * UNSIGNED_CHAR_MAXIMUM; ii++ ) {
    // a.a = 255;
    // int ii = 1;
    // a = UniverseDate_add(a, 1);
    // UniverseDate_print(a);
  // }
}

