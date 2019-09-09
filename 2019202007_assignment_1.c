
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER  1000

char* readinput();
char** tokenize(char* buffer);
void exec_cmd(char** inputarray);
int exec_pipe(char** first, char** second);
void cd(char** path);
void echo(char** str);
int ii=0;
char** history;

int main(int argc,char*argv[])
 {
  printf("\033[H\033[J");  //ansi escape code to clear screen when shell opens
  printf("*****************MANIK SHELL**************************\nCommands implemented:\n1.echo\n2.cd\n3.Binary commands\n4.Multipipe\n5.Redirection\n");
  while(1) {
   
    char cwd[1000];
    getcwd(cwd,sizeof(cwd));
  
    printf("manik@Shell:~%s$",cwd);
    char* input = readinput();
    char** inputArray = tokenize(input);

     if(!strcmp("quit",inputArray[0])) break;  //if arg[0] is quit, break from loop and close shell
    exec_cmd(inputArray);
  }
  return 1;
}


char* readinput()
 {
  //read till end of input or newline
   char *buffer = malloc(sizeof(char) * BUFFER);
   int counter=0;
   char c;
  
    while (1)
    {
     if(counter>=BUFFER) 
     {
      printf("input exceeding buffer\n");
      break;
     }
     c = getchar();
      if (c!=EOF && c!= '\n') 
      {
        if(c=='|' || c=='>'  || c== '<')    //insert delims before and after | so that tokenzer can separate them
        {
          buffer[counter]=' ';
          counter++;
          buffer[counter]=c;
          counter++;
          buffer[counter]=' ';
        }
         else
         buffer[counter] = c;
      } 
      else
      {
       buffer[counter] = '\0';    // \0 in end so tokenizer knows where to stop
      return buffer;
      }
     counter++;
  }
 return buffer;
}

char** tokenize(char* buffer)
 {

  // Split input into tokens based on spaces
  char** tokenarray = malloc(sizeof(char) * BUFFER);
  int i = 0;
  char* token = strtok(buffer, " ");  //returns pointer to first substring and NULL when no more tokens

  while(token != NULL)
   {              
    tokenarray[i] = token;
    token = strtok(NULL, " ");   //NULL pointer as first argument to signal strtok to search in same string
    i++;
  }
  tokenarray[i] = NULL;

  return tokenarray;
 }

void exec_cmd(char** inputarray) 
{
    int i = 0;
      int j,in,out;
  int rdin = 0;
  int rdout = 0;

  while(inputarray[i] != NULL)
   {
  if(!strcmp(inputarray[i],"<"))
   {
      in = open(inputarray[i+1], O_RDONLY,00444);  //redonly opens for read only
      inputarray[i] = NULL;
      rdin = 1;
    }
    else if(!strcmp(inputarray[i], ">"))
     {
      out = open(inputarray[i+1], O_WRONLY | O_TRUNC | O_CREAT, 00777);   //wronly opens for write only, trunc changes lt to 0, creat makes file of not

      inputarray[i] = NULL;
      rdout = 1;
    } 
    else if(!strcmp(inputarray[i], ">>")) 
    {
      out = open(inputarray[i+1], O_WRONLY | O_APPEND | O_CREAT, 00777);
      inputarray[i] = NULL;
      rdout = 1;
    }
    else if(!strcmp(inputarray[i], "|")) 
    {
      inputarray[i] = NULL;
      exec_pipe(&inputarray[0], &inputarray[i+1]);
      return;   
    }
    i++;
  }

    if(!strcmp(inputarray[0],"echo")) { 
      echo(inputarray);    
    }

    else if(!strcmp(inputarray[0],"cd")){ 
      cd(inputarray);
    }

  else 
  {
    pid_t pid;
    if((pid=fork()) < 0) 
    {
      printf("Fork errror\n");
    }
    else if(pid==0)
     {
      //child
      if(rdin==1) 
      {
        dup2(in, STDIN_FILENO);
      }
      if(rdout == 1)
       {
        dup2(out, STDOUT_FILENO);
      }
     
      execvp(inputarray[0], inputarray);  //no redirection, run command from bin 
      printf("Command %s not found.\n", inputarray[0]);
    }
    else 
    {
        wait(NULL);
    }
  }
  
}


int exec_pipe(char** first, char** second) 
{
  pid_t p1, p2;
   int fd[2];
  pipe(fd);

  if( (p1 = fork()) < 0) 
  {
    printf("Fork error\n");
  }
  else if(p1 == 0) {
   //child
    dup2(fd[1], STDOUT_FILENO);
    close(fd[0]);
    exec_cmd(first);
    exit(0);
  }
  else {
//parent
    
    if((p2 = fork() )< 0) 
    {
      printf("Fork error\n");
    }
    else if(p2 == 0) {
        //child
      dup2(fd[0], STDIN_FILENO);
     
       close(fd[1]);
      exec_cmd(second);
      exit(0);
    }
    else {
   //parent
      close(fd[0]);
      close(fd[1]);
    
      waitpid(p1,NULL,0);  //waiting for childrn wait(pid,status,options)
      waitpid(p2,NULL,0);
    }
  }
  return 1;
}




void cd(char** path)
 { int i= chdir(path[1]);
 
   if(i==-1) 
  {
    printf("%s directory doesnt exist.\n", path[1]);
  }
 }
void echo(char** str)
 {
  for(int i=0;str[i+1]!=NULL;i++)
  {
     printf("%s ", str[i+1]);
  }
   printf("\n");
 }