/*
 ============================================================================
 Name        : nicson.c
 Author      : Nick
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "json.h"

int main(int count, const char**argv) {
	if(count < 1) {
		puts("Error: missing filename argument!");
		return EXIT_FAILURE;
	}
	printf("Loading JSON: %s\n", argv[1]);
	JValue *val = jsonParse(argv[1]);

	if(!val) {
	  exit(0);
	}
	
	if(count >= 3) {
	  JObject *obj = (JObject*)val->value;
	  printf("%s: %d\n", argv[2], jsonInt(obj, argv[2]));
	}
	
	jsonFree(val);
	return EXIT_SUCCESS;
}
