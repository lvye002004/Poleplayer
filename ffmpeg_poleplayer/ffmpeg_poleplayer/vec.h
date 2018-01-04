//#pragma once
#ifndef VEC_H
#define VEC_H

#include <stdio.h>
#include<malloc.h>
struct vec
{
	void *data;
	struct vec *nextptr;
	struct vec *preptr;
};
typedef struct vec VEC;
void createNode(VEC* topvec, VEC* endvec, void *data);

void getcurrentdata(VEC* currentvec, void **data);
VEC* getnextnode(VEC* currentvec);
VEC* getforenode(VEC* currentvec);
void emptyNode(VEC* topvec, VEC* endvec, void **data);
VEC* pushtailNode(VEC* endvec, void *data);
VEC* popheadNode(VEC* topvec, void **data);

#endif