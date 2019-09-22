
#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_dev.h"
#include "util.h"
#include "kernel_proc.h"
#include "kernel_cc.h"
#include <assert.h>
#include <stdlib.h>

int BufPush(Pipe_Buffer_t *c, char data, void* p)
{
    pipe_t* pipe = (pipe_t*)p;
    int next = c->writer + 1;
    if (next >= c->maxLen)
        next = 0;
    // an einai gematos
    while (next == c->reader){
        Cond_Signal(&(pipe->wait_for_read));
	Cond_Wait(&kernel_mutex,&(pipe->wait_for_write));
  }
 
 Cond_Broadcast(&(pipe->wait_for_read));
 
    c->Pipe_Buffer[c->writer] = data;
    c->writer = next;
    return 1;
}

int BufPop(Pipe_Buffer_t *c, char *data,  void* p)
{
  pipe_t* pipe = (pipe_t*)p;
    //an einai adeios o buffer
    while (c->writer == c->reader){
         Cond_Signal(&(pipe->wait_for_write));
	Cond_Wait(&kernel_mutex,&(pipe->wait_for_read));
  
 }
 Cond_Broadcast(&(pipe->wait_for_write));

    *data = c->Pipe_Buffer[c->reader];
    c->Pipe_Buffer[c->reader] = 0;  // clear the data (optional)
 
    int next = c->reader + 1;
    if(next >= c->maxLen)
        next = 0;
 
    c->reader = next;
 
    return 1;
}

/*******************************************************************************************************/

pipe_t* allocate_pipe()
{
pipe_t* ptr = (pipe_t*)malloc(sizeof(pipe_t));
 return ptr;
}


int pipe_close(void *p) 
{ 
pipe_t* pipe = (pipe_t*)p; 
FCB* fcb1=get_fcb(pipe->write);
FCB* fcb2=get_fcb(pipe->read);


if(pipe->flag==0){
   if(fcb1!=NULL && fcb2!=NULL){
      if(fcb1->refcount==0){
        pipe->flag++;
      }
      else if(fcb2->refcount==0){
        pipe->flag++;
      }
   }
}
if(pipe->flag==1){
   if(fcb1==NULL){
      if(fcb2->refcount==0){
         pipe->flag++;
         free(pipe);
      }
   }
   else if(fcb2==NULL){
      if(fcb1->refcount==0){
         pipe->flag++;
         free(pipe);
      }
   }
}
return 0; 
}


int pipe_read_w()
{return -1;}

int pipe_write_r()
{return -1;}


int pipe_write_w(void* p, const char* buf, unsigned int size)
{

pipe_t* pipe = (pipe_t*)p;
char* buffer=(char*)buf;
uint count = 0;
int success=-1;

FCB* fcb=get_fcb(pipe->read);

if(fcb==NULL) {

 return -1;
}

  while(count < size) {
     success = BufPush(pipe->p_buf, buffer[count],p );

    if(success) {
      count++;
    } 
   

}
  
  return count;  

}

int pipe_read_r(void* p,  char* buf, unsigned int size)
{
  pipe_t* pipe = (pipe_t*)p;
  uint count =  0;
  int success=-1;
FCB* fcb=get_fcb(pipe->write);

if(fcb == NULL){
     if(pipe->p_buf->writer == pipe->p_buf->reader){
 return 0;
}
}


while(count < size) {
     success = BufPop(pipe->p_buf, &buf[count],p );

    if(success) {
      count++;
    } 
    
  }


  return count;
}


file_ops pipe_read_fops = {
  .Open = NULL,
  .Read = pipe_read_r,
  .Write = pipe_write_r,
  .Close = pipe_close
};

file_ops pipe_write_fops = {
  .Open = NULL,
  .Read = pipe_read_w,
  .Write = pipe_write_w,
  .Close = pipe_close
};


/***************************************************************************************************/
int Pipe(pipe_t* pipe)
{

FCB* fcb_r;
FCB* fcb_w;

Pipe_Buffer_t * p_buf_t=(Pipe_Buffer_t *)malloc(sizeof(Pipe_Buffer_t));
if (p_buf_t==NULL)
goto finerr;

//Find available fcb's one for read and one for write
if(! FCB_reserve(1, &(pipe->read), &fcb_r))
      goto finerr;
if(! FCB_reserve(1, &(pipe->write), &fcb_w))
      goto finerr;
if(pipe->read >= MAX_FILEID)
      goto finerr;
if(pipe->write >= MAX_FILEID)
      goto finerr;

pipe->wait_for_read=COND_INIT;
pipe->wait_for_write=COND_INIT;
pipe->p_buf=p_buf_t;

char Buffer[50000];
pipe->p_buf->Pipe_Buffer = Buffer;
pipe->p_buf->reader=0;
pipe->p_buf->writer=0;
pipe->p_buf->maxLen=50000;
pipe->flag=0;

//Match fcb with pipe and file_ops 
fcb_r->streamobj=pipe;
fcb_w->streamobj=pipe;
fcb_r->streamfunc=&pipe_read_fops;
fcb_w->streamfunc=&pipe_write_fops;


goto finok;

finerr:
return -1;

finok:
return 0;
}

