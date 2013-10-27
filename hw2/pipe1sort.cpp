/**
 * @file   pipe1sort.c
 * @author Zheqi Lu <albuscrow@gmail.com>
 * @date   Fri Oct 25 17:47:08 2013
 * 
 * @brief  nothing to say, let's begin!
 * 
 * 
 */

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
void intToByteArray(int input, char* output){
  output[0] = (char)input;
  output[1] = (char)input>>8;
  output[2] = (char)input>>16;
  output[3] = (char)input>>24;
}

unsigned int byteArrayToInt(char* output){
  return (unsigned int)(output[0] | (output[1] << 8) | (output[2] << 16) | (output[3] << 24));
}

int main(int argc, char *argv[]){
  /* the num of element which will be sorted */
  int num = atoi(argv[1]);
  int *pipeIds = (int*) malloc(2*(num+1)*sizeof(int));
  for(unsigned int i = 0; i < num+1; ++i){
    if(pipe(&pipeIds[2*i]) < 0){
      printf("pipe create error\n");
      exit(-1);
    }
  }
  unsigned int index;
  for(index = 0; index < num; ++index){
    //printf("%d\n", index);

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
  if(index == 0u){
    //printf("begin main thread\n");
    FILE *fp;
    fp = fopen(argv[2], "r");
    if(fp == NULL){
      printf("open data file failed!\n");
      exit(1);
    }
    unsigned int ele;
    char ele_byteArray[4];
    for(unsigned int i_e = 0; i_e < num; ++i_e){
      fscanf(fp,"%u",&ele);

      //debug output
      //printf("%u:%u\n", i_e, ele);

      intToByteArray(ele,ele_byteArray);
      write(pipeIds[3], ele_byteArray, 4);
    }
    printf("sorted array is:");
  }else{
    //printf("begin child thread %u\n",index);
    char ele_byteArray[4];
    read(pipeIds[index*2],ele_byteArray,4);
    unsigned int ele;
    ele = byteArrayToInt(ele_byteArray);
    unsigned int ele_readed;
    for(unsigned int i_i = index; i_i < num; ++i_i){
      read(pipeIds[index*2],ele_byteArray,4);
      ele_readed = byteArrayToInt(ele_byteArray);
      if(ele_readed > ele){
	ele_readed += ele;
	ele = ele_readed - ele;
	ele_readed = ele_readed - ele;
      }
      intToByteArray(ele_readed, ele_byteArray);
      write(pipeIds[index*2+3], ele_byteArray, 4);
    }
    printf("%u ", ele);
  }
  return 0;
}

