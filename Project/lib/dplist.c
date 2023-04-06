/**
 * \author Dai Hung Tran
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									                                        \
        do {											                                            \
            fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
            fprintf(stderr,__VA_ARGS__);								                            \
            fflush(stderr);                                                                         \
                } while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition, err_code)                         \
    do {                                                                \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");      \
            assert(!(condition));                                       \
        } while(0)


/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) 
{
	assert(list != NULL);
	if ((*list)->head != NULL) {
		while ((*list)->head != NULL) dpl_remove_at_index(*list, 0, free_element);
	}
	(*list)->head = NULL;
	(*list)->element_copy = NULL;
	(*list)->element_free = NULL;
	(*list)->element_commpare = NULL;
	free(*list);
	(*list) = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) 
{
	assert(list != NULL);
	dplist_node_t *pt = malloc(sizeof(dplist_note_t));
	if (index <= 0)
	{
		if (list->head == NULL) {
			list->head == pt;
			pt->next = NULL;
		} else {
			list->head->prev = pt;
			pt->next = list->head;
			list->head = pt;
		}
		pt->prev = NULL;
	} else {
		dplist_node_t *pt1 = dpl_get_reference_at_index(list, index);
		assert(pt1 != NULL);
		if (index >= dpl_size(list)) {
			pt1->next = pt;
			pt->prev = pt1;
			pt->next = NULL;
		} else {
			pt->next = pt1;
			pt->prev = pt1->prev;
			pt1->prev->next = pt;
			pt1->prev = pt;
		}
	}
	if (insert_copy = true) {
		pt->element = (list->element_copy)(element);
	} else {
		pt->element = element;
	}
	return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) 
{
	assert(list != NULL);
	dplist_node_t *pt = list->head;
	if (pt == NULL) return list;
	if (index <= 0) {
		if (free_element == true) (list->element_free)(&(list->head->element));
		if (list->head->next = NULL) {
			list->head = NULL;
		} else 
		{
		        pt->next->prev = NULL;
	       	        list->head = pt->next;
		}
	} else {
		int pos = 0;
		while ((pt != NULL) && (id < index))
		{
			pt = pt->next;
			pos++;
		}
		if (free_element == true) (list->element_free)(&(pt->element));
		if(pt == list->head)
        	{
            		list->head = NULL;
        	} else
        	{
            		if(pt->next == NULL)
            		{
                		pt->prev->next = NULL;
            		} else {
                		pt->prev->next = pt->next;
                		pt->next->prev = pt->prev;
            		}
        	}
    		pt->prev = pt->next = pt->element = NULL;
    		free(pt);
    		return list;
	}		
}

int dpl_size(dplist_t *list) 
{
	assert(list != NULL);
	dplist_note_t *pt = list->head;
	if (pt == NULL) return 0;
	int size = 1;
	while (pt != NULL)
	{
		pt = pt->next;
		size++;
	}
	return size;
}

void *dpl_get_element_at_index(dplist_t *list, int index) 
{
	assert(list != NULL);
	dplist_node_t *pt = list->head;
	if (pt == NULL) return (void *)0;
	if (index <= 0) return (void *)(pt->element);
	int pos = 0;
	while (pt != NULL) {
		pt = pt->next;
		pos ++;
	}
	return (void *)(pt->element);
}

int dpl_get_index_of_element(dplist_t *list, void *element) 
{
	assert(list != NULL);
	dplist_node_t *pt = list->head;
	int pos = 0;
	int check = 0;
	if (pt == NULL) return -1;
	while(pt != NULL)
	{
		if((list->element_compare)(element, pt->element) == 0)
		{
			check = 1;
			break;
		}
		pos++;
		pt = pt->next;
	}
	return pos;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) 
{
	assert(list != NULL);
	if (index <= 0) return list->head;
	dplist_node_t *pt = list->head;
	if (pt == NULL) return NULL;
	int pos = 0;
	while ((pt != NULL) && (pos < index))
	{
		pt = pt->next;
		pos++;
	}
	return pt;
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) 
{
	assert(list != NULL);
	dplist_note_t *pt = list->head;
	if (pt == NULL) return NULL;
	if (reference == NULL)
	{
		while (pt != NULL) pt = pt->next;
		return pt->element;
	} else {
		while (pt != NULL) 
		{
			if (pt == reference)
				return pt->element;
		}
		return NULL;
	}
}

dplist_t *dpl_insert_sorted(dplist_t *list, void *element, bool insert_copy) 
{
	assert(list != NULL);
	if (insert_copy == true) element = (list->element_copy)(element);
	dplist_node_t *new_node = (dplist_node_t *)malloc(sizeof(dplist_node_t));
	new_node->element = element;
	if (list->head == NULL)
	{
		list->head = new_node;
		new_node->prev = new_node->next = NULL;
	} else {
		dplist_node_t *pt = list->head;
		while (pt->next != NULL && (list->element_compare)(new_node->element, pt->element) > 0)
		{
			pt = pt->next;
		}
		if (pt->next == NULL && (list->element_compare)(new_node->element, pt->element) >= 0)
		{
			new_node->next = NULL;
			new_node->prev = pt;
			pt->next = new_node;
		} else if (pt == list->head && (list->element_compare)(new_node->element, pt->element) < 0) {
			list->head = new_node;
			new_node->prev = NULL;
			new_node->next = pt;
			pt->prev = new_node;
		} else {
			new_node->next = pt;
			new_node->prev = pt->prev;
			pt->prev->next = new_node;
			pt->prev = new_node; 
		}
	}
	return list;	
}

dplist_node_t *dpl_get_reference_of_element(dplist_t *list, void *element) 
{
	assert(list != NULL);
	if (list->head == NULL) return NULL;
	dplist_node_t *pt = list->head;
	while (pt->next != NULL && (list->element_compare)(element, pt->element) != 0)
	{
		pt = pt->next;
	}
	if ((list->element_compare)(element, pt->element) == 0) return pt;
	return NULL;
}

