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
	printf("FNV Hash: %d\n", fnvstr("Hello"));
	JValue *val = jsonParse(argv[1]);

	if(!val && val->value_type != VAL_OBJ) {
	  jsonPrintError();
	  exit(1);
	}
	
	JObject *obj = (JObject*)val->value;
	puts("Adding keys...");
	printf("Message %d, status %d\n", fnvstr("message") % 100, fnvstr("status") % 100);
	jsonAddVal(obj, "message", jsonStringValue("Success."));
	jsonAddVal(obj, "status", jsonIntValue(200));

	printf("Object key size now: %d\n", obj->size);

	JObject *nobj = jsonObject(obj, "nick.me");

	if(nobj == 0) {
	  printf("Could not find the obj\n");
	}
	
	jsonFree(val);
	return EXIT_SUCCESS;
}
