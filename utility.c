#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "utility.h"

/*
	case insensitive string compare - can add to a seperate utility header
	1 if equal
	0 if not equal
*/
int strcicmp(char const *a, char const *b) {
	int i, j;
	// printf("the first string is %s and it's length is %d, the second string len is %d and it is %s", a, strlen(a), strlen(b), b);
  for (i = 0, j = 0; i < strlen(a) && j < strlen(b) - 1; i++, j++) {
		if (a[i] != b[j]) {
			// printf("i run %d times ", i);
			return 0;
		}
	}
	if (strlen(b) - 1 > strlen(a)) return 0;
  return 1;
}

/*
	get a substring from a string
	start_pos is the char you want to start from
	end_pos is the last char
	for eg : string = hello world 
	substring = world
	start_pos = 6
	end_pos = 11
*/
char* substring(char* string, int start_pos, int end_pos) {
	char* ans; 
	int c;

	ans = malloc(end_pos + 1);

	if (ans == NULL) {
		printf("Unable to allocate memory.\n");
		exit(1);
	}

	for (c = 0; c < end_pos - start_pos; c++) {
		*(ans + c) = *(string + start_pos + c);
	}

	*(ans + c - 1) = '\0';

	return ans;
}

/* 
	reverse a string
*/
void strreverse(char* begin, char* end) {
	
	char aux;
	
	while(end>begin)
	
		aux=*end, *end--=*begin, *begin++=aux;
	
}

/*
	convert int to char*
	in pure ANSI C
*/
void itoa(int value, char* str, int base) {
	
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int sign;

	// Validate base
	if (base<2 || base>35){ *wstr='\0'; return; }

	// Take care of sign
	if ((sign=value) < 0) value = -value;

	// Conversion. Number is reversed.
	do *wstr++ = num[value%base]; while(value/=base);
	
	if(sign<0) *wstr++='-';
	
	*wstr='\0';

	// Reverse string
	strreverse(str,wstr-1);

}

