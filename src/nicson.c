/*
 ============================================================================
 Name        : nicson.c
 Author      : Nick
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "json.h"

#define SEARCH_TYPE_ARRAY         0
#define SEARCH_TYPE_ARRAY_RANGE   1
#define SEARCH_TYPE_WILDCARD      2
#define SEARCH_TYPE_MATCH         3
#define SEARCH_TYPE_KEY_MATCH     4

typedef enum SEARCH_TYPE {
  ARRAY = 0,
  ARRAY_RANGE,
  WILDCARD,
  MATCH,
  KEY_MATCH
} SEARCH_TYPE;

/**
 * Defines a single search action within the json loaded by
 * nicson.
 *
 * Examples:
 *
 *  Array Range: key.[0-5].*.key.*.*match.hello*world.end*.[4,6,1234].*hello*world*
 *
 */
typedef struct KeySearch {
  SEARCH_TYPE searchType : 4; // up to 16 types
  union meta {
    // makes this 4-8 bytes on 32/64 depending
    const char *matchStr;
    const char *keyStr;
    int arrayIndex;
    struct range {
      int32_t start;
      int32_t end;
    } range;
  } meta;
} KeySearch;

JItemValue query(JObject *obj, const char *key, int *type);

int main(int count, const char**argv) {
	if(count < 1) {
		puts("Error: missing filename argument!");
		return EXIT_FAILURE;
	}
	short fileArgNum = 1;
	short keyArgNum = 0;
	char wholeFilePrint = 1;
	char useStandardIn = 1;
	char findByArg = 0;
	char printHelpAndExit = 0;
	char interpKey = 0;

  if(argv[1][0] == '-') {
    //we have options
    if(argv[1][1] == 'p') {
      //pretty print the whole file
      fileArgNum = 2;
      wholeFilePrint = 1;
      useStandardIn = 0;
    }else if(argv[1][1] == 'e') {
      //find by argument
      fileArgNum = 2;
      wholeFilePrint = 0;
      findByArg = 1;
      useStandardIn = 0;
      keyArgNum = 3;
    }else if(argv[1][1] == 'E') {
      fileArgNum = 2;
      wholeFilePrint = 0;
      findByArg = 1;
      useStandardIn = 0;
      keyArgNum = 3;
      interpKey = 1;
    }else if(argv[1][1] == 'h') {
      printHelpAndExit = 1;
    }
  }else{
    fprintf(stderr, "Error: Options must come first\n");
    exit(0);
  }

	if(printHelpAndExit) {
	  printf("Usage: nicson [options] [file] [key]\n");
	  printf("Manipulate/Search JSON files\n\n");
	  printf("\t-p\t Pretty print either the JSON file or standard-in.\n");
	  printf("\t-e\t Search and find by key a value in the JSON file\n");
	  return 0;
	}

	const char* file = argv[fileArgNum];
	printf("Loading JSON: %s\n", file);
	short type;

	JItemValue val;
	if(useStandardIn) {
	  val = jsonParseF(stdin, &type);
	} else {
	  val= jsonParse(file, &type);
	}

#ifdef DEBUG
	if(stringCache) {
	  printf("Strings cached used %d\n", stringCache->size);
	}
#endif

	if(!val.ptr_val) {
	  fprintf(stderr, "Error Parsing file!\n");
	  exit(0);
	}

	if(wholeFilePrint) {
	  jsonPrintObject(stdout, val.object_val);
	}
	
	if(count >= 3 && (!wholeFilePrint || findByArg)) {
	  const char *key = argv[keyArgNum];
    JItemValue item;
    short extractType = 0;

	  if(interpKey) {
	    item = query(val.object_val, key, &extractType);
	  }else{
	    item = jsonGet(val.object_val, key, &extractType);
	  }

	  if(extractType == VAL_INT) {
	    printf("%s: %d\n", key, item.int_val);
	  }else if(extractType == VAL_FLOAT) {
	    printf("%s: %f\n", key, item.float_val);
	  }else if(extractType == VAL_DOUBLE) {
	    printf("%s: %f\n", key, item.double_val);
	  }else if(extractType == VAL_STRING) {
	    printf("%s: %s\n", key, item.string_val);
	  }else if(extractType == VAL_OBJ) {
	    jsonPrintObject(stdout, item.object_val);
	  }else if(extractType == VAL_BOOL_ARRAY || extractType == VAL_DOUBLE_ARRAY || extractType == VAL_FLOAT_ARRAY ||
	      extractType == VAL_INT_ARRAY || extractType == VAL_MIXED_ARRAY || extractType == VAL_STRING_ARRAY) {
	    jsonPrintEntry(stdout, extractType, &item, 3, 0);
	  }else{
	    fprintf(stderr, "Error: Could not find key '%s'\n", key);
	  }
	}
	
	jsonFree((JItemValue)val, type);
	return EXIT_SUCCESS;
}

JItemValue query(JObject *obj, const char *key, int *type) {
  JItemValue val;
  memset(&val, 0, sizeof(JItemValue));
  if(strlen(key) == 0) {
    return val; //short-circuit
  }

  if(key[0] == '[') {

  } else if(key[0] == '*') {
    return query(obj, &key[0], type);
  }
  return val;
}
