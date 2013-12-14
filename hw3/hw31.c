#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

unsigned int* arrayForSort1;	/* store the data for the first sort */
unsigned int n;			/* element number */
unsigned int threadNum;

unsigned int** arrayForSort2;	/* store the data for the second sort */
unsigned int* quantitiesForSort2; /* record the quantity of each thread should handle in the second sort */


void* sort1(void *rank);     /* sort function for first sort:get random separated n/threadNum elements, sort them for finding real separators */
void* sort2(void *rank);     /* sort function for second sort:get element in some rage, sort them for the next step--- merging them  */

/* funciton for qsort() */
int comparator(const void *n1, const void *n2);

int main(int argc, char *argv[]){
  //check argument num;
  if(argc != 3){
    printf("please input corretly argument!\n");
    return -1;
  }
  
  //check input thread quantity
  threadNum = atoi(argv[1]);
  if(threadNum <= 0){
    printf("please input corretly argument!\n");
    return -1;
  }

  //check input file name
  FILE* fp = fopen(argv[2],"r");
  if(fp == NULL){
    printf("please input corretly argument!\n");
    return -1;
  }

  //input data
  n = 0;
  unsigned int element;
  unsigned int capacity = 1024u;
  arrayForSort1 = (unsigned int *) calloc(1024u, sizeof(unsigned int));
   while(fscanf(fp,"%ud",&element)!=EOF){
     arrayForSort1[n] = element;
     ++n;
     if(n >= capacity){
       capacity += 1024u;
       arrayForSort1 = realloc(arrayForSort1, capacity*sizeof(unsigned int));
     }
   }
   
   //first sort
   pthread_t *threadIds = calloc(threadNum, sizeof(pthread_t));
   for(unsigned int i = 0; i < threadNum; ++i){
     pthread_create(threadIds+i, NULL, sort1, (void*) i);
   }
   for(unsigned int i = 0; i < threadNum; ++i){
     pthread_join(threadIds[i], NULL);
   }

  
  //get threadNum*(threadNum-1) sparators
  unsigned int handleQuantityInOneThread = n/threadNum;
  unsigned int temp = handleQuantityInOneThread / threadNum;
  unsigned int separatorsNum = threadNum * (threadNum - 1);
  unsigned int *separators = calloc(separatorsNum, sizeof(unsigned int));
  for(unsigned int i = 0; i < separatorsNum; ++i){
    unsigned int group = i/(threadNum - 1);
    unsigned int orderInGroup = i%(threadNum - 1);
    separators[i] = arrayForSort1[group * handleQuantityInOneThread + (orderInGroup + 1) * temp];
  }

  qsort(separators, separatorsNum, sizeof(unsigned int), comparator);

  //get threadNum-1 sparators;
  unsigned int *finalSeparators = calloc(threadNum-1, sizeof(unsigned int));
  for(unsigned int i = 1; i < threadNum; ++i){
    finalSeparators[i-1] = separators[i*(threadNum - 1)];
  }
  
  //separator elements in right rage
  quantitiesForSort2 = calloc(threadNum, sizeof(unsigned int));
  arrayForSort2 = calloc(threadNum, sizeof(unsigned int*));
  for(int i = 0; i < threadNum; ++i){
    arrayForSort2[i] = calloc(handleQuantityInOneThread * 3, sizeof(unsigned int));
  }
  for(int i = 0; i < n; ++i){
    int flag = 0;
    for(int j = 0; j < threadNum-1; ++j){
      if(arrayForSort1[i] < finalSeparators[j]){
	arrayForSort2[j][quantitiesForSort2[j]++] = arrayForSort1[i];
	flag = 1;
	break;
      }
    }
    if(flag == 0){
      arrayForSort2[threadNum-1][quantitiesForSort2[threadNum-1]++] = arrayForSort1[i];
    }
  }

  //sort elements in corresponding thread
  for(unsigned int i = 0; i < threadNum; ++i){
    pthread_create(threadIds+i, NULL, sort2, (void*) i);
  }
  for(unsigned int i = 0; i < threadNum; ++i){
    pthread_join(threadIds[i], NULL);
  }

  //merge elements sorted by childer thread
  unsigned offset = 0;
  for(unsigned int i = 0; i < threadNum; ++i){
    memcpy(arrayForSort1+offset,arrayForSort2[i],quantitiesForSort2[i]*sizeof(unsigned int));
    offset+=quantitiesForSort2[i];
  }

  //output
  printf("\n%d %d %d %d\n",
	 arrayForSort1[0],
	 arrayForSort1[n/4-1],
	 arrayForSort1[n/2-1],
	 arrayForSort1[n-1]);
 
  //free pointer
  free(threadIds);
  free(arrayForSort1);
  for(int i = 0; i < threadNum; ++i){
    free(arrayForSort2[i]);
  }  
  free(arrayForSort2);
  free(separators);
  free(quantitiesForSort2);
  return 0;
}

int comparator(const void *n1, const void *n2){
  unsigned int n1_i = *((unsigned int *)n1);
  unsigned int n2_i = *((unsigned int *)n2);
  if(n1_i < n2_i){
    return -1;
  }else if(n1_i > n2_i){
    return 1;
  }else{
    return 0;
  }
}

void* sort1(void* rank){
  unsigned int rank_i = (int) rank;
  unsigned int handleQuantityInOneThread = n / threadNum;
  unsigned int begin = rank_i * handleQuantityInOneThread;
  unsigned int size;
  if(rank_i != threadNum - 1){
    size = handleQuantityInOneThread;
  }else{
    size = n - begin;
  }
  qsort(arrayForSort1 + begin, size, sizeof(unsigned int), comparator);
  return NULL;
}

void* sort2(void* rank){
  unsigned int rank_i = (int) rank;
  qsort(arrayForSort2[rank_i], quantitiesForSort2[rank_i], sizeof(unsigned int), comparator);
  return NULL;
}

