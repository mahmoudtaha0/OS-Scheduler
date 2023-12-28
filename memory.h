#include "headers.h"

// buddy memory
typedef struct Memory
{
    bool is_free;
    int size;
    int pid;
    char *flag;
    struct Memory *next;
} Memory;

// memory to save values
Memory *memory = NULL;

char* inc_flag(const char *str, char num) {
    char numStr[2];
    numStr[0] = num;
    numStr[1] = '\0';

    size_t originalLength = str ? strlen(str) : 0;

    char *newStr = (char *)malloc(originalLength + 2); 

    if (newStr != NULL) {
        if (str != NULL) {
            strcpy(newStr, str);
        }
        strcat(newStr, numStr);
    }

    return newStr;
}

char* dec_flag(const char *str) {
    if (str != NULL && str[0] != '\0') {
        size_t originalLength = strlen(str);
        char *newStr = (char *)malloc(originalLength);
        
        if (newStr != NULL) {
            strncpy(newStr, str, originalLength - 1);
            newStr[originalLength - 1] = '\0';
        }

        return newStr;
    }

    return NULL;
}

// Function to initialize memory
void init_memory() {
    memory = (Memory*)malloc(sizeof(Memory));
    memory->is_free = true;
    memory->size = 1024;
    memory->pid = -1;
    memory->flag = NULL;
    memory->next = NULL;
}

// Function to split memory block
void split_memory(Memory* curr, int size)
{
    while (curr->size >= size * 2)
    {
        // Split memory
        Memory* split = (Memory*)malloc(sizeof(Memory));
        split->is_free = true;
        split->size = curr->size / 2;
        split->pid = -1;
        split->next = curr->next;
        curr->next = split;
        curr->size /= 2;
        char *temp_flag = curr->flag;
        split->flag = inc_flag(curr->flag, '2');
        curr->flag = inc_flag(temp_flag, '1');
        // printf("curr: %s, split: %s\n", curr->flag, split->flag);
    }
}

// Allocate memory and store process ID
int allocate_memory(int pid, int size) {
    Memory* curr = memory;

    while (curr != NULL) {
        if (curr->is_free && curr->size >= size) {
            // Split memory
            split_memory(curr, size);

            // Store process ID in the memory
            curr->pid = pid;

            curr->is_free = false;

            // Return pointer to the data within the block
            return true;
        }

        curr = curr->next;
    }

    // Allocation failed
    return false;
}

// merge all free blocks of the same size
void merge() {
    bool merged;

    do {
        merged = false;
        Memory* curr = memory;
        Memory* prev = NULL;

        while (curr != NULL && curr->next != NULL) 
        {
            if (curr->is_free && curr->next->is_free && curr->size == curr->next->size && curr->flag[strlen(curr->flag) - 1] == '1' && curr->next->flag[strlen(curr->next->flag) - 1] == '2')
            {
                // Merge blocks and double the size
                curr->size *= 2;
                Memory* temp = curr->next;
                curr->next = curr->next->next;
                free(temp);
                merged = true;
                curr->flag = dec_flag(curr->flag);
            }

            prev = curr;
            curr = curr->next;
        }
    } while (merged);
}

// Deallocate memory using process ID
int deallocate_memory(int pid) {
    Memory* curr = memory;
    Memory* prev = NULL;

    while (curr != NULL) {
        if (!curr->is_free && curr->pid == pid)
        {
            // block is now free
            curr->is_free = true;
            curr->pid = -1;
            // Merge adjacent free blocks
            merge();

            return true;
        }

        prev = curr;
        curr = curr->next;
    }

    
    return false;
}

// Function to get memory ranges allocated to a process
bool get_memory_ranges(int pid, int* start_range, int* end_range)
{

    Memory* curr = memory;
    int start_byte = 0;
    int end_byte = 0;

    while (curr != NULL) 
    {
        end_byte = start_byte + curr->size - 1;
       
        if (pid == curr->pid)
        {
            *start_range = start_byte;
            *end_range = end_byte;
            return true;
        }
        start_byte = end_byte + 1;  // Update start_byte for the next block
        curr = curr->next;
    }

    // Process with the given PID not found in memory
    return false;
}

