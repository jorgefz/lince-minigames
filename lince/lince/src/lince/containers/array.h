#ifndef ARRAY_H
#define ARRAY_H

#include <inttypes.h>
#include <stddef.h>

/** @struct array_t
* @brief Data structure with dynamic contiguous storage of generic data.
*/
typedef struct array_container {
	void *data;				///< Main memory pool
	uint32_t size;			///< Number of stored elements
	uint32_t capacity;		///< Max number of elements allocated
	uint32_t element_size;	///< Size in bytes of an element
} array_t;

/** @brief Initialises an array via a given pointer.
*   Allows the user to manage the memory of the struct itself.
*   The memory pool will still be stored on the heap.
*   Should be freed with `array_uninit`.
*	@param array the return array.
*	@param element_size size in bytes of an array element.
*/
void array_init(array_t* array, uint32_t element_size);

/** @brief Resets the array state and frees the memory pool.
*	@param array the array to uninitialise.
*/
void array_uninit(array_t* array);

/** @brief Returns a pointer to a new array allocated on the heap
*   Should be later freed with `array_destroy`.
*	@param element_size size in bytes of an array element.
*/
array_t* array_create(uint32_t element_size);

/** @brief Frees an allocated array
*	@param array the array to deallocate.
*/
void array_destroy(array_t* array); // should also free array_t itself

/** @brief Duplicates an array, allocating the new copy on the heap.
*	@param array the array to copy.
*/
array_t* array_copy(array_t* orig);

/* Initialises an array from existing data
If a size of zero or empty data are provided, no elements are added to the array.
*/
// array_t* array_from_data(void* data, uint32_t size, uint32_t element_size);

/** @brief Pre-allocates a given number of elements.
* The new elements are not initialised.
* Set the value of these elements with `array_set`.
* If the size of an array is reduced, the extra elements are removed.
* @param array Array to resize.
* @param size New size of the array.
*/
array_t* array_resize(array_t* array, uint32_t size);

/** @brief Sets the value of an element.
* Any previously data contained in the element is overwritten.
* If the given pointer to data is NULL, the specified element is filled with zeros.
* Returns a pointer to the element in the array.
* @param array Array for which to modify an element
* @param data Memory which will overwrite the element in question.
* @param index Which element to modify (starting from zero).
*/
void* array_set(array_t* array, void* data, uint32_t index);

/** @brief Returns a pointer to the element at the given index.
* Returns NULL if the index is invalid.
* @param array Array from which to retrieve an element.
* @param index Which element to retrieve.
*/
void* array_get(array_t* array, uint32_t index);

/** @brief Returns a pointer to the first element */
void* array_front(array_t* array);

/** @brief Returns a pointer to the last element */
void* array_back(array_t* array);

/** @brief Returns a pointer to first byte after the end of the array
* Useful for C++ style (pointer-based) iteration.
*/
void* array_end(array_t* array);

/** @brief Inserts an element at the given index.
* If the given element is NULL, the inserted element is zeroed.
* @param array Array object
* @param element Value to insert
* @param index Location where to insert the element.
* 	A previous element at this index is displaced one position forward.
*/
array_t* array_insert(array_t* array, void* element, uint32_t index);

/** @brief Inserts an element at the end of the array */
array_t* array_push_back(array_t* array, void* element);

/** @brief Inserts an element at the beginning of the array */
array_t* array_push_front(array_t* array, void* element);


/** @brief Removes the element at the given index.
* Note that, whilst the size of the array is reduced, its capacity is not.
*/
array_t* array_remove(array_t* array, uint32_t index);

/** @brief Removes the last element of the array */
array_t* array_pop_back(array_t* array);

/** @brief Removes the element first element of the array */
array_t* array_pop_front(array_t* array);

/** @brief Removes all elements on the array */
array_t* array_clear(array_t* array);

#endif /* ARRAY_H */