#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>
void
parse (char *cmd, char **args)
{
  while (*cmd != '\0')
    {
      while (*cmd == ' ' || *cmd == '\t' || *cmd == '\n')
	*cmd++ = '\0';
      *args++ = cmd;
      while (*cmd != ' ' && *cmd != '\0' && *cmd != '\t' && *cmd != '\n')
	cmd++;

    }
  *args = NULL;
}
int getToken(char **args,char *file)
{
  int i;
  int res=-1;
  for(i=0;args[i]!='\0';i++)
  {
    if(strcmp(args[i],">")==0)
    {
      res = 1;
      args[i] = NULL;
      strcpy(file,args[i+1]);
    }
    else if(strcmp(args[i],"<")==0)
    {
      res = 0;
      args[i] = NULL;
      strcpy(file,args[i+1]);
    }
    else if(strcmp(args[i],"|")==0)
    {
      return 2;
    }
  }
  return res;
}
int count_pipes(char **args)
{
  int i=0,count=0;
  while(args[i]!=NULL)
  {
    if(strcmp(args[i],"|")==0)
    {
      strcpy(args[i],NULL);
      count++;
    }
    i++;
  }
  return count;
}
void execute_pipe(char **args,int pipe[],int n,int i)
{
  if(i>n)
    return;
  if(fork()==0)
  {
      if(i==0)
        dup2(pipe[1],1);
      else if(i==n)
        dup2(pipe[2*i-2],0);
      else
      {
          dup2(pipe[2*i-2],0);
          dup2(pipe[2*i+2],1); 
      }
      int k=0;
      while(k<2*n)
      {
        close(pipe[k]);
        k++;
      }
      char **arg = args;
      while(*args!=NULL)
      {
        args++;
      }
      args++;
      execvp(*arg,arg);
  }
  else
  {
    execute_pipe(args,pipe,n,i+1);
  }
}
void
execute (char **args)
{
  //char *args[] = { "ls", "-l", NULL };
  pid_t pid;
  pid = fork ();
  char file[64];
  int status;
  int token = getToken(args,file);
  if (pid < 0)
    {
      perror ("Fork Failed");
      exit (-1);
    }
  else if (pid == 0)
    {
      if(token==0)//input
      {
        int in = open(file,O_RDONLY);
        dup2(in,0);
        close(in);
      }
      else if(token==1)//output
      {
        int out = open("out", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        dup2(out,1);
        close(out);
      }
      else if(token==2)
      {
        int no_pipes = count_pipes(args);
        int pipefd[2*no_pipes];
	int i;
        for(i=0;i<no_pipes;i++)
        {
          pipe(pipefd + 2*i);
        }
        execute_pipe(args,pipefd,no_pipes,0);
        int k=0;
        while(k<2*no_pipes)
        {
          close(pipefd[k]);
          k++;
        }
        k=0;
        while(k<(no_pipes+1))
        {
           wait(&status);
           k++;
        }
        return;
      }
      int status = execvp (*args, args);
      if (status == -1)
	{
	  perror ("Command Not Found!!\n");
	  exit (1);
	}
    }
  else
    {
      wait (NULL);
      //printf ("child complete\n");
      //exit (0);
    }
}
int
main ()
{
  char cmd[100];
  char *args[50];
  char cwd[64];
  int res;
  while (1)
    {
      printf ("Akshay's PC:");
      if (getcwd (cwd, sizeof (cwd)) != NULL)
	printf ("~%s# ", cwd);
      scanf ("%[^\n]%*c", cmd);
      if (*cmd == '\0')
	       continue;
      parse (cmd, args);
      if (strcmp (args[0], "quit") == 0)
	exit (0);
      else if (strcmp (args[0], "cd") == 0)
	{
	  res = chdir (args[1]);
	  if (res != 0)
	    printf ("Error: ");
	}
      else
	execute (args);
    }
printf("linuxminishell");
}
