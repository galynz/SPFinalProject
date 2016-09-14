#ifndef SPBPRIORITYQUEUE_H_
#define SPBPRIORITYQUEUE_H_
#include "SPListElement.h"
#include <stdbool.h>

#define NEG_VAL -1

/**
 * SP Bounded Priority Queue summary
 *
 * Implements a bounded priority queue, which works the same as a regular priority queue, except the fact that the number of elements in the queue is limited.
 * The elements of the queue are kept in a SPList object.
 * 
 *
 **/

/** type used to define Bounded priority queue **/
typedef struct sp_bp_queue_t* SPBPQueue;

/** type for error reporting **/
typedef enum sp_bp_queue_msg_t {
	SP_BPQUEUE_OUT_OF_MEMORY,
	SP_BPQUEUE_FULL,
	SP_BPQUEUE_EMPTY,
	SP_BPQUEUE_INVALID_ARGUMENT,
	SP_BPQUEUE_SUCCESS
} SP_BPQUEUE_MSG;

/**
 * This function creates a new empty SPBPriorityQueue.
 *
 * @param maxSize - queue's maximum capacity
 * @return
 *  NULL - If maxSize<0 or allocation failed.
 *  A new SPBPriorityQueue else.
 */
SPBPQueue spBPQueueCreate(int maxSize);

/**
 * Creates a copy of source queue.
 * 
 * @param source - SPBPriorityQueue to copy.
 * @return
 *  NULL - If source is NULL or allocation failed or SPList creation failed.
 *  A new SPBPQueue else.
 */
SPBPQueue spBPQueueCopy(SPBPQueue source);

/**
 * Deallocates a  queue.
 * @param source - the queue we want to destroy.
 * If source is NULL, does nothing.
 */
void spBPQueueDestroy(SPBPQueue source);

/**
 * Removes all the elements in the queue.
 * @param source - the queue we want to clear.
 * If source is NULL or the source->list is NULL, does nothing.
 */
void spBPQueueClear(SPBPQueue source);

/**
 * Returns the number of elements in the queue.
 * @param source - the queue whose number of elements we wish to count.
 * @return 
 * if source is NULL, returns -1
 * else, returns queue's size
 */
int spBPQueueSize(SPBPQueue source);

/**
 * Returns the maximum capacity of the queue.
 * @param source - the queue whoes maximum capacity we want to check.
 * @return 
 * if source is NULL, returns -1
 * else, returns queue's max size
 */
int spBPQueueGetMaxSize(SPBPQueue source);

/**
 * Inserts a copy of a given element to the queue.
 * @param source - a queue to insert the element into.
 * @param elemet - the element we want to copy and insert into the queue.
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT if source or element are NULL
 * SP_BPQUEUE_OUT_OF_MEMORY if an allocation failed
 * SP_BPQUEUE_FULL if the number of elements in the queue equals maxSize
 * SP_BPQUEUE_SUCCESS the element has been inserted successfully
 */
SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue source, SPListElement element);

/**
 * Removes the element with the lowest value from the queue.
 * @param source - a queue we want to remove an element from.
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT if source is NULL
 * SP_BPQUEUE_EMPTY if there are no elements in the queue
 * SP_BPQUEUE_SUCCESS if dequeue was a success
 */
SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue source);

/**
 * Returns a new copy of the element with the lowest value.
 * @param source - the queue from which we want to get the lowest element
 * @return
 * if source is NULL or the queue is empty, returns NULL
 * else, returns a SPListElement object
 */
SPListElement spBPQueuePeek(SPBPQueue source);

/**
 * Returns a new copy of the element with the highest value.
 * @param source - the queue from which we want to get the highest element
 * @return
 * if source is NULL or the queue is empty, returns NULL
 * else, returns a SPListElement object
 */
SPListElement spBPQueuePeekLast(SPBPQueue source);

/**
 * Returns the minimal value in the queue.
 * @param source - the queue from which we want to get the minimal value
 * @return
 * -1 if source or the first element in source are NULL
 * else, returns the value of the mainimal element in the queue
 */
double spBPQueueMinValue(SPBPQueue source);

/**
 * Returns the maximal value in the queue.
 * @param source - the queue from which we want to get the maximal value
 * @return 
 * -1 if source or the last element in source are NULL
 * else, returns the value of the maximal element in the queue
 */
double spBPQueueMaxValue(SPBPQueue source);

/**
 * Returns true if the queue is empty.
 * @param source - the queue we want to check
 * @assert source != NULL
 */
bool spBPQueueIsEmpty(SPBPQueue source);

/**
 * Returns true if the queue is full.
 * @param source - the queue we want to check
 * @assert source != NULL
 */
bool spBPQueueIsFull(SPBPQueue source);

#endif
