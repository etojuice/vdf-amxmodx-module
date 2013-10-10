#ifndef __VDFCOMMON_H__
#define __VDFCOMMON_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 1024
#define MAX_OPEN_FORWARDS 8
#define MAX_PARSE_FORWARDS 8

typedef unsigned int UINT;

/*
 *  Macro for vdf indentation
 */
#define TABFILL(tabStr, tabNum)\
	memset(tabStr, '\t', tabNum);\
	tabStr[tabNum] = '\0';\


template <typename T>
void Finalize(T &pTarget)
{
	if(pTarget == NULL)
		return;

	delete (T*) pTarget;
	pTarget = NULL;
}

template <typename T>
void FinalizeArray(T &pTarget)
{
	if(pTarget == NULL)
		return;

	delete [] pTarget;
	pTarget = NULL;
}

/**
 *	Adds a slot to a pointer array
 *	@param	target		target pointer Array
 *	@param	curLength	current size of pointer Array
 */
template <typename T>
void GrowPArray(T ***target, size_t &curLength)
{
	T **cache;

	cache = *target;

	*target = new T*[sizeof(T*) * curLength + 1];

	if(curLength)
	{
		memcpy(*target, cache, curLength * sizeof(T*));
		FinalizeArray(cache);
	}

	(*target)[curLength] = NULL;
}


void ToLowerCase(char *src, char *dest);


#endif //__VDFCOMMON_H__
