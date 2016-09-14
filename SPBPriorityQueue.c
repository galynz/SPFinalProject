#include "SPBPriorityQueue.h"
#include "SPList.h"
#include <assert.h>
#include <stdio.h>
#include <malloc.h>


struct sp_bp_queue_t {
    SPList list;
    int maxSize;
};

SPBPQueue spBPQueueCreate(int maxSize){
	SPBPQueue queue = NULL;
    // Validating max size
    if (maxSize < 0){
        return NULL;
    }
    // Allocating queue
    queue = (SPBPQueue)malloc(sizeof(struct sp_bp_queue_t));
    if (queue == NULL){
        return NULL;
    }
    queue->list = spListCreate();
    // If list creation failed, free previous allocations
    if (queue->list == NULL){
        free(queue);
        return NULL;
    }
    queue->maxSize = maxSize;
    return queue;
}

SPBPQueue spBPQueueCopy(SPBPQueue source){
	SPBPQueue queue = NULL;
    if (source == NULL){
        return NULL;
    }
    // Allocating queue
    queue = (SPBPQueue)malloc(sizeof(struct sp_bp_queue_t));
    if (queue == NULL){
        return NULL;
    }
    queue->list = spListCopy(source->list);
    // If list creation failed, free previous allocations
    if (queue->list == NULL){
        free(queue);
        return NULL;
    }
    queue->maxSize = source->maxSize;
    return queue;
}

void spBPQueueDestroy(SPBPQueue source){
    if (source == NULL){
        return;
    }
    // If the list isn't null, we need to destroy it before freeing the allocated memory
    if (source->list != NULL){
        spListDestroy(source->list);
    }
    free(source);
}

void spBPQueueClear(SPBPQueue source){
    if (source == NULL) {
        return;
    }
    if (source->list == NULL) {
        return;
    }
    // Using SPList function to clear the queue
    spListClear(source->list);
}
    
int spBPQueueSize(SPBPQueue source) {
    if (source == NULL){
		return NEG_VAL;
	}
    // Using SPList function to get the size
    return spListGetSize(source->list);
}

int spBPQueueGetMaxSize(SPBPQueue source) {
    if (source == NULL){
		return NEG_VAL;
	}
    return source->maxSize;
}

SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue source, SPListElement element) {
	int cur_size = 0;
	SPListElement cur = NULL;
	// Verifying arguments
    if (source == NULL || element == NULL) {
        return SP_BPQUEUE_INVALID_ARGUMENT;
    }
    
    cur_size = spListGetSize(source->list);
    
    // Find insert place
    cur = spListGetFirst(source->list);
    while (cur != NULL && spListElementGetValue(cur) < spListElementGetValue(element)){
        cur = spListGetNext(source->list);
    }
    
    // Insert at the place we found
    if (cur == NULL){
        // handling of the last element is different
        // if we are the biggest value & queue is full, we don't insert
        if (cur_size >= source->maxSize) {
            return SP_BPQUEUE_FULL;
        }
        
        // otherwise, we insert at the last place
        if (SP_LIST_OUT_OF_MEMORY == spListInsertLast(source->list, element)) {
            // in case of memory error - destroy the element
            return SP_BPQUEUE_OUT_OF_MEMORY;
        }
    } else {
        if (SP_LIST_OUT_OF_MEMORY == spListInsertBeforeCurrent(source->list, element)) {
            return SP_BPQUEUE_OUT_OF_MEMORY;
        }
    }
    
    cur_size++; // update queue size after insert
    
    // If the queue is over the limit - remove last element
    
    if (cur_size > source->maxSize) {
        
        spListGetFirst(source->list);
        
        // do get next (cur_size-1) times, to get right before the last
        while (cur_size > 1) {
            spListGetNext(source->list);
            cur_size--;
        }
        
        // remove
        spListRemoveCurrent(source->list);
    }
    
    return SP_BPQUEUE_SUCCESS;
}

SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue source){
    SPListElement cur = NULL;
    if (source == NULL){
        return SP_BPQUEUE_INVALID_ARGUMENT;
    }
    // Go to the first element
    cur = spListGetFirst(source->list);
    if (cur == NULL){
        return SP_BPQUEUE_EMPTY;
    }
    // remove it
    spListRemoveCurrent(source->list);
    return SP_BPQUEUE_SUCCESS;
}

SPListElement spBPQueuePeek(SPBPQueue source){
	SPListElement cur = NULL;
    if (source == NULL){
        return NULL;
    }
    // Go to the first element
    cur = spListGetFirst(source->list);
    if (cur == NULL){
        return NULL;
    }
    
    // Copy the element
    return spListElementCopy(cur);
}

SPListElement spBPQueuePeekLast(SPBPQueue source){
       SPListElement prev = NULL;
	   SPListElement cur = NULL;
	   if (source == NULL){
           return NULL;
       }
       // Getting the first element and saving it to cur
       cur = spListGetFirst(source->list);
       if (cur == NULL){
           return NULL;
       }
       
       // Finding the last element (will be saved in prev)
       while (cur != NULL){
           prev = cur;
           cur = spListGetNext(source->list);
       }
       return spListElementCopy(prev);
}

double spBPQueueMinValue(SPBPQueue source){
    SPListElement first = NULL;
	double value = 0;
	if (source == NULL){
		return NEG_VAL;
	}
    // Getting the first element
    first = spBPQueuePeek(source);
	if (first == NULL){
		return NEG_VAL;
	}
    // Getting the element's value
    value = spListElementGetValue(first);
    // Destroying the element
    spListElementDestroy(first);
    return value;
}

double spBPQueueMaxValue(SPBPQueue source){
	SPListElement last = NULL;
	double value = 0;
    if (source == NULL){
		return NEG_VAL;
	}
    // Getting the last element
    last = spBPQueuePeekLast(source);
    if (last == NULL){
		return NEG_VAL;
	}
    // Getting the element's value
    value = spListElementGetValue(last);
    // Destroying the element
    spListElementDestroy(last);
    return value;
}

bool spBPQueueIsEmpty(SPBPQueue source){
    assert(source != NULL);
    return spListGetSize(source->list)==0;    
}

bool spBPQueueIsFull(SPBPQueue source){
    assert(source != NULL);
    return spListGetSize(source->list)==source->maxSize;
}
