/**
 * @file   pipe2sort.c
 * @author Zheqi Lu <albuscrow@gmail.com>
 * @date   Mon Oct 28 15:21:46 2013
 * 
 * @brief  nothing to say,let't begin!
 * 
 * 
 */


#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
void intToByteArray(unsigned int input,unsigned char* output){
  output[0] = (unsigned char)input;
  output[1] = (unsigned char)(input>>8);
  output[2] = (unsigned char)(input>>16);
  output[3] = (unsigned char)(input>>24);
}

int cmp(const void *ele1, const void *ele2){
  unsigned int e1 = *(unsigned int *)ele1;
  unsigned int e2 = *(unsigned int *)ele2;
  if(e1 > e2){
    return -1;
  }else if(e1 < e2){
    return 1;
  }else{
    return 0;
  }
}

unsigned int byteArrayToInt(unsigned char* output){
  return (unsigned int)(output[0] | (output[1] << 8) | (output[2] << 16) | (output[3] << 24));
}

int main(int argc, char *argv[]){
  /* the num of element which will be sorted */
  int threadNum = atoi(argv[1]);
  int *pipeIds = (int*) malloc(2*(threadNum+1)*sizeof(int));
  for(unsigned int i = 0; i < threadNum+1; ++i){
    if(pipe(&pipeIds[2*i]) < 0){
      printf("pipe create error\n");
      exit(-1);
    }
  }
  unsigned int index;
  for(index = 0; index < threadNum; ++index){
    int pid = fork();
    if(pid < 0){
      printf("fork failed\n");
      exit(-1);
    }else if(pid > 0){
      break;
    }
  }
  close(pipeIds[index*2+1]);
  close(pipeIds[(index+1)*2%22]);

  unsigned int eleNum = atoi(argv[2]);
  unsigned int capatity = eleNum/threadNum + 1;
  if(index == 0u){
    FILE *fp;
    fp = fopen(argv[3], "r");
    if(fp == NULL){
      printf("open data file failed!\n");
      exit(1);
    }
    unsigned int *eles = (unsigned int *)malloc(sizeof(unsigned int)*capatity*threadNum);
    memset(eles, 0, sizeof(unsigned int)*capatity*threadNum);
    for(unsigned int i = 0; i < eleNum; ++i){
      fscanf(fp, "%u", eles+i);
    }
    unsigned char ele_byteArray[4];
    unsigned char *buffer =(unsigned char *) malloc(capatity*4*sizeof(unsigned char));
    //printf("%u %u",threadNum, capatity);
    for(unsigned int i_e = 0; i_e < threadNum; ++i_e){
      for(unsigned int i = 0; i < capatity; ++i){
	intToByteArray(eles[capatity*i_e + i],ele_byteArray);
	memcpy(buffer + 4*i, ele_byteArray , 4);
      }
      write(pipeIds[3], buffer, 4*capatity);
    }

    printf("sorted array is:\n");
    for(unsigned int i = 0; i < threadNum; ++i){
      read(pipeIds[0], buffer, 4*capatity);
      for(unsigned j = 0; j < capatity && capatity*i+j < eleNum; ++j){
	printf("%u ",byteArrayToInt(buffer + j*4));
      }
      printf("\n");
    }
    free(buffer);
    free(eles);
  }else{
    unsigned char *buffer = (unsigned char *)malloc(capatity*4*sizeof(unsigned char));
    read(pipeIds[index*2],buffer,4*capatity);
    unsigned int *eles = (unsigned int *) malloc(capatity*sizeof(unsigned int));
    for(unsigned int i = 0; i < capatity; ++i){
      eles[i] = byteArrayToInt(buffer + 4*i);
    }
    qsort(eles, capatity ,sizeof(unsigned int),cmp);	
    unsigned int *eles_readed = (unsigned int *) malloc(capatity*sizeof(unsigned int));
    unsigned int *eles_temp =  (unsigned int *) malloc(capatity*sizeof(unsigned int)*2);
    for(unsigned int i_i = index; i_i < threadNum; ++i_i){
      read(pipeIds[index*2],buffer,4*capatity);
      for(unsigned int i = 0; i < capatity; ++i){
	eles_readed[i] = byteArrayToInt(buffer + 4*i);
      }
      memcpy(eles_temp,eles,capatity*sizeof(unsigned int));
      memcpy(eles_temp + capatity,eles_readed, capatity*sizeof(unsigned int));
      qsort(eles_temp, 2*capatity, sizeof(unsigned int), cmp);
      memcpy(eles, eles_temp, capatity*sizeof(unsigned int));
      for(unsigned int i = 0; i < capatity; ++i){
	intToByteArray(*(eles_temp+capatity+i), buffer+i*4);
      }
      write(pipeIds[index*2+3], buffer, 4*capatity);
    }
    for(unsigned int i = 1; i < index; ++i){
      read(pipeIds[index*2],buffer,4*capatity);
      write(pipeIds[(index*2+3)%(2*threadNum+2)], buffer, 4*capatity);
    }
    for(unsigned int i = 0; i < capatity; ++i){
      intToByteArray(*(eles+i), buffer+i*4);
    }
    write(pipeIds[(index*2+3)%(2*threadNum+2)], buffer, 4*capatity);
    free(buffer);
    free(eles);
    free(eles_temp);
    free(eles_readed);
  }
  free(pipeIds);
  return 0;
}
