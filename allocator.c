//
//  COMP1927 Assignment 1 - Vlad: the memory allocator
//  allocator.c ... implementation
//
//  Created by Liam O'Connor on 18/07/12.
//  Modified by John Shepherd in August 2014, August 2015
//  Copyright (c) 2012-2015 UNSW. All rights reserved.
//

#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define HEADER_SIZE    sizeof(struct free_list_header)
#define MAGIC_FREE     0xDEADBEEF // Marks free blocks
#define MAGIC_ALLOC    0xBEEFDEAD // Marks allocated blocks

typedef unsigned char byte;
typedef u_int32_t vlink_t; // for links; indexes, not pointers.
typedef u_int32_t vsize_t; // for sizes of allocated/free chunks.
typedef u_int32_t vaddr_t; // for references; indexes, not pointers.
// u_int32_t is 32-bit unsigned value

typedef struct free_list_header {
   u_int32_t magic;  // ought to contain MAGIC_FREE
   vsize_t size;     // # bytes in this block (including header)
   vlink_t next;     // memory[] index of next free block
   vlink_t prev;     // memory[] index of previous free block
} free_header_t;

// Global data

static byte *memory = NULL;   // pointer to start of allocator memory
static vaddr_t free_list_ptr; // index in memory[] of first block in free list
static vsize_t memory_size;   // number of bytes malloc'd in memory[]

// Helper functions

u_int32_t power2(u_int32_t n){
   // ALTERNATIVE:
   // for (int i = 1; i <= 0xFFFFFFFF; i = i << 1){
   //    if ((n & i) != 0){ // if there is a 1
   //       ones++;
   //    }
   // }
   u_int32_t rounded;
   int ones = 0;
   u_int32_t i = 1 << 31;
   while (i > 0 && ones < 2){
      if ((n & i) != 0){ // if there is a 1
         ones++;
         rounded = i << 1;
      }
      i = i >> 1;
   }
   if (ones > 1){
      n = rounded;
   }
   return n;
}

void *itop(vlink_t i){ // converts indexes to pointers
   void *p;
   p = memory + i;
   return p;
}

vlink_t ptoi(void * p){ // converts pointer to indexes
   vlink_t i;
   i = (byte *) p - memory;
   return i;
}

// Allocator Functions

// Input: size - number of bytes to make available to the allocator
// Output: none              
// Precondition: Size is a power of two.
// Postcondition: `size` bytes are now available to the allocator
// 
// (If the allocator is already initialised, this function does nothing,
//  even if it was initialised with different size)

void vlad_init(u_int32_t size)
{
   // dummy statements to keep compiler happy
   // memory = NULL;
   // free_list_ptr = (vaddr_t)0;
   // memory_size = 0;
   // TODO
   // remove the above when you implement your code
   // printf("0\n"); // debug
   if (memory == NULL){ // if already initialised, do nothing
      size = power2(size); // translate to smallest larger power of 2
      memory = malloc(size); // malloc returns NULL on fail
      // printf("b\n"); // debug
      if (memory == NULL){   // if malloc failed:
         fprintf(stderr, "vlad_init: insufficient memory");
         abort();
      }
      memory_size = size;
      free_list_ptr = (vaddr_t) 0;
      free_header_t *init_header = (free_header_t *) memory;
      init_header->magic = MAGIC_FREE;
      init_header->size = size;
      init_header->next = free_list_ptr;
      init_header->prev = free_list_ptr;
      // printf("a\n"); // debug
   }
}


// Input: n - number of bytes requested
// Output: p - a pointer, or NULL
// Precondition: n is < size of memory available to the allocator
// Postcondition: If a region of size n or greater cannot be found, p = NULL 
//                Else, p points to a location immediately after a header block
//                      for a newly-allocated region of some size >= 
//                      n + header size.

void *vlad_malloc(u_int32_t n)
{
   // TODO
   // return NULL; // temporarily

   free_header_t *curr = (free_header_t *) itop(free_list_ptr);
   if (curr->magic != MAGIC_FREE){ // or MAGIC_ALLOC too?
      fprintf(stderr, "Memory corruption");
      abort();
   }

   free_header_t *chosen = NULL;
   int done = 0;
   int chosen_size = 0;

   // NOTE: Need to search for smallest region BEFORE splitting regions.
   while (done == 0){ // search for smallest region that can fit n
      // printf("1\n"); // debug
      if ((curr->size >= HEADER_SIZE + n) && ((curr->size < chosen_size) || (chosen_size == 0))){
         chosen = curr;
         chosen_size = curr->size;
      }
      if (curr->next == free_list_ptr){
         done = 1; // finished search
      }
      // printf("2\n"); // debug
      curr = (free_header_t *) itop(curr->next); // move to next region
   }
   if (chosen == NULL) return NULL;
   // OLD UNFINISHED METHOD:
   // while (done == 0) {
   //    if (curr->size < HEADER_SIZE + n){
   //       curr = itop(curr->next); // region too small, move to next region
   //    } else {
   //       if (curr->size < chosen_size){
   //          chosen = curr; // choose this region

   //       }

   //       // is this right?
   //       if (itop(curr->next) == curr){ // && curr->prev = curr // only free region
   //          return NULL;
   //       }
   //    }
   //    if (curr->next == free_list_ptr){
   //       // reached end of list
   //       done = 1;
   //    }
   // }

   // printf("3\n"); // debug
   // NOTE: next and prev are not real pointers but indexes!
   // void *, vaddr_t, vlink_t refer to locations in memory[]
   // need a way to map vetween void * and vaddr_t (i.e. pointer & index)
   byte *new_addr; // used for pointer arithmetic
   free_header_t *new;
   if ((curr->size/2) >= (HEADER_SIZE + n)){ // if can fit in half
      // split region into 2
      new_addr = (byte *) curr + (curr->size/2);
      new = (free_header_t *) new_addr;
      new->next = curr->next;
      new->prev = ptoi(curr);
      new->size = curr->size/2;
      new->magic = MAGIC_FREE;
      curr->size = curr->size/2;
      curr->next = ptoi(new);
   }

   // Allocate Region
   free_header_t *temp;
   temp = (free_header_t *) itop(curr->prev);
   temp->next = curr->next;
   temp = (free_header_t *) itop(curr->next);
   temp->prev = curr->prev;
   curr->magic = MAGIC_ALLOC;

   // if free_list_ptr is now allocated, adjust free_list_ptr
   curr = (free_header_t *) itop(free_list_ptr);
   if (curr->magic == MAGIC_ALLOC) free_list_ptr = curr->next;

   return ((void*) (chosen + HEADER_SIZE));
}


// Input: object, a pointer.
// Output: none
// Precondition: object points to a location immediately after a header block
//               within the allocator's memory.
// Postcondition: The region pointed to by object can be re-allocated by 
//                vlad_malloc

void vlad_free(void *object)
{
   // TODO
   byte *to_free_addr;
   free_header_t *to_free;
   to_free_addr = (byte *) object - HEADER_SIZE + 0x10;
   to_free = (free_header_t *) to_free_addr;
   printf("%p\n", to_free_addr); // pointer
   if (to_free->magic != MAGIC_ALLOC){
      printf("%x\n",to_free->magic); // pointer
      fprintf(stderr, "Attempt to free non-allocated memory");
      abort();
   }
   vaddr_t index = (byte *) to_free - memory;
   free_header_t *curr = (free_header_t *) itop(free_list_ptr);
   while (curr->next < index){
      curr = itop(curr->next);
   }
   to_free->prev = ptoi(curr);
   curr = itop(curr->next);
   to_free->next = ptoi(curr);
   to_free->magic = MAGIC_FREE;
   if (index < free_list_ptr){
      free_list_ptr = index;
   }
}


// Stop the allocator, so that it can be init'ed again:
// Precondition: allocator memory was once allocated by vlad_init()
// Postcondition: allocator is unusable until vlad_int() executed again

void vlad_end(void)
{
   // TODO

   free(memory);
}


// Precondition: allocator has been vlad_init()'d
// Postcondition: allocator stats displayed on stdout

void vlad_stats(void)
{
   // TODO
   // put whatever code you think will help you
   // understand Vlad's current state in this function
   // REMOVE all pfthese statements when your vlad_malloc() is done
   printf("vlad_stats() won't work until vlad_malloc() works\n");

   return;
}


//
// All of the code below here was written by Alen Bou-Haidar, COMP1927 14s2
//

//
// Fancy allocator stats
// 2D diagram for your allocator.c ... implementation
// 
// Copyright (C) 2014 Alen Bou-Haidar <alencool@gmail.com>
// 
// FancyStat is free software: you can redistribute it and/or modify 
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or 
// (at your option) any later version.
// 
// FancyStat is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>


#include <string.h>

#define STAT_WIDTH  32
#define STAT_HEIGHT 16
#define BG_FREE      "\x1b[48;5;35m" 
#define BG_ALLOC     "\x1b[48;5;39m"
#define FG_FREE      "\x1b[38;5;35m" 
#define FG_ALLOC     "\x1b[38;5;39m"
#define CL_RESET     "\x1b[0m"


typedef struct point {int x, y;} point;

static point offset_to_point(int offset,  int size, int is_end);
static void fill_block(char graph[STAT_HEIGHT][STAT_WIDTH][20], 
                        int offset, char * label);



// Print fancy 2D view of memory
// Note, This is limited to memory_sizes of under 16MB
void vlad_reveal(void *alpha[26])
{
    int i, j;
    vlink_t offset;
    char graph[STAT_HEIGHT][STAT_WIDTH][20];
    char free_sizes[26][32];
    char alloc_sizes[26][32];
    char label[3]; // letters for used memory, numbers for free memory
    int free_count, alloc_count, max_count;
    free_header_t * block;

	// TODO
	// REMOVE these statements when your vlad_malloc() is done
    // printf("vlad_reveal() won't work until vlad_malloc() works\n");
    // return;

    // initilise size lists
    for (i=0; i<26; i++) {
        free_sizes[i][0]= '\0';
        alloc_sizes[i][0]= '\0';
    }

    // Fill graph with free memory
    offset = 0;
    i = 1;
    free_count = 0;
    while (offset < memory_size){
        block = (free_header_t *)(memory + offset);
        if (block->magic == MAGIC_FREE) {
            snprintf(free_sizes[free_count++], 32, 
                "%d) %d bytes", i, block->size);
            snprintf(label, 3, "%d", i++);
            fill_block(graph, offset,label);
        }
        offset += block->size;
    }

    // Fill graph with allocated memory
    alloc_count = 0;
    for (i=0; i<26; i++) {
        if (alpha[i] != NULL) {
            offset = ((byte *) alpha[i] - (byte *) memory) - HEADER_SIZE;
            block = (free_header_t *)(memory + offset);
            snprintf(alloc_sizes[alloc_count++], 32, 
                "%c) %d bytes", 'a' + i, block->size);
            snprintf(label, 3, "%c", 'a' + i);
            fill_block(graph, offset,label);
        }
    }

    // Print all the memory!
    for (i=0; i<STAT_HEIGHT; i++) {
        for (j=0; j<STAT_WIDTH; j++) {
            printf("%s", graph[i][j]);
        }
        printf("\n");
    }

    //Print table of sizes
    max_count = (free_count > alloc_count)? free_count: alloc_count;
    printf(FG_FREE"%-32s"CL_RESET, "Free");
    if (alloc_count > 0){
        printf(FG_ALLOC"%s\n"CL_RESET, "Allocated");
    } else {
        printf("\n");
    }
    for (i=0; i<max_count;i++) {
        printf("%-32s%s\n", free_sizes[i], alloc_sizes[i]);
    }

}

// Fill block area
static void fill_block(char graph[STAT_HEIGHT][STAT_WIDTH][20], 
                        int offset, char * label)
{
    point start, end;
    free_header_t * block;
    char * color;
    char text[3];
    block = (free_header_t *)(memory + offset);
    start = offset_to_point(offset, memory_size, 0);
    end = offset_to_point(offset + block->size, memory_size, 1);
    color = (block->magic == MAGIC_FREE) ? BG_FREE: BG_ALLOC;

    int x, y;
    for (y=start.y; y < end.y; y++) {
        for (x=start.x; x < end.x; x++) {
            if (x == start.x && y == start.y) {
                // draw top left corner
                snprintf(text, 3, "|%s", label);
            } else if (x == start.x && y == end.y - 1) {
                // draw bottom left corner
                snprintf(text, 3, "|_");
            } else if (y == end.y - 1) {
                // draw bottom border
                snprintf(text, 3, "__");
            } else if (x == start.x) {
                // draw left border
                snprintf(text, 3, "| ");
            } else {
                snprintf(text, 3, "  ");
            }
            sprintf(graph[y][x], "%s%s"CL_RESET, color, text);            
        }
    }
}

// Converts offset to coordinate
static point offset_to_point(int offset,  int size, int is_end)
{
    int pot[2] = {STAT_WIDTH, STAT_HEIGHT}; // potential XY
    int crd[2] = {0};                       // coordinates
    int sign = 1;                           // Adding/Subtracting
    int inY = 0;                            // which axis context
    int curr = size >> 1;                   // first bit to check
    point c;                                // final coordinate
    if (is_end) {
        offset = size - offset;
        crd[0] = STAT_WIDTH;
        crd[1] = STAT_HEIGHT;
        sign = -1;
    }
    while (curr) {
        pot[inY] >>= 1;
        if (curr & offset) {
            crd[inY] += pot[inY]*sign; 
        }
        inY = !inY; // flip which axis to look at
        curr >>= 1; // shift to the right to advance
    }
    c.x = crd[0];
    c.y = crd[1];
    return c;
}
