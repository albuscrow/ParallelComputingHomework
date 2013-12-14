#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int thread_count;

void* Hello(void* rand);

int main(int argc, char* argv[]){
  int thread;
  pthread_t* thread_handles;
  thread_count = atoi(argv[1]);
  thread_handles = malloc(thread_count*sizeof(pthread_t));

  for(thread = 0 ; thread < thread_count; ++thread){
    pthread_create(&thread_handles[thread], NULL, Hello, (void*) thread);
  }
  
  printf("hello world from the main thread\n");

  for(thread = 0; thread < thread_count; ++thread){
    pthread_join(thread_handles[thread], NULL);
  }
  pthread_
  free(thread_handles);
  return 0;
  
}

void* Hello(void* rank){
  int my_rank = (int)rank;
  printf("Hello from thread %d of %d\n", my_rank, thread_count);
  return NULL;
}
