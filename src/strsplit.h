/**
 * File              : strsplit.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 21.02.2022
 * Last Modified Date: 20.12.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/* 
 * Get array from string separated by delims
 */

#ifndef STRSPLIT_H__
#define STRSPLIT_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <string.h>
/* strsplit
 * split string with delims, fill allocated NULL-terminated 
 * array of allocated tokens and return number of tokens
 * %str    - string to split
 * %delim  - string with delim chars
 * %tokens - pointer to tokens array
 */
static int strsplit(
		const char *str, const char *delim, char ***tokens)
{
	int i;
	char *s, *t, **arr = (char **)malloc(sizeof(char*));
	if (!arr)
		return 0;
	
	// do safe strtok
	s = strdup(str);
	if (!s)
		return 0;

	// loop through the string to extract 
	// all tokens
	for(t=strtok(s, delim), i=0; 
			t; 
			t=strtok(NULL, delim), ++i) 
	{
		arr = 
			(char **)realloc(arr, (i+2)*sizeof(char*));
		arr[i] = strdup(t);
	}

	arr[i] = NULL;
	if (tokens)
		*tokens = arr;
	else
		free(arr);

	free(s);
	
	return i;
}

static void strsplit_free(char **tokens)
{
	char **ptr = tokens;
	while (*ptr)
		free(*ptr++);
	free(tokens);
}

#ifdef __cplusplus
}
#endif

#endif //STRSPLIT_H__
