#include "vec.h"
void createNode(VEC* topvec, VEC* endvec, void *data)
{
	endvec = pushtailNode(endvec, data);
	//printf("good2\n");
	topvec = endvec;
	
	//printf("good3\n");
}
VEC* pushtailNode(VEC* endvec, void *data)
{
	VEC *tmpNode = (VEC*)malloc(sizeof(VEC));
	tmpNode->data = (void*)data;
	tmpNode->nextptr = nullptr;
	if (endvec!=nullptr)
	{
		endvec->nextptr = tmpNode;
		tmpNode->preptr = endvec;
		endvec = endvec->nextptr;
	}
	else
	{
		endvec = tmpNode;
		endvec->preptr = nullptr;
	}
	//printf("good1\n");
	return endvec;
}

VEC* popheadNode(VEC* topvec,void **data)
{

	if (topvec == nullptr)
	{
		*data = nullptr;
		return topvec;
	}

	VEC *tmpNode = topvec;
	*data = topvec->data;
	if (*data == nullptr)
		printf("bad\n");

	topvec = topvec->nextptr;
	free(tmpNode);
	tmpNode = nullptr;

	if(topvec!=nullptr)
		topvec->preptr = nullptr;

	return topvec;
}
void emptyNode(VEC* topvec, VEC* endvec,void **data)
{
	int i = 0;
	while (endvec != nullptr)
	{
		//void *pdata = nullptr;
		topvec = popheadNode(topvec,(void**)data);
		int *gdata = (int*)(*data);
		//printf("%d\n", gdata[0]);
		free(*data);
		*data = nullptr;
		
	}
	endvec = nullptr;

}

VEC* getnextnode(VEC* currentvec)
{
	if (currentvec == nullptr)
	{
		return currentvec;
	}
	else
	{
		return currentvec->nextptr;
	}
}
VEC* getforenode(VEC* currentvec)
{
	if (currentvec == nullptr)
	{
		return currentvec;
	}
	else
	{
		return currentvec->preptr;
	}
}

void getcurrentdata(VEC* currentvec, void **data)
{
	if (currentvec == nullptr)
	{
		*data = nullptr;
		return ;
	}
	*data = currentvec->data;
	return;
}