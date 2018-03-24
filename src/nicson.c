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
	JObject *obj = jsonParse(argv[1]);

	jsonAddKey(obj, "message", jsonStringValue("Success."));
	jsonAddKey(obj, "status", jsonIntValue(200));

	printf("Object key size now: %d\n", obj->size);

	JObject *nobj = jsonObject(obj, "nick.me");

	if(nobj == 0) {
	  printf("Could not find the obj\n");
	}
	return EXIT_SUCCESS;
}
