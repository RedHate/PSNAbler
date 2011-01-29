#define _PSP_FW_VERSION 150
#include <pspkernel.h>
#include <pspkerneltypes.h>
#include <pspmoduleinfo.h>
#include <pspiofilemgr.h>
#include <pspmodulemgr.h>
#include <pspthreadman.h>
#include <pspwlan.h>
#include <stdlib.h>
#include <pspchnnlsv.h>
#include <pspctrl.h>
#include <string.h>
#include <pspctrl_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspumd.h>
#include "crt0_prx.h"

PSP_MODULE_INFO("psnabler", 0x3007, 1, 2);
PSP_MAIN_THREAD_ATTR(0); 

SceUID thid;
char gameId[10];
int running=0;

unsigned int version;
unsigned int *patches[9];

//Functions
int module_start(SceSize args, void *argp) __attribute__((alias("_start")));
int module_stop(SceSize args, void *argp) __attribute__((alias("_stop")));

//thanks again SANiK pluck!
unsigned int char2hex(unsigned char *a_data, unsigned int *a_type)
{
  unsigned int dword=0;
  unsigned int power=0;
  
  while(power < 8){
    switch(a_data[power]){
      case '_': break; //Freeze cheat
      case '0': dword|=0 * (1 << (4*(7-power))); break;
      case '1': dword|=1 * (1 << (4*(7-power))); break;
      case '2': dword|=2 * (1 << (4*(7-power))); break;
      case '3': dword|=3 * (1 << (4*(7-power))); break;
      case '4': dword|=4 * (1 << (4*(7-power))); break;
      case '5': dword|=5 * (1 << (4*(7-power))); break;
      case '6': dword|=6 * (1 << (4*(7-power))); break;
      case '7': dword|=7 * (1 << (4*(7-power))); break;
      case '8': dword|=8 * (1 << (4*(7-power))); break;
      case '9': dword|=9 * (1 << (4*(7-power))); break;
      case 'a':case 'A': dword|=0xA * (1 << (4*(7-power))); break;
      case 'b':case 'B': dword|=0xB * (1 << (4*(7-power))); break;
      case 'c':case 'C': dword|=0xC * (1 << (4*(7-power))); break;
      case 'd':case 'D': dword|=0xD * (1 << (4*(7-power))); break;
      case 'e':case 'E': dword|=0xE * (1 << (4*(7-power))); break;
      case 'f':case 'F': dword|=0xF * (1 << (4*(7-power))); break;
      default: dword>>=4*(8-power); *a_type=power; return dword;
    }
    power++;
  }
  
  *a_type=8;
  return dword;
}

void getVersion()
{
	int fd=sceIoOpen("ms0:/seplugins/psnabler.txt", PSP_O_RDONLY, 0777); 
	if(fd > 0)
	{
		sceIoRead(fd, &version, 4);
		sceIoClose(fd);
	}
	else
	{
		sprintf(&version, "06.2");
	}
}

char buf[64];

int loadModule(char buffer[])
{
	
	unsigned int counter = 0;
	unsigned int offset = 0;
	unsigned int a_type;
	
	char path[64];
	
	sprintf(path, "ms0:/seplugins/psnabler/%s.bin", buffer);
	
	int fd=sceIoOpen(path, PSP_O_RDONLY, 0777); 
	
	if(fd > 0)
	{
		sceIoRead(fd, buf, 64);
		sceIoClose(fd);
		
		char key[64]={"redhatesyounoobiesdontbesnoopingididyouafavornowyouwannafuckmeee"};
		key[65]="\0";

		int x;
		for(x=0; x<63; x++)
		{
			buf[x]=(~buf[x]^~key[x]);
		}

		patches[0]=char2hex(&buf[0], &a_type);
		patches[1]=char2hex(&buf[8], &a_type);
		patches[2]=char2hex(&buf[16], &a_type);
		patches[3]=char2hex(&buf[24], &a_type);
		patches[4]=char2hex(&buf[32], &a_type);
		patches[5]=char2hex(&buf[40], &a_type);
		patches[6]=char2hex(&buf[48], &a_type);
		patches[7]=char2hex(&buf[56], &a_type);
		
		return 1;
	}
	else
	{
		return 0;
	}
}

void apply(unsigned int ptr, unsigned int ptrVal)
{
	if(*((unsigned int*)(ptr+0x08800000)) == ptrVal+0x08800000)
	{
		*(unsigned int*) (ptrVal+0x08800104) = version;
		*(unsigned int*) (ptrVal+0x0880014C) = version;
	}
}

void getId()
{
  signed int fd;
  
  do
  {
  	fd=sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777); 
    sceKernelDelayThread(10000);
  } while(fd<=0);
  sceIoRead(fd, gameId, 10);
  sceIoClose(fd);
}

int mainThread()
{

  running=1;
  
  sceKernelDelayThread(15000000);
  while(!sceKernelFindModuleByName("sceKernelLibrary"))
  sceKernelDelayThread(100000);
  sceKernelDelayThread(100000);

  getId();
  
  if(!loadModule(gameId))
  {
	running=0;
  }
  else
  { 
	getVersion();
  }

  while(running)
  {
	apply(patches[0], patches[1]);
	apply(patches[2], patches[3]);
	apply(patches[4], patches[5]);
	apply(patches[6], patches[7]);
	sceKernelDelayThread(15000);
  }
  
  return 0;
  
}

int _start(SceSize args, void *argp)
{
	thid=sceKernelCreateThread("psnabler", &mainThread, 0x18, 0x500, 0, NULL);
	if(thid >= 0) sceKernelStartThread(thid, 0, NULL);
	return 0;
}

int _stop(SceSize args, void *argp)
{
	running=0;
	sceKernelTerminateThread(thid);
	return 0;
}
