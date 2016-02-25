/*------------------------------------------------------------------------------------
Program description	:	Implement replacements for the malloc/calloc, realloc and free system calls using "Buddy Memory Allocation".
Reference :- https://en.wikipedia.org/wiki/Buddy_memory_allocation
--------------------------------------------------------------------------------------*/

#include <stdio.h>	/* for standard io */
#include <unistd.h>	/* for brk() and sbrk() */
#include <string.h>	/* for memcpy and memset */
#include <math.h>	/* for pow() */

/* structure of block to be allocated */
typedef struct memory{
	int isFree;	/* block is free or allocated */
	int order;	/* order of the block */
	size_t size;	/* size of data stored in the block */
	struct memory *next;	/* address of next block */
	struct memory *prev;	/* address of prev block */
}mem;

/* first represents address of first memory chunk. initially no dynamic memory is allocated */
mem *first = NULL;

/*
<-------------------------------------------- 1024*1024 -------------------------------------------->
 ____________________________________________________________________________________________________
|__________|__________|__________|__________|__________|____________________________|________________|
 <--block-->
max #of blocks in a memory chunk are (1024*1024)/256 = 2^12...Hence maxOrder is 12.

*/

#define chunkSize 1024
#define blockSize 64
#define maxOrder 4 
#define structSize 40

/* find the order of the memory block. */
int findOrderOfRequiredBlock(size_t size){
	int orderOfBlock = 0, i = blockSize;
	while (i <= size){
	    i += i;
	    orderOfBlock++;
	}
	return orderOfBlock;
}


/* allocate new memory chunk of size "chunkSize" */
void* allocateNewMemoryChunk (){
	mem *block = NULL;
	/* Find starting address of uninitialized data segment.*/
	block = sbrk(0); 
	
	if (sbrk(chunkSize) == (void *) -1){
		return NULL;
	}
	block->order = maxOrder;
	block->next = NULL;
	block->prev = NULL;
	block->size = 0;
	block->isFree = 1;
	return block;
}

/* find block with sufficient space */
void * findMemoryBlock(int orderOfBlockRequired){
	mem *chunk;
	if (first == NULL){
	    chunk = allocateNewMemoryChunk ();
		if (chunk == NULL){
		    printf (" Memory is not available in the system \n ");
			return NULL;
		}
		first = chunk;
		return chunk;
	}
	chunk = first;
	while ((chunk->order < orderOfBlockRequired || chunk->isFree == 0) && chunk->next != NULL){
	    chunk = chunk->next;
	
	}
	if (chunk->next == NULL && (chunk->order < orderOfBlockRequired || chunk->isFree == 0)){
		mem *temp = allocateNewMemoryChunk();
		if (temp == NULL){
			printf (" Memory is not available in the system \n ");
			return NULL;
		}
		temp->prev = chunk;
		chunk->next = temp;
		return temp;
	}
	return chunk;
}

/* if chunk is of bigger size ie order of chunk is greater than required, split the chunk in 2 halves  */
void* splitChunk (int orderOfBlockRequired, void* chunk){
	mem *temp, *ptr = chunk;
	int k;
	while ( ptr->order != orderOfBlockRequired ){
		k = pow(2, ptr->order - 1);		
		temp = chunk + blockSize * k;
		if (ptr->next != NULL)
			ptr->next->prev = temp;
		temp->next = ptr->next;
		temp->order = ptr->order - 1;
		temp->prev = ptr;
		temp->isFree = 1;
		temp->size = 0;
		ptr->next = temp;
		ptr->order -= 1;
	}
	return ptr;
}

/* allocate memory */
void* myMalloc (size_t sizeOfMemory){
	if (sizeOfMemory == 0){
		return NULL;
	}

	else if (sizeOfMemory > chunkSize - structSize){
		printf ("We can't allocate memory greater then %d bytes\n", chunkSize);
		return NULL;
	}
	int sizeOfData = sizeOfMemory;
	
	/* add the size of structure */
	sizeOfMemory += structSize;
	
	int orderOfBlockRequired = findOrderOfRequiredBlock (sizeOfMemory);
	
	/* check the availability of free  space in memory chunk */
	mem* chunk = findMemoryBlock (orderOfBlockRequired);
	if (chunk->order > orderOfBlockRequired)
		chunk = splitChunk (orderOfBlockRequired, chunk);
	chunk->size = sizeOfData;
	chunk->isFree = 0;
	
	return chunk+(structSize/sizeof(mem));

}/* end of malloc */


/* calloc function */
void* myCalloc(int size, size_t dataType){
	int actualSize = size * dataType;
	if (actualSize / size != dataType){
		/*Multiplication overflow*/
		return NULL;
	}
	
	else{
		mem * chunk = myMalloc(actualSize);
		memset(chunk, 0, actualSize);
		return chunk;
	}	
}/* end of calloc */

/* free the allocated memory */
void myFree(void* block){
	if (block == NULL)
		return ;
	
	/* go to starting address of the block ie starting address of structure. */
	mem* temp = block - structSize;
		
	if (temp->size == 0 && temp->isFree == 1){
		printf ("Memory can't be freed\n");
		return ;
	}
	
	/* if whole memory is freed then update first chunk address */
	if (temp->order == maxOrder){
		if (temp->prev != NULL)
			temp->prev->next = temp->next;
		if (temp->next != NULL)
			temp->next->prev = temp->prev;
		temp = NULL;
		return ;
	}
	
	if (temp->isFree == 0){
		temp->size = 0;
		temp->isFree = 1;
		
		/* condition for right one is buddy */
		void* base = first;
		int k = pow(2, temp->order + 1);
		
		if ( (block - structSize - base) % (blockSize * k) == 0)
			
			/* check whether buddy is free or not */
			if ((temp->next != NULL && temp->next->isFree == 1 && temp->next->order == temp->order)){
				temp->order++;
				mem * ptr = temp->next;
				temp->next = ptr->next;
				if(temp->next != NULL)
					temp->next->prev = temp;
				ptr->next = NULL;
				ptr->prev = NULL;
				temp->isFree = 0;
				temp->size = 1;
				void * k = temp;
				myFree (k + structSize);
			}
		
		/* else left one is buddy */
		else {
			
			/* check whether buddy is free or not */
			if (temp->prev != NULL && temp->prev->isFree == 1 && temp->prev->order == temp->order) {
				temp->isFree = 1;
				temp->prev->isFree = 0;
				void * k = temp->prev ;
				/* merge the left block with it by calling free on left block */
				myFree (k + structSize);
			}
		}
	}
}

/* reallocate the memory with the new size */
void* myRealloc (void *block , size_t newSize){
	/* if given memory is NULL, realloc works same as malloc */
	if (block == NULL)
		return myMalloc (newSize);
		
	/* if the given size is zero and given memory is non-null then free the block */
	if (newSize == 0){
		myFree (block);
		return NULL;
	}
	mem * temp = block - structSize;
	
	if (newSize <= temp->size){
		int orderOfBlockRequired = findOrderOfRequiredBlock (newSize);
		
		mem* newBlock = splitChunk (orderOfBlockRequired, block - structSize);
		newBlock->size = newSize;
		newBlock->isFree = 0;
		return newBlock+structSize;
	}
	else {
		mem * newBlock = myMalloc (newSize);
		memcpy (newBlock, block, temp->size);
		myFree (block);
		return newBlock+structSize;
	}
}
