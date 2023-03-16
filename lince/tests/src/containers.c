#include "tests.h"
#include "test.h"
#include "lince/containers/array.h"
#include "lince/containers/hashmap.h"
#include "lince/containers/linkedlist.h"

#define array_foreach(T, element, array) \
for(T element = array->data; element != array_end(array); element++)


void scan_array(array_t* a){
	if(!a) return;
	printf(" array of size %u, capacity %u, and element size %u\n",
		a->size, a->capacity, a->element_size);
	for(uint32_t i = 0; i != a->size; ++i){
		char* p = (char*)a->data + i * a->element_size;
		printf("[%u] %d at %p\n", i, *(int*)p, p);
	}
}

int test_array(){
	array_t *r = NULL;

	// Create with element size 0
	// Should fail all attempts to insert or manipulate via provided functions
	array_t z = array_create(0);
	r = array_resize(&z, 10);
	TEST_ASSERT(!r, "Array of element size zero was able to be resized");

	r = array_insert(&z, (void*)5, 0);
	TEST_ASSERT(!r, "Array of element size zero could insert an element");

	// Create with ints
	array_t a = array_create(sizeof(int));
	TEST_ASSERT((a.size==0) && (a.capacity==0) && (!a.data),
		"Failed to create array with elements of size int");

	// Resize
	r = array_resize(&a, 10);
	TEST_ASSERT(r, "Failed to resize array of ints from size 0 to 10");
	TEST_ASSERT((a.size==10) && (a.capacity==16) && (a.data),
		"Failed to resize array of ints from size 0 to 10");
	
	// Setting elements
	int res = 1;
	for(int i = 0; i != (int)a.size; ++i){
		res = res && array_set(&a, &i, i);
	}
	TEST_ASSERT(res, "Failed to set array elements incrementally from 0 to 10");

	// Getting elements
	res = 1;
	for(int i = 0; i != (int)a.size; ++i){
		res = res && (*(int*)array_get(&a, i) == i);
	}
	TEST_ASSERT(res, "Failed to retrieve items, or retrieved item is wrong")

	res = (*(int*)array_front(&a)==0) && (*(int*)array_back(&a)==9);
	TEST_ASSERT(res, "Wrong front or back value");

	// Case: value passed is NULL
	r = array_set(&a, NULL, 1);
	res = (r) && (*(int*)array_get(&a,1) == 0);
	TEST_ASSERT(res, "Set value to NULL does not zero the value");
	
	// Limiting case: index out of bounds
	r = array_set(&a, NULL, a.size);
	TEST_ASSERT(!r, "Setting out-of-bounds value does not return NULL");

	// Inserting a value
	int prev = *(int*)array_get(&a, 5);
	int value = 99;
	r = array_insert(&a, &value, 5);

	res = r && (a.size == 11);
	res = res && (*(int*)array_get(&a,5)==value) && (*(int*)array_get(&a,6)==prev);
	TEST_ASSERT(res, "Failed to insert value");
	
	// Removing a value
	r = array_remove(&a, 1);
	r = array_pop_back(&a);
	r = array_pop_front(&a);

	res = r && (a.size == 8)
			&& (*(int*)array_front(&a)==2)
			&& (*(int*)array_back(&a)==8);
	TEST_ASSERT(res, "Failed to pop first or last value");

	// array_clear
	r = array_clear(&a);
	TEST_ASSERT(r && a.size==0, "Failed to clear array");

	array_destroy(&a);
	TEST_ASSERT(a.size==0 && a.capacity==0 && a.data==NULL && a.element_size==0,
		"Failed to destroy array");
	
	return TEST_PASS;
}

void scan_hashmap(hashmap_t* map){
	printf("\n Hashmap size %u and %u entries\n", map->size, map->entries);
	for(uint32_t i = 0; i != map->size; ++i){

		hm_entry_t* entry = map->table[i];
		if(!entry) continue;

		printf("%u) ", i);
		while(entry){
			if(entry->key) printf("<%s>, ", entry->key);
			else printf("<NULL>, ");
			entry = entry->next;
		}
		printf("\n");
	}
}


int test_hashmap(){

	hashmap_t map = hashmap_create(5);
	TEST_ASSERT(map.table && map.size==7, "Failed to create hashmap");
	
	int r = 1, x = 10, y = 20, z = 30;
	r = r && hashmap_set(&map, "x", &x);
	r = r && hashmap_set(&map, "y", &y);
	r = r && hashmap_set(&map, "z", &z);
	TEST_ASSERT(r, "Failed to insert values to hashmap");
	
	int *rx, *ry, *rz, *none;
	rx = hashmap_get(&map, "x");
	ry = hashmap_get(&map, "y");
	rz = hashmap_get(&map, "z");
	none = hashmap_get(&map, "n");
	
	TEST_ASSERT(rx && ry && rz && !none, "Failed to retrieve values from hashmap");
	TEST_ASSERT(*rx == x && *ry == y && *rz == z && map.entries == 3,
		"Failed to retrieve values from hashmap");
	
	char* key = NULL;
	int key_count = 0;
	do{
		key = hashmap_iter_keys(&map, key);
		key_count++;
	} while(key);
	key_count--;
	TEST_ASSERT(key_count==3, "Key iteration failed, unexpected number of keys");

	// scan_hashmap(&map);
	hashmap_free(&map);
	return TEST_PASS;
}

int test_hashmap_large(){

	hashmap_t map = hashmap_create(5);
	int r = 1;

	TEST_CLOCK_START(time);
	int x = 99;
	for(int i = '!'; i != '~'+1; ++i){
		for(int j = '!'; j != '~'+1; ++j){
			char str[] = {i,j,'\0'};
			r = r && hashmap_set(&map, str, &x);
		}
	}
	long int n_op = ('~' - '!') * ('~' - '!');
	TEST_CLOCK_END(time, n_op);
	TEST_ASSERT(r, "Failed to add item to hashmap");

	// scan_hashmap(&map);
	hashmap_free(&map);
	return TEST_PASS;
}



void containers_test(){
	struct test_t tests[] = {
		{.fn = test_array,   .name = "test_array"},

		{.fn = test_hashmap,        .name = "test_hashmap"},
		{.fn = test_hashmap_large,  .name = "test_hashmap_large"},
	};
	uint32_t count = sizeof(tests) / sizeof(struct test_t);

	run_tests(tests, count, "containers");
}

