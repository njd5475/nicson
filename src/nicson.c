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
  printf("Debug JArray(%d)\n", (int)sizeof(JArray));
  printf("Debug JObject(%d)\n", (int)sizeof(JObject));
  printf("Debug JEntry(%d)\n", (int)sizeof(JEntry));
	if(count < 1) {
		puts("Error: missing filename argument!");
		return EXIT_FAILURE;
	}
	printf("Loading JSON: %s\n", argv[1]);
	short type;
	JItemValue val = jsonParse(argv[1], &type);

	if(stringCache) {
	  printf("Strings cached used %d\n", stringCache->size);
	}

	if(!val.ptr_val) {
	  fprintf(stderr, "Error Parsing file!\n");
	  exit(0);
	}
	
	if(count >= 3) {
	  JObject *obj = val.object_val;
	  if(type == VAL_INT) {
	    printf("%s: %d\n", argv[2], jsonInt(obj, argv[2]));
	  }else if(type == VAL_STRING) {
	    printf("%s: %s\n", argv[2], jsonString(obj, argv[2]));
	  }else{
	    fprintf(stderr, "Error: Could not find key '%s'\n", argv[2]);
	  }
	}
	
	jsonFree((JItemValue)val, type);
	return EXIT_SUCCESS;
}
