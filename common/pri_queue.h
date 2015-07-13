#ifndef ____pri_queue__
#define ____pri_queue__

typedef struct { void * data; int pri; } q_elem_t;
typedef struct { q_elem_t *buf; int n, alloc; } pri_queue_t, *pri_queue;

#endif /* defined(____pri_queue__) */


pri_queue priq_new(int size);

void priq_push(pri_queue q, void *data, int pri);

void *priq_pop(pri_queue q, int *pri);

void *priq_top(pri_queue q, int *pri);
 
void priq_combine(pri_queue q, pri_queue q2);
