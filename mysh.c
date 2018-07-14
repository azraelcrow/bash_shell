/*
CS 4348.003 Project 1
Lucas Castro
Compiled with gcc on the cs1 UTDallas linux server
*/


#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// main function declarations
int rpe_cd(char **args);
int rpe_exit(char **args);
int rpe_history(char **args);

// string representations for builtin functions
char *builtin_str[] = { "cd","history","exit" };

// list of builtin references
int (*builtin_func[]) (char **) = {
  &rpe_cd,
  &rpe_history,
  &rpe_exit
};

// abstract the number of builtin functions
int rpe_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/******************************************
 * cd command 
*******************************************/
// implementation of cd command
int rpe_cd(char **args)
{
  // if no argument is given, print error message
  if (args[1] == NULL) 
  {
    fprintf(stderr, "rpe: expected argument to \"cd\"\n");
  } 
  else 
  { // else, change directory to args[1]
    if (chdir(args[1]) != 0) 
    {
      perror("rpe");
    }
  }
  return 1;
}
/******************************************
 * end cd command 
*******************************************/


/******************************************
 * exit command 
*******************************************/
//exit command
int rpe_exit(char **args)
{
 // just return a 0 to exit
  return 0;
}
/******************************************
 * end exit command 
*******************************************/

/***********************************************
 * line parse function
 * *********************************************/
#define RPE_TOK_BUFFERSIZE 64
#define RPE_TOK_DELIMITERS " \t\r\n\a"

// argument parser function
char **rpe_split_line(char *line)
{
  int size = RPE_TOK_BUFFERSIZE, position = 0;
  char **tokens = malloc(size * sizeof(char*));
  char *token;

  // if allocation for tokens fails, print error and exit
  if (!tokens) 
  {
    fprintf(stderr, "rpe: allocation error\n");
    exit(EXIT_FAILURE);
  }
  
  // tokenize the input string and assign it to token array 
  token = strtok(line, RPE_TOK_DELIMITERS);

  // place strings in token at positions in tokens 
  while (token != NULL) {
    tokens[position] = token;
    position++;
    
    // if the end of the array is reached, reallocate
    if (position >= size) {
      size += RPE_TOK_BUFFERSIZE;
      tokens = realloc(tokens, size * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "rpe: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    
    // append null character to token
    token = strtok(NULL, RPE_TOK_DELIMITERS);
  }
  tokens[position] = NULL;
  return tokens;
}

/***********************************************
 * end line parse function
 * *********************************************/

/******************************************
 * History command functions
*******************************************/
#define HISTORY_LIST_NUMBER 100 // number of commands in history array
int num_command = 0; // number of command, for history
char *hist_list[HISTORY_LIST_NUMBER]; // array to hold command history


// add input line to hist_list
void rpe_add_history(char *command, int i)
{
  if(i > -1)
  {
    hist_list[i] = command;
  }
  else
  {
    int k = 0;
    do
    {
      // if the entry exists, print it
      if(hist_list[k] != NULL)
      {
       printf("%4d   %s\n",k, hist_list[k]);
      }
      k = (k + 1) % HISTORY_LIST_NUMBER;
    } while(k <= num_command); 
  }
}

// main history command
int rpe_history(char **args)
{
  // if there is an argument 
  if(args[1] != NULL)
  {
    // if history is called with the -c option
    if(strcmp(args[1],"-c") == 0)
    {
      int i = 0;
      // reset all entries in hist_list to NULL
      while(hist_list[i] != NULL)
      {
        hist_list[i] = NULL;
        i++;
      }
      num_command = 0; //reset num_command 
    }
    // else, if args[1] is an integer and isn't 0, or if args[1] is 0 and the actual argument is 0
    else if((atoi(args[1]) != 0 || (atoi(args[1]) == 0 && args[1] == "0")) || strcmp(args[1], "0") == 0)
    {
      // if a number is entered that does not have a corresponding command in hist_list
      if(hist_list[atoi(args[1])] == NULL)
      {
        fprintf(stderr, "history: invalid command number\n");
      }
      else
      {
        int piping=0;
	char *pipeString[2];
	char **args1;
	char **args2;
        char lineCopy[HISTORY_LIST_NUMBER];
        strcpy(lineCopy, hist_list[atoi(args[1])]);
	piping = parsePipes(lineCopy, pipeString);
	if(!piping)
	{
          rpe_execute(rpe_split_line(lineCopy));    
	}
	else
	{  
          // pipe commands
          args1 = rpe_split_line(pipeString[0]);
          args2 = rpe_split_line(pipeString[1]);
          rpe_execute_piped(args1, args2);
	}
        // execute command by making a copy of the string stored at index args[1]
	// and sending it to the execute functions
      }
    }
    else
    {
      fprintf(stderr, "history: invalid option\n");
    }

  }
  else if(args[1] == NULL) // if history is called with no arguments
  {
    rpe_add_history("dummy", -1); // display all history
  }
  else
  {
    fprintf(stderr, "history: invalid option\n");
  }
  return 1;
}
/***********************************************
 * End history command functions
 * *********************************************/

/***********************************************
 * read input function
 * *********************************************/
#define RPE_RL_BUFFERSIZE 1024
char *rpe_read_line(void)
{
  int size = RPE_RL_BUFFERSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * size);
  int c;

  if (!buffer) {
    fprintf(stderr, "rpe: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= size) {
      size += RPE_RL_BUFFERSIZE;
      buffer = realloc(buffer, size);
      if (!buffer) {
        fprintf(stderr, "rpe: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

/***********************************************
 * End read function
 * *********************************************/

/***********************************************
 * Parse line for pipes 
 * *********************************************/
int parsePipes(char* line, char** pipeString)
{
  int i;
  for(i = 0; i < 2; i++)
  {
    pipeString[i] = strsep(&line, "|");
    if(pipeString[i] == NULL)
    {
      break;
    }
  }
  if(pipeString[1] == NULL)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
/***********************************************
 * End parse line for pipes 
 * *********************************************/

/******************************************
 * execution command functions
*******************************************/
// function to create child process to execute new commands
int rpe_launch(char **args)
{
  pid_t pid, wpid;
  int state;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("rpe");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("rpe");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &state, WUNTRACED);
    } while (!WIFEXITED(state) && !WIFSIGNALED(state));
  }

  return 1;
}

int rpe_execute(char **args)
{
  int i; //inline declarations not allowed

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }
  
  //run through the array and compare input to the builtin commands
  //if the command passed is a builtin, run this version of it
  for (i = 0; i < rpe_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  
  // the command wasn't a builtin, send it to the kernel for normal execution
  return rpe_launch(args); 
}

int rpe_execute_piped(char **args1, char **args2)
{
  printf("command 1: %s\n", args1[0]);
  printf("command 2: %s\n", args2[0]);

  int pipefd[2];
  pid_t p1, p2;
  if(pipe(pipefd) == 0)
  {
    perror("pipe");
  }
  p1 = fork();
  if (p1 < 0) {
          printf("\nCould not fork");
          return;
      }
 
      if (p1 == 0) {
          // Child 1 executing..
          // It only needs to write at the write end
        //  close(pipefd[1]);
          dup2(pipefd[1], /*STDOUT_FILENO*/1);
         // close(pipefd[1]);
 
          if (execvp(args1[0], args1) < 0) {
              printf("\nCould not execute command 1..");
              exit(0);
          }
      } else {
          // Parent executing
          p2 = fork();
   
          if (p2 < 0) {
              printf("\nCould not fork");
              return;
          }
 
          // Child 2 executing..
          // It only needs to read at the read end
          if (p2 == 0) {
             // close(pipefd[0]);
              dup2(pipefd[0], /*STDIN_FILENO*/0);
            //  close(pipefd[0]);
              if (execvp(args2[0], args2) < 0) {
                  printf("\nCould not execute command 2..");
                  exit(0);
              }
          } else {
              // parent executing, waiting for two children
              wait(NULL);
              //wait(NULL);
          }
      }
}

/******************************************
 * end execution command functions
*******************************************/


// read-parse-execute loop
void rpe_loop(void)
{
  char *line; // command
  char lineTwo[HISTORY_LIST_NUMBER];
  char **args; // parsed command

  char **args1; // for piping
  char **args2; // for piping

  int state; // success/failure
  int piping = 0; // does command use piping
  char* pipeString[2];
  do {
    printf("$");
    line = rpe_read_line(); // get line
    rpe_add_history(line, num_command); // add line to history array with the command number
    num_command++; // increment the command number
    strcpy(lineTwo, line);
    piping = parsePipes(lineTwo, pipeString); // separate the commands, piping takes 1 if there is a pipe and 0 if not  
    if(!piping)
    {
      args = rpe_split_line(lineTwo); // parse line
      state = rpe_execute(args); // execute command on args, assign its state to state
    }
    else
    {
    // pipe commands
      args1 = rpe_split_line(pipeString[0]);
      args2 = rpe_split_line(pipeString[1]);
      state = rpe_execute_piped(args1, args2);
    }
    // free memory
    //free(line);
    //free(args);
  } while (state);
}

// main function
int main(int argc, char **argv)
{ 
  // run read-parse-execute loop
  rpe_loop();
  return EXIT_SUCCESS;
}

