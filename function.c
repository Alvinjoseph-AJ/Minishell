/*Alvin Joseph
 */

#include "minishell.h"
#include<string.h>

extern char input_string[30];
char *external_commands[200];
//int last_exit_status = 0; // store last child exit status
//pid_t fg_child = 0;
int pid = 0;
int status;

Slist *head = NULL;

void handler(int signum)
{
    if(signum==SIGINT)
    {
	if(pid==0)//no child is present
	{
	    printf(YELLOW"\nminishell$: "RESET);
	    fflush(stdout);
	}
    }
    else if(signum==SIGTSTP)
    {

	if(pid==0)//no child is present
	{
	    printf(YELLOW"\nminishell$: "RESET);
	    fflush(stdout);
	}
	else
	{
	    if(input_string[0]!='\0')
	    {
		if(insert_at_first(&head,input_string))
		{
		    printf("\n[%d]+   Stopped   %s\n",pid,input_string);
		}
		else
		{
		    printf("add failed\n");
		}
	    }
	}
    }
}

int insert_at_first(Slist **head,char *cmd)
{
    Slist *node =malloc(sizeof(Slist));
    if(node == NULL)
    {
	return 0;
    }

    node->child_pid = pid;
    strcpy(node->command,cmd);
    node->link=NULL;
    if(*head==NULL)
    {
	*head=node;
    }
    else
    {
	node->link=*head;
	*head = node;
    }
    return 1;
}

void print_list(Slist **head)
{
    Slist *temp = *head;
    if(temp==NULL)
    {
	return;
    }
    while(temp->link!=NULL)
    {
	printf("[%d]+   Stopped  %s\n",temp->child_pid,temp->command);
	temp = temp->link;
    }	
    printf("[%d]+   Stopped  %s\n",temp->child_pid,temp->command);
}

void scan_input(char *prompt, char *input)
{
    int ch;

    extract_external_commands(external_commands);

    signal(SIGINT,handler);
    signal(SIGTSTP,handler);
    while(1)
    {
	printf(YELLOW"%s"RESET,prompt);

	scanf("%[^\n]",input);
	ch=getc(stdin);
	if(strncmp(input,"PS1=",4)==0)
	{
	    if(strstr(input," ")==NULL)
	    {
		strcpy(prompt,input+4);
	    }
	}
	else
	{
	    char *cmd = get_command(input);
	    //printf("command is = %s\n",cmd);
	    int res = check_command_type(cmd);
	    //printf("The command return is %d\n", res);
	    if(res==BUILTIN)//builtin
	    {
		if(strcmp(cmd,"exit")==0)
		{
		    exit(0);
		}
		if(strcmp(cmd,"pwd")==0)
		{
		    char cwd[1024];

		    if (getcwd(cwd, sizeof(cwd)) != NULL) {
			printf("%s\n", cwd);
		    } else {
			perror("getcwd");
			return;
		    }
		}

		if(strcmp(cmd,"cd")==0)
		{
		    char *path = input + 3;
		    chdir(path);
		}

		if(strcmp(cmd,"echo")==0)
		{
		    if(strcmp(input,"echo $$")==0)
		    {
			printf("%d\n", getpid());
		    }
		    if(strcmp(input,"echo $?")==0)
		    {
			int status;
			wait(&status);
			printf("%d\n", WEXITSTATUS(status));
		    }
		    if(strcmp(input,"echo $SHELL")==0)
		    {
			printf("%s\n", getenv("SHELL"));
		    }
		}
		if(strcmp(cmd,"jobs")==0)
		{
		    print_list(&head);
		}
		if(strcmp(cmd,"fg")==0)
		{
		    Slist * temp = head;
		    if(temp!= NULL)
		    {
			printf("%s\n",temp->command);
			int fg_id = temp->child_pid;
			pid = fg_id;
			kill(fg_id,SIGCONT);
			waitpid(fg_id,&status,WUNTRACED);
			pid = 0;
			delete_first();
		    }
		    else
		    {
			printf("bash: fg: current: no such job\n");
		    }
		}
		if(strcmp(cmd,"bg")==0)
		{ 
		    if(head!=NULL)
		    {
			printf("[%d]+  %s &\n",head->child_pid,head->command);	
			int bg_id = head->child_pid;
			kill(bg_id,SIGCONT);
			delete_first();
		    }
		    else
		    {
			printf("bash: bg: current: no such job\n");
		    }
		}
	    }
	    if(res==EXTERNAL)
	    {
		execute_external_commands(input,cmd);
	    }
	    if(res==NO_COMMAND)
	    {
		printf("command not available\n");
	    }
	} 
    }
}

void delete_first()
{
    if(head==NULL)
	return;
    Slist * temp = head;
    head = temp->link;
    free(temp);
}
char *get_command(char *input)
{
    static char command[20];
    int i=0;
    while(input[i]!=' ' && input[i]!='\0')
    {
	command[i]=input[i];
	i++;
    }
    command[i]='\0';
    return command;
}


void extract_external_commands(char **external_commands)
{
    char ch;
    char buffer[100]; 
    int fd = open("external_command.txt",O_RDONLY);
    int w_c = 0;
    int index=0;
    if(fd==-1)
    {
	perror("error");
	return;
    }
    while(read(fd,&ch,1)==1)
    {
	if(ch==' ' || ch=='\n')
	{
	    if(w_c>0)
	    {
		buffer[w_c]='\0';
		external_commands[index]=malloc((strlen(buffer)+1)*sizeof(char));
		strcpy(external_commands[index],buffer);
		index++;
		w_c=0;
	    }
	}
	else
	{
	    buffer[w_c++]=ch;
	}
    }
    if (w_c > 0)
    {
	buffer[w_c] = '\0';
	external_commands[index] = malloc(strlen(buffer) + 1);
	strcpy(external_commands[index], buffer);
	index++;
    }

    external_commands[index] = NULL;
    close(fd);
}


int check_command_type(char *command)
{
    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
	"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
	"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", "jobs","fg","bg"};

    int j=0,k=0;

    if(command[0]=='\n')
    {
	return 4;
    }
    while (external_commands[j] != NULL || builtins[k] != NULL)
    {
	if (external_commands[j] != NULL)
	{
	    if (strcmp(command, external_commands[j]) == 0)
		return EXTERNAL;
	    j++;
	}

	if (builtins[k] != NULL)
	{
	    if (strcmp(command, builtins[k]) == 0)
		return BUILTIN;
	    k++;
	}
    }
    return NO_COMMAND;
}


void execute_external_commands(char *input_strings, char *command)
{
    char *args[100];
    char buffer[100];
    int i=0,j=0,index=0;
    int pipe_position[10]={0};
    int pipe_count=0;
    while(input_strings[i])
    {
	if(input_strings[i]==' ' || input_strings[i]=='\n')
	{
	    if(j>0)
	    {
		buffer[j]='\0';
		args[index] = malloc(strlen(buffer) + 1);
		strcpy(args[index], buffer);
		index++;
		j=0;
	    }
	}
	else
	{
	    buffer[j++]=input_strings[i];
	}
	i++;
    }

    if(j>0)
    {
	buffer[j]='\0';
	args[index] = malloc(strlen(buffer) + 1);
	strcpy(args[index], buffer);
	index++;
    }
    args[index]=NULL;
    for(int p=0;p<index;p++)
    {
	if(strcmp(args[p],"|")==0)
	{
	    args[p]=NULL;
	    pipe_position[pipe_count++]=p;
	}
    }
    if(pipe_count>0)
    {
	//printf("pipe is present\n");
	int fd[pipe_count][2];
	for(int p=0;p<pipe_count;p++)
	{
	    pipe(fd[p]);
	}
	int start = 0;
	for(int p=0;p<=pipe_count;p++)
	{
	    pid = fork();
	    if(pid==0)
	    {
		if(p>0)
		{
		    dup2(fd[p-1][0],0);
		}
		if (p < pipe_count) {
		    dup2(fd[p][1],1);
		}
		for (int k = 0; k < pipe_count; k++) {
		    close(fd[k][0]);
		    close(fd[k][1]);
		}
		char *cmd_args[20];
		int idx = 0;
		int end = (p == pipe_count) ? index : pipe_position[p];
		for (int q = start; q < end; q++)
		{
		    cmd_args[idx++] = args[q];
		}
		cmd_args[idx] = NULL;

		execvp(cmd_args[0], cmd_args);
		perror("execvp failed");
		exit(1);
	    }

	    start = pipe_position[p] + 1;
	}
	for (int p = 0; p < pipe_count; p++) {
	    close(fd[p][0]);
	    close(fd[p][1]);
	}
	for (int p = 0; p < pipe_count+1; p++) {
	    wait(NULL);
	}
	printf("\n");
    }
    else
    {
	pid = fork();
	if(pid==0)
	{
	    signal(SIGINT,SIG_DFL);
	    signal(SIGTSTP,SIG_DFL);
	    execvp(command,args);
	    fflush(stdout);
	    exit(1);
	}
	else if(pid>0)
	{
	    int status;
	    waitpid(pid, &status, WUNTRACED);
	    pid =0;
	}
	else
	{
	    perror("fork");
	}
    }
}
