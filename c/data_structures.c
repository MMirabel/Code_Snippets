/**
 * Strutture dati leggere per sistemi embedded
 * 
 * Implementazioni ottimizzate per memoria limitata:
 * - Stack LIFO con array statico
 * - Queue FIFO circolare
 * - Linked list con pool di nodi
 * 
 * Complessità: O(1) per tutte le operazioni base
 * Memoria: Allocazione statica, nessun malloc/free
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// =============================================================================
// STACK IMPLEMENTATION
// =============================================================================

#define STACK_MAX_SIZE 128

typedef struct {
    int32_t data[STACK_MAX_SIZE];
    int top;
} Stack;

void stack_init(Stack* stack) {
    stack->top = -1;
}

bool stack_is_empty(const Stack* stack) {
    return stack->top == -1;
}

bool stack_is_full(const Stack* stack) {
    return stack->top == STACK_MAX_SIZE - 1;
}

bool stack_push(Stack* stack, int32_t value) {
    if (stack_is_full(stack)) {
        return false;  // Stack overflow
    }
    stack->data[++stack->top] = value;
    return true;
}

bool stack_pop(Stack* stack, int32_t* value) {
    if (stack_is_empty(stack)) {
        return false;  // Stack underflow
    }
    *value = stack->data[stack->top--];
    return true;
}

bool stack_peek(const Stack* stack, int32_t* value) {
    if (stack_is_empty(stack)) {
        return false;
    }
    *value = stack->data[stack->top];
    return true;
}

int stack_size(const Stack* stack) {
    return stack->top + 1;
}

// =============================================================================
// CIRCULAR QUEUE IMPLEMENTATION
// =============================================================================

#define QUEUE_MAX_SIZE 64

typedef struct {
    int32_t data[QUEUE_MAX_SIZE];
    int front;
    int rear;
    int count;
} Queue;

void queue_init(Queue* queue) {
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;
}

bool queue_is_empty(const Queue* queue) {
    return queue->count == 0;
}

bool queue_is_full(const Queue* queue) {
    return queue->count == QUEUE_MAX_SIZE;
}

bool queue_enqueue(Queue* queue, int32_t value) {
    if (queue_is_full(queue)) {
        return false;  // Queue overflow
    }
    
    queue->rear = (queue->rear + 1) % QUEUE_MAX_SIZE;
    queue->data[queue->rear] = value;
    queue->count++;
    return true;
}

bool queue_dequeue(Queue* queue, int32_t* value) {
    if (queue_is_empty(queue)) {
        return false;  // Queue underflow
    }
    
    *value = queue->data[queue->front];
    queue->front = (queue->front + 1) % QUEUE_MAX_SIZE;
    queue->count--;
    return true;
}

bool queue_peek(const Queue* queue, int32_t* value) {
    if (queue_is_empty(queue)) {
        return false;
    }
    *value = queue->data[queue->front];
    return true;
}

int queue_size(const Queue* queue) {
    return queue->count;
}

// =============================================================================
// LINKED LIST WITH STATIC POOL
// =============================================================================

#define NODE_POOL_SIZE 32

typedef struct Node {
    int32_t data;
    struct Node* next;
} Node;

typedef struct {
    Node pool[NODE_POOL_SIZE];
    bool used[NODE_POOL_SIZE];
    Node* head;
    int size;
} LinkedList;

static Node* allocate_node(LinkedList* list) {
    for (int i = 0; i < NODE_POOL_SIZE; i++) {
        if (!list->used[i]) {
            list->used[i] = true;
            return &list->pool[i];
        }
    }
    return NULL;  // Pool exhausted
}

static void free_node(LinkedList* list, Node* node) {
    int index = node - list->pool;
    if (index >= 0 && index < NODE_POOL_SIZE) {
        list->used[index] = false;
    }
}

void list_init(LinkedList* list) {
    list->head = NULL;
    list->size = 0;
    memset(list->used, false, sizeof(list->used));
}

bool list_insert_front(LinkedList* list, int32_t value) {
    Node* new_node = allocate_node(list);
    if (!new_node) {
        return false;  // No memory available
    }
    
    new_node->data = value;
    new_node->next = list->head;
    list->head = new_node;
    list->size++;
    return true;
}

bool list_remove_front(LinkedList* list, int32_t* value) {
    if (!list->head) {
        return false;  // List is empty
    }
    
    Node* temp = list->head;
    *value = temp->data;
    list->head = temp->next;
    free_node(list, temp);
    list->size--;
    return true;
}

bool list_find(const LinkedList* list, int32_t value) {
    Node* current = list->head;
    while (current) {
        if (current->data == value) {
            return true;
        }
        current = current->next;
    }
    return false;
}

bool list_remove_value(LinkedList* list, int32_t value) {
    if (!list->head) {
        return false;
    }
    
    // Se è il primo elemento
    if (list->head->data == value) {
        Node* temp = list->head;
        list->head = temp->next;
        free_node(list, temp);
        list->size--;
        return true;
    }
    
    // Cerca negli altri elementi
    Node* current = list->head;
    while (current->next) {
        if (current->next->data == value) {
            Node* temp = current->next;
            current->next = temp->next;
            free_node(list, temp);
            list->size--;
            return true;
        }
        current = current->next;
    }
    return false;
}

int list_size(const LinkedList* list) {
    return list->size;
}

void list_print(const LinkedList* list) {
    printf("List: ");
    Node* current = list->head;
    while (current) {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\n");
}

// =============================================================================
// TEST FUNCTIONS
// =============================================================================

void test_stack(void) {
    printf("=== Testing Stack ===\n");
    Stack stack;
    stack_init(&stack);
    
    // Test push
    for (int i = 1; i <= 5; i++) {
        stack_push(&stack, i * 10);
        printf("Pushed %d, size: %d\n", i * 10, stack_size(&stack));
    }
    
    // Test peek
    int32_t value;
    if (stack_peek(&stack, &value)) {
        printf("Top value: %d\n", value);
    }
    
    // Test pop
    while (!stack_is_empty(&stack)) {
        if (stack_pop(&stack, &value)) {
            printf("Popped %d, size: %d\n", value, stack_size(&stack));
        }
    }
    printf("\n");
}

void test_queue(void) {
    printf("=== Testing Queue ===\n");
    Queue queue;
    queue_init(&queue);
    
    // Test enqueue
    for (int i = 1; i <= 5; i++) {
        queue_enqueue(&queue, i * 100);
        printf("Enqueued %d, size: %d\n", i * 100, queue_size(&queue));
    }
    
    // Test peek
    int32_t value;
    if (queue_peek(&queue, &value)) {
        printf("Front value: %d\n", value);
    }
    
    // Test dequeue
    while (!queue_is_empty(&queue)) {
        if (queue_dequeue(&queue, &value)) {
            printf("Dequeued %d, size: %d\n", value, queue_size(&queue));
        }
    }
    printf("\n");
}

void test_linked_list(void) {
    printf("=== Testing Linked List ===\n");
    LinkedList list;
    list_init(&list);
    
    // Test insert
    for (int i = 1; i <= 5; i++) {
        list_insert_front(&list, i * 1000);
        printf("Inserted %d, size: %d\n", i * 1000, list_size(&list));
        list_print(&list);
    }
    
    // Test find
    if (list_find(&list, 3000)) {
        printf("Found 3000 in list\n");
    }
    
    // Test remove specific value
    if (list_remove_value(&list, 3000)) {
        printf("Removed 3000 from list\n");
        list_print(&list);
    }
    
    // Test remove front
    int32_t value;
    while (list_size(&list) > 0) {
        if (list_remove_front(&list, &value)) {
            printf("Removed front %d, size: %d\n", value, list_size(&list));
        }
    }
    printf("\n");
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    printf("Data Structures Test Program\n");
    printf("===========================\n\n");
    
    test_stack();
    test_queue();
    test_linked_list();
    
    printf("All tests completed successfully!\n");
    return 0;
}