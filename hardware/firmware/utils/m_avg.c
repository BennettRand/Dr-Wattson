#include<stdio.h>
#include<stdint.h>

#include"m_avg.h"

struct queue{
	int pos;
	int end;
	int32_t array[SAMPLES];
	int n;
};

/*void queue_inc(int* x)
{
	if (*x >= SAMPLES)
		{
			*x = 0;
		}
	else
		{
			*x = *x+1;
		}
}*/

void init_queue(struct queue* x)
{
	x->pos = 0;
	x->end = 0;
	x->n = 0;
}

int push(struct queue* x, int32_t n)
{
	if(x->n >= SAMPLES)
		{
			return -1;
		}
	else
		{
			x->array[x->pos] = n;
			x->n++;
			QUEUE_INC(x->pos);
		}
	return 0;
}

int32_t pop(struct queue* x)
{
	int to_ret = 0;
	if(x->n <= 0)
		{
			return -1;
		}
	else
		{
			to_ret = x->array[x->end];
			x->n--;
			QUEUE_INC(x->end);
		}
	return to_ret;
}

struct m_avg{
	int32_t sum;
	struct queue q;
	int16_t n;
};

void avg_init(struct m_avg* x)
{
	x->sum = 0;
	init_queue(&x->q);
	x->n = 0;
	return;
}

int avg_add(struct m_avg* x, long n)
{
	if(x->n >= SAMPLES)
		{
			x->sum -= pop(&x->q);
			x->n--;
		}
	x->sum += n;
	push(&x->q, n);
	x->n++;
	return 1;
}

int32_t avg_get(struct m_avg* x)
{
	return x->sum/x->n;
}


#ifdef DEBUG
int main(int argc, char** argv)
{
	struct m_avg a;
	avg_init(&a);
	printf("Init done. Window of %d\n", SAMPLES);
	avg_add(&a, 5);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 6);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 5);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 4);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 6);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 10);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 8);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 6);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 7);
	printf("%d\n", avg_get(&a));
	avg_add(&a, 3);
	printf("%d\n", avg_get(&a));
	return 0;
}
#endif
