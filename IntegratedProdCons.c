/*
 *	File	: pc.c
 *
 *	Title	: Demo Producer/Consumer.
 *
 *	Short	: A solution to the producer consumer problem using
 *		pthreads.	
 *
 *	Long 	:
 *
 *	Author	: Andrae Muys
 *
 *	Date	: 18 September 1997
 *
 *	Revised	:
 */


// Kotoulas Emmanouil 9697 
// Compile with gcc IntegratedProdCons.c -o integrated.out -pthread -lm


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h> 


#define QUEUESIZE 10
#define LOOP 20000

struct timeval cur_time,cur_time2;

double timer ;

void *producer (void *args);
void *consumer (void *args);


void mod(){
	int j = 0;
	for(int i = 1 ; i < 11 ; i++){
		j = sin(11 * i);
	}
}

void coses(){
	int k = 0;
	for(int i = 0 ; i < 10 ; i++){
		k = cos((i*10));
	}
	//printf("i calcd coses\n");
}

void* randSelFunc(){
	int randNum = (rand() % 2);
	if(randNum){
		void (*workpntr)() = coses;
		return workpntr;
	}
	else{
		void (*workpntr)() = mod;
		return workpntr;
	}
}

typedef struct workFunction {
  void * (*work)(void *);
  void * arg;
  struct timeval startwtime, endwtime;
} workFunc;

typedef struct {
  workFunc buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;


queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, workFunc in);
void queueDel (queue *q, workFunc *out);

int main (int argc, char *argv[])
{
  srand((unsigned int)time(NULL));

  int n = atoi(argv[1]);
  int m = atoi(argv[2]);

  timer = 0;
	
  queue *fifo;
  pthread_t *pro = (pthread_t*)malloc(sizeof(pthread_t)*n);
  pthread_t *con = (pthread_t*)malloc(sizeof(pthread_t)*m);

  fifo = queueInit ();
  if (fifo ==  NULL) {
    fprintf (stderr, "main: Queue Init failed.\n");
    exit (1);
  }
  
  for(int i = 0 ; i < n ; i++){
	pthread_create (&pro[i], NULL, producer, fifo);
  }
  
  for(int j = 0 ; j < m ; j++){
	pthread_create (&con[j], NULL, consumer, fifo);
  }
  
  for(int i = 0 ; i < n ; i++){
	pthread_join (pro[i], NULL);
  }
  
  for(int j = 0 ; j < m ; j++){
	pthread_join (con[j], NULL);
	}
  
  queueDelete (fifo);
  
  return 0;
}

void *producer (void *q)
{
  queue *fifo;
  int i;

  fifo = (queue *)q;

  for (i = 0; i < LOOP; i++) {
    pthread_mutex_lock (fifo->mut);
    while (fifo->full) {
      //printf ("producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }
	
	workFunc workpoint;
	workpoint.work = randSelFunc();
	
  gettimeofday (&cur_time, NULL);
  timer -= (double)(cur_time.tv_usec/1.0e6 + cur_time.tv_sec);

  queueAdd (fifo, workpoint);
  pthread_mutex_unlock (fifo->mut);
  pthread_cond_signal (fifo->notEmpty);
	
  }
  
  sleep(30);
  printf("%f \n",timer);
  return (NULL);
  
}

void *consumer (void *q)
{
  queue *fifo;
  int i = 0;
  workFunc workpoint;

  fifo = (queue *)q;
	
  while(1){
    pthread_mutex_lock (fifo->mut);
    while (fifo->empty) {
      //printf ("consumer: queue EMPTY.\n");
      pthread_cond_wait (fifo->notEmpty, fifo->mut);
    }
  
  gettimeofday(&cur_time2,NULL);
	timer += (double)(cur_time2.tv_usec/1.0e6 + cur_time2.tv_sec);

  queueDel (fifo, &workpoint);
	workpoint.work(NULL);
	
  pthread_mutex_unlock (fifo->mut);
  pthread_cond_signal (fifo->notFull);
	

  }
  
  return (NULL);
}

queue *queueInit (void)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);
	
  return (q);
}

void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);	
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

void queueAdd (queue *q, workFunc in)
{
  q->buf[q->tail] = in;
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;

  return;
}

void queueDel (queue *q, workFunc *out)
{
  *out = q->buf[q->head];
  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  return;
}
