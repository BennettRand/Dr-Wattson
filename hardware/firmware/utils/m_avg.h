#ifndef M_AVG_H
#define M_AVG_H

#define SAMPLES 5

#define QUEUE_INC(x) x = (x >= SAMPLES) ? 0 : (x + 1)

struct queue{
	int pos;
	int end;
	int32_t array[SAMPLES];
	int n;
};

//Initializes queue
void init_queue(struct queue* x);

//Pushes n onto queue x. Returns 0 on success, -1 on failure.
int push(struct queue* x, int32_t n);

//Returns oldest element from queue and removes, returns -1 if there are 0 elements
int32_t pop(struct queue* x);


struct m_avg{
	int32_t sum;
	struct queue q;
	int16_t n;
};

//Initializes moving average
void avg_init(struct m_avg* x);

//Adds a number into the current window
int avg_add(struct m_avg* x, long n);

//Returns average of current window
int32_t avg_get(struct m_avg* x);

#endif
