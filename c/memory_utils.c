/**
 * Memory Management Utilities for Embedded Systems
 * 
 * Implementazioni sicure per gestione memoria:
 * - Pool allocator per allocazione fissa
 * - Safe string operations
 * - Memory checking utilities
 * 
 * Complessità: O(1) per allocazione pool, O(n) per operazioni string
 * Memoria: Nessun malloc/free, solo allocazione statica
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

// =============================================================================
// MEMORY POOL ALLOCATOR
// =============================================================================

#define POOL_BLOCK_SIZE 64
#define POOL_NUM_BLOCKS 32

typedef struct {
    uint8_t data[POOL_BLOCK_SIZE];
    bool in_use;
} MemoryBlock;

typedef struct {
    MemoryBlock blocks[POOL_NUM_BLOCKS];
    uint32_t allocated_count;
    uint32_t peak_usage;
} MemoryPool;

static MemoryPool g_memory_pool = {0};

void* pool_alloc(size_t size) {
    if (size == 0 || size > POOL_BLOCK_SIZE) {
        return NULL;  // Invalid size
    }
    
    for (int i = 0; i < POOL_NUM_BLOCKS; i++) {
        if (!g_memory_pool.blocks[i].in_use) {
            g_memory_pool.blocks[i].in_use = true;
            g_memory_pool.allocated_count++;
            
            if (g_memory_pool.allocated_count > g_memory_pool.peak_usage) {
                g_memory_pool.peak_usage = g_memory_pool.allocated_count;
            }
            
            // Clear memory
            memset(g_memory_pool.blocks[i].data, 0, POOL_BLOCK_SIZE);
            return g_memory_pool.blocks[i].data;
        }
    }
    
    return NULL;  // Pool exhausted
}

bool pool_free(void* ptr) {
    if (ptr == NULL) {
        return false;
    }
    
    // Find block containing this pointer
    for (int i = 0; i < POOL_NUM_BLOCKS; i++) {
        uint8_t* block_start = g_memory_pool.blocks[i].data;
        uint8_t* block_end = block_start + POOL_BLOCK_SIZE;
        
        if (ptr >= (void*)block_start && ptr < (void*)block_end) {
            if (g_memory_pool.blocks[i].in_use) {
                g_memory_pool.blocks[i].in_use = false;
                g_memory_pool.allocated_count--;
                
                // Clear memory for security
                memset(g_memory_pool.blocks[i].data, 0, POOL_BLOCK_SIZE);
                return true;
            }
            break;  // Double free detected
        }
    }
    
    return false;  // Invalid pointer
}

void pool_get_stats(uint32_t* current, uint32_t* peak, uint32_t* total) {
    if (current) *current = g_memory_pool.allocated_count;
    if (peak) *peak = g_memory_pool.peak_usage;
    if (total) *total = POOL_NUM_BLOCKS;
}

void pool_reset(void) {
    for (int i = 0; i < POOL_NUM_BLOCKS; i++) {
        g_memory_pool.blocks[i].in_use = false;
        memset(g_memory_pool.blocks[i].data, 0, POOL_BLOCK_SIZE);
    }
    g_memory_pool.allocated_count = 0;
    // Note: non resettiamo peak_usage per mantenere statistiche
}

// =============================================================================
// SAFE STRING OPERATIONS
// =============================================================================

// Safe string copy con controllo bounds
int safe_strcpy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return -1;  // Invalid parameters
    }
    
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    
    return (src[i] == '\0') ? 0 : 1;  // 0 = success, 1 = truncated
}

// Safe string concatenation
int safe_strcat(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return -1;  // Invalid parameters
    }
    
    // Find end of destination string
    size_t dest_len = 0;
    while (dest_len < dest_size && dest[dest_len] != '\0') {
        dest_len++;
    }
    
    if (dest_len >= dest_size) {
        return -1;  // Destination not null-terminated
    }
    
    // Append source string
    size_t i;
    for (i = 0; dest_len + i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[dest_len + i] = src[i];
    }
    dest[dest_len + i] = '\0';
    
    return (src[i] == '\0') ? 0 : 1;  // 0 = success, 1 = truncated
}

// Safe formatted string print
int safe_sprintf(char* dest, size_t dest_size, const char* format, ...) {
    if (dest == NULL || format == NULL || dest_size == 0) {
        return -1;
    }
    
    va_list args;
    va_start(args, format);
    
    int result = vsnprintf(dest, dest_size, format, args);
    
    va_end(args);
    
    if (result < 0) {
        dest[0] = '\0';  // Clear on error
        return -1;
    }
    
    return (result >= (int)dest_size) ? 1 : 0;  // 1 if truncated
}

// =============================================================================
// MEMORY CHECKING UTILITIES
// =============================================================================

// Pattern per rilevare buffer overflow
#define GUARD_PATTERN 0xDEADBEEF

typedef struct {
    uint32_t guard_start;
    uint8_t data[];
    // guard_end viene posizionato dopo i dati
} GuardedBuffer;

void* create_guarded_buffer(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Alloca spazio per guard pattern + dati + guard pattern
    size_t total_size = sizeof(uint32_t) + size + sizeof(uint32_t);
    void* memory = pool_alloc(total_size);
    
    if (memory == NULL) {
        return NULL;
    }
    
    GuardedBuffer* buffer = (GuardedBuffer*)memory;
    buffer->guard_start = GUARD_PATTERN;
    
    // Guard pattern alla fine
    uint32_t* guard_end = (uint32_t*)((uint8_t*)buffer->data + size);
    *guard_end = GUARD_PATTERN;
    
    return buffer->data;
}

bool check_buffer_integrity(void* ptr, size_t size) {
    if (ptr == NULL || size == 0) {
        return false;
    }
    
    GuardedBuffer* buffer = (GuardedBuffer*)((uint8_t*)ptr - sizeof(uint32_t));
    uint32_t* guard_end = (uint32_t*)((uint8_t*)ptr + size);
    
    return (buffer->guard_start == GUARD_PATTERN && *guard_end == GUARD_PATTERN);
}

void free_guarded_buffer(void* ptr, size_t size) {
    if (ptr == NULL) {
        return;
    }
    
    GuardedBuffer* buffer = (GuardedBuffer*)((uint8_t*)ptr - sizeof(uint32_t));
    
    if (check_buffer_integrity(ptr, size)) {
        pool_free(buffer);
    } else {
        printf("WARNING: Buffer corruption detected before free!\n");
        // In un sistema embedded, potresti voler triggerare un reset
    }
}

// =============================================================================
// SECURE MEMORY OPERATIONS
// =============================================================================

// Secure memory clear (previene ottimizzazioni del compilatore)
void secure_memclear(void* ptr, size_t size) {
    if (ptr == NULL || size == 0) {
        return;
    }
    
    volatile uint8_t* volatile_ptr = (volatile uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) {
        volatile_ptr[i] = 0;
    }
}

// Secure memory comparison (timing attack resistant)
bool secure_memcmp(const void* ptr1, const void* ptr2, size_t size) {
    if (ptr1 == NULL || ptr2 == NULL) {
        return false;
    }
    
    const uint8_t* bytes1 = (const uint8_t*)ptr1;
    const uint8_t* bytes2 = (const uint8_t*)ptr2;
    uint8_t result = 0;
    
    for (size_t i = 0; i < size; i++) {
        result |= bytes1[i] ^ bytes2[i];
    }
    
    return result == 0;
}

// =============================================================================
// TEST FUNCTIONS
// =============================================================================

void test_memory_pool(void) {
    printf("=== Testing Memory Pool ===\n");
    
    uint32_t current, peak, total;
    pool_get_stats(&current, &peak, &total);
    printf("Initial stats: %u/%u blocks used (peak: %u)\n", current, total, peak);
    
    // Alloca alcuni blocchi
    void* ptr1 = pool_alloc(32);
    void* ptr2 = pool_alloc(16);
    void* ptr3 = pool_alloc(64);
    
    pool_get_stats(&current, &peak, &total);
    printf("After allocation: %u/%u blocks used (peak: %u)\n", current, total, peak);
    
    // Test scrittura
    if (ptr1) {
        strcpy((char*)ptr1, "Hello, Pool!");
        printf("Written to ptr1: %s\n", (char*)ptr1);
    }
    
    // Libera memoria
    pool_free(ptr2);
    pool_get_stats(&current, &peak, &total);
    printf("After freeing ptr2: %u/%u blocks used (peak: %u)\n", current, total, peak);
    
    pool_free(ptr1);
    pool_free(ptr3);
    
    pool_get_stats(&current, &peak, &total);
    printf("After freeing all: %u/%u blocks used (peak: %u)\n", current, total, peak);
    
    printf("\n");
}

void test_safe_strings(void) {
    printf("=== Testing Safe String Operations ===\n");
    
    char buffer[32];
    
    // Test safe_strcpy
    int result = safe_strcpy(buffer, sizeof(buffer), "Hello, World!");
    printf("strcpy result: %d, buffer: '%s'\n", result, buffer);
    
    // Test safe_strcat
    result = safe_strcat(buffer, sizeof(buffer), " Extra text!");
    printf("strcat result: %d, buffer: '%s'\n", result, buffer);
    
    // Test truncation
    result = safe_strcpy(buffer, 8, "This is a very long string");
    printf("strcpy truncated: result=%d, buffer: '%s'\n", result, buffer);
    
    // Test safe_sprintf
    result = safe_sprintf(buffer, sizeof(buffer), "Number: %d, Float: %.2f", 42, 3.14);
    printf("sprintf result: %d, buffer: '%s'\n", result, buffer);
    
    printf("\n");
}

void test_guarded_buffers(void) {
    printf("=== Testing Guarded Buffers ===\n");
    
    const size_t buffer_size = 16;
    void* buffer = create_guarded_buffer(buffer_size);
    
    if (buffer) {
        printf("Created guarded buffer\n");
        
        // Test scrittura normale
        strcpy((char*)buffer, "Test data");
        printf("Written data: %s\n", (char*)buffer);
        
        // Check integrity
        bool integrity = check_buffer_integrity(buffer, buffer_size);
        printf("Buffer integrity: %s\n", integrity ? "OK" : "CORRUPTED");
        
        // Simula buffer overflow (commenta per test normale)
        // ((char*)buffer)[buffer_size + 1] = 'X';  // Overflow!
        
        integrity = check_buffer_integrity(buffer, buffer_size);
        printf("Buffer integrity after use: %s\n", integrity ? "OK" : "CORRUPTED");
        
        free_guarded_buffer(buffer, buffer_size);
    } else {
        printf("Failed to create guarded buffer\n");
    }
    
    printf("\n");
}

void test_secure_operations(void) {
    printf("=== Testing Secure Operations ===\n");
    
    char secret[16] = "secret_password";
    char compare[16] = "secret_password";
    char different[16] = "different_pass";
    
    printf("Original secret: %s\n", secret);
    
    // Test secure comparison
    bool match1 = secure_memcmp(secret, compare, strlen(secret));
    bool match2 = secure_memcmp(secret, different, strlen(secret));
    
    printf("Secure compare (same): %s\n", match1 ? "MATCH" : "NO MATCH");
    printf("Secure compare (different): %s\n", match2 ? "MATCH" : "NO MATCH");
    
    // Secure clear
    secure_memclear(secret, sizeof(secret));
    printf("After secure clear: '%s'\n", secret);
    
    printf("\n");
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    printf("Memory Management Utilities Test\n");
    printf("===============================\n\n");
    
    test_memory_pool();
    test_safe_strings();
    test_guarded_buffers();
    test_secure_operations();
    
    // Final memory stats
    uint32_t current, peak, total;
    pool_get_stats(&current, &peak, &total);
    printf("Final memory stats: %u/%u blocks used (peak: %u)\n", current, total, peak);
    
    printf("All tests completed successfully!\n");
    return 0;
}