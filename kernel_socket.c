
#include "tinyos.h"
#include "kernel_streams.h"

#include "kernel_dev.h"
#include "util.h"
#include "kernel_proc.h"
#include "kernel_cc.h"
#include <assert.h>
#include <stdlib.h>


PRT* PORT[MAX_PORT];

Fid_t Socket(port_t port)
{
FCB* fcb;

sock_t * sockets=(sock_t *)malloc(sizeof(sock_t));
if (sockets==NULL)
goto finerr;


if(! FCB_reserve(1, &(sockets->sock), &fcb) )
      goto finerr;

if(PORT==NOPORT)
     goto finerr;

if(port>MAX_PORT)
     goto finerr;

sockets->port=PORT[port];
sockets->port->sock=sockets->sock;
sockets->port->lsock=-1;
sockets->lsock=-1;
fcb->streamobj=sockets;
PORT[port]->port=port;
PORT[port]->sock=sockets->sock;
PORT[port]->lsock=-1;

goto finok;

finerr:
	return NOFILE;

finok:

        return (sockets->sock);
}


int Listen(Fid_t sock)
{

FCB* fcb=get_fcb(sock);

if(fcb==NULL)
  goto finerr;

sock_t* socket=(sock_t*)fcb->streamobj;

if((socket->port->sock)!=sock)
  goto finerr;

if(sock<0 || sock>=MAX_FILEID)
  goto finerr;

FCB* lfcb=get_fcb(socket->port->lsock);

if(lfcb!=NULL)
  goto finerr;

socket->port->lsock=sock;
socket->lsock=sock;

goto finok;

finerr:
	return -1;

finok:
        return 0;
}


Fid_t Accept(Fid_t lsock)
{

if(lsock<0 || lsock>=MAX_FILEID)
  goto finerr;

FCB* fcb=get_fcb(lsock);

if(fcb==NULL)
  goto finerr;




goto finok;

finerr:
	return NOFILE;

finok: 
        return ;
}


int Connect(Fid_t sock, port_t port, timeout_t timeout)
{

FCB* fcb=get_fcb(sock);
sock_t* socket=(sock_t*)fcb->streamobj;


if(fcb==NULL)
   goto finerr;

if(socket->port==NULL || socket->lsock!=sock)
  goto finerr;

if(port==NOPORT)
     goto finerr;

if(port>MAX_PORT)
     goto finerr;

FCB* lfcb=get_fcb(PORT[port]->lsock);

if(lfcb==NULL)
  goto finerr;

sock_t* port_socket=(sock_t*)lfcb->streamobj;

socket->pipe->read=socket->sock;
port_socket->pipe->read=port_socket->sock;
socket->pipe->write=port_socket->pipe->write;


goto finok;


finerr:
	return -1;

finok:
        return 0;
}


int ShutDown(Fid_t sock, shutdown_mode how)
{
	return -1;
}

