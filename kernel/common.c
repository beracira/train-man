#include "common.h"

int strlen( char * str ) {
  int i = 0;
  while (str[i++]);
  return i;
}

void strcpy( char * origin, char * dest) {
  int i = 0;
  while (dest[i] = origin[i++]);
  return; 
}

int strcmp( char * str1, char * str2) {
  int i = 0;
  int len_1 = strlen(str1);
  int len_2 = strlen(str2);
  if (len_1 != len_2) return 0;
  while (i < len_1) 
    if (str1[i] != str2[i++]) return 0;
  return 1;
}
