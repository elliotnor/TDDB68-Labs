#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"


static void syscall_handler (struct intr_frame *);
void assert(bool expression);
bool validPointer(void *esp);
bool validString(char *cmd_line);
bool validBuffer(void *buffer, unsigned size);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void halt(void){
  power_off();
}

bool create (const char *file, unsigned initial_size){
  assert(initial_size >= 0);
  assert(validString(file));
  return filesys_create (file, initial_size);
}
int open (const char *file){
  assert(validString(file));
  struct file *f = filesys_open(file);
  struct thread *thread = thread_current();
  
  if(thread == NULL || f == NULL){
    return -1;
  }

  for(int i = 2; i < 130; i++){
    if(thread->fd_list[i] == NULL){
      thread->fd_list[i] = f;
      return i;
    }
  }
  file_close(file);
  return -1;
}

void close (int fd){
  assert(fd > 1 && fd < 130);
  struct thread *thread = thread_current();
  if(thread == NULL){
    return;
  }
  for(int i = 2; i < 130; i++){ 
    if(i == fd){
      file_close(thread->fd_list[i]);
      thread->fd_list[i] = NULL;
      return;
    }
  }
  return;
}

int read (int fd, char *buffer, unsigned size){
  assert(fd >= 0 && fd < 130);
  assert(size >= 0);
  assert(validBuffer(buffer, size));
  struct thread *thread = thread_current();
  struct file *file = thread->fd_list[fd];
  int counter = 0;
  int bytesRead = 0;
  if(fd == 0){
    while(counter < size){      
      buffer[counter] = input_getc();
      counter++;
    }
    return counter;
  }
  else{
    if(fd > 0 && file != NULL){
      bytesRead = file_read(file, buffer, size); 
      return bytesRead;
    }
    else{
      return -1;
    
      
    }
  }
}


int write (int fd, const char *buffer, unsigned size){
  assert(validBuffer(buffer, size));
  assert(fd > 0 && fd < 130);
  struct thread *thread = thread_current();
  struct file *file = thread->fd_list[fd];
  int bytesWritten;
  if(fd == 1){
      putbuf(buffer, size);
      bytesWritten = size;
  }
  else{
    bytesWritten = file_write(file, buffer, size); 
  }
  return bytesWritten;
}
tid_t exec(const char *cmd_line){
  assert(validPointer(cmd_line));
  tid_t tid;
  tid = process_execute(cmd_line);
  if (tid == TID_ERROR)
    return -1;
  return tid;
}

void exit (int status){
  printf("%s: exit(%d)\n",thread_current()->name, status);
  thread_current()->parentChild->exit_status = status;
  thread_exit();
  
}

int wait(tid_t child_tid){
  return process_wait(child_tid);
}

void seek (int fd, unsigned position) {
  assert(fd >= 0 && fd < 130);
  struct file * file = thread_current()->fd_list[fd];
  assert(file != NULL);
  file_seek(file, position);
}

unsigned tell (int fd) {
  assert(fd >= 0 && fd < 130);
  struct file * file = thread_current()->fd_list[fd];
  assert(file != NULL);
  return file_tell(file);
}

int filesize (int fd) {
  assert(fd >= 0 && fd < 130);
  struct file * file = thread_current()->fd_list[fd];
  assert(file != NULL);
  return file_length(file);
}

bool remove (const char *file_name) {
  assert(validString(file_name));
  return filesys_remove(file_name);
}

void assert(bool expression){
  if(!expression)
    exit(-1);
}

bool validPointer(void *esp){
  if(esp != NULL && is_user_vaddr(esp) && pagedir_get_page(thread_current()->pagedir, esp) != NULL){
    return true;
  }
  return false;
}

bool validString(char *cmd_line){
  if (!(validPointer(cmd_line)))
    return false;
  unsigned index = 0;
  while (((cmd_line[index])) != '\0'){
    if (!validPointer(cmd_line + index))
      return false;
    index++;  
  }
  return true;

}

bool validBuffer(void *buffer, unsigned size){
  unsigned index = 0;
  while (index < size)
  {
    if (!validPointer(buffer + index))
      return false;
    index++;
  }
  return true;
  }


static void
syscall_handler (struct intr_frame *f) 
{
  assert(validPointer(f->esp));
  assert(validPointer(f->esp+4));
  assert(validPointer(f->esp+8));
  assert(validPointer(f->esp+12));

  switch (*(int*) (f->esp)){

    case SYS_EXIT:
      exit(*(int*) (f->esp+4));
      break;

    case SYS_HALT:
      halt();
      break;

    case SYS_CREATE:
      f->eax = create(*(char**) (f->esp+4), *(unsigned*) (f->esp+8));
      break;
    
    case SYS_OPEN:
      f->eax = open(*(char**) (f->esp+4));
      break;
    
    case SYS_CLOSE:
      close(*(int*) (f->esp+4));
      break;

    case SYS_READ:
      f->eax = read(*(int*) (f->esp+4), *(void**) (f->esp+8), *(unsigned*) (f->esp+12));
      break;

    case SYS_WRITE:
      f->eax = write(*(int*) (f->esp+4), *(void**) (f->esp+8), *(unsigned*) (f->esp+12));
      break;

    case SYS_EXEC:
      f->eax = exec(*(char**) (f->esp+4));
      break; 

    case SYS_WAIT:
      f->eax = wait(*(tid_t*) (f->esp+4)); 
      break;

    case SYS_SEEK:
      seek(*(int*)(f->esp+4), *(unsigned*)(f->esp+8));
      break;

    case SYS_TELL:
      f->eax = tell(*(int*)(f->esp+4));
      break;

    case SYS_FILESIZE:
      f->eax = filesize(*(int*)(f->esp+4));
      break;

    case SYS_REMOVE:
      f->eax = remove(*(const char**)(f->esp+4));
      break;
  }
}
