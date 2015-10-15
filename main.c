/*
   |============================================================|
   |	  	Exercise #  :	 1				|
   |								|
   |   	   	File nam	:	main.c			|
   |		Date		:	6/4/2014		|
   |		Author    	:	Shai Pe'er		|
   |		Email     	:	shaip86@gmail.com 	|
   |============================================================|
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

enum  errorAndAlertCodes{EXECVP_ERROR = 255,	NO_GREATRE_SIGN_LOCATION = -1, FORK_ERROR = 10, OPEN_FILE_ERROR = 11, DUP2_ERROR = 12, CLOSE_FILE_ERROR = 13, SIGNAL_KILL_CHILD = 14, WAIT_ERROR = 15};

static int   greaterSignLoc;	//represent the location of the '>' in the cmd array ; (-1) - not appear in cmd array ; other value - the location of '>' in 'cmd array'
static int   file;			    //represent the file path in case the user want to save the content of operation to file.
static pid_t child_pid;			//represent a child process id

void getInitialPrompt(char hostname[512],char *name);
void PrintTokens(char** tokens);
char** ReadTokens(FILE* stream);
void FreeTokens(char** tokens);
void greaterSignCheck(char* wordToCheck, int wordLocation);
int runCommand(char** cmd);
void signalHandler(int sig_num);
void printErrorsAndAlerts(int errorNumber, char **cmd);


int main()
{
	char **cmd;										//represent the command line
	char *name = getlogin();						//represent the login name
	char hostname[512];								//represent the hostname
	int returnCode = 0;								//represent the last function return code

	if(signal(SIGINT, signalHandler) == SIG_ERR)	//define SIGINT to active signalHandler
		printf("Error while define SIGINT!");		//print error

	getInitialPrompt(hostname, name);				//get the initial prompt

	while(1)
	{
		greaterSignLoc = NO_GREATRE_SIGN_LOCATION;			//reset the greater sign location
		child_pid = 0;										//reset child pid

		printf("%d %s@%s$ ",returnCode, name, hostname); 	//print prompt: [return code][user]@[login]$:
		cmd = ReadTokens(stdin);							//get user command input

		if(cmd != NULL)										//check if the word array is not null
		{
			if((strcmp(cmd[0],"exit") == 0) && (cmd[1] == NULL))	//check if the first word is 'exit', if does, break the loop.
				break;
			else
			{
				returnCode = runCommand(cmd);

				if((greaterSignLoc != NO_GREATRE_SIGN_LOCATION) && (returnCode != OPEN_FILE_ERROR))
					if(close(file) == -1)					//close the file
						returnCode = CLOSE_FILE_ERROR;		//if get error while closing file, return an error

				printErrorsAndAlerts(returnCode, cmd);
			}
		}
		FreeTokens(cmd);	//free the command array
	}
	FreeTokens(cmd);		//free the command array
	return 0;
}

//get the initial prompt
void getInitialPrompt(char hostname[512],char *name)
{
	if (!name) 												//check if getlogin() succeed, if not, print an error
		fprintf(stderr, "getlogin() didn't succeed\n");		//Print error

	 if ( gethostname(hostname, 512) == -1 )				//try to get the user hostname, if it fails,
		fprintf(stderr, "gethostname() didn't succeed\n");	//Print error
}

//print the tokens
void PrintTokens(char** tokens)
{
	int i;
	for(i = 0 ; tokens[i] != NULL ; i++)
		printf("%s\n",tokens[i]);
}

//read tokens from the stream and return list of them
char** ReadTokens(FILE* stream)
{

	char streamLine[512];			//represent a stream line
	char streamLineCopy[512];		//represent the copy of a stream line
	char* tempWord;					//represent a temporary word that will be add to the words to return
	char** wordsToReturn;			//represent the words to return
	int wordCounter, i;

	fgets(streamLine,512,stream);  //copy the string from 'stream' to 'streamLine'

	for(i = 0 ; (i < 512) && (streamLine[i] != '\n') ; i++); //'i' gets the location of the first '\n' - the end of the wanted string

	streamLine[i] = '\0';
	strcpy(streamLineCopy,streamLine);	//backup the stream line

	tempWord = strtok(streamLineCopy, " ");

	wordCounter = 0;
	while(tempWord!=NULL)	//count the words
	{
		tempWord = strtok(NULL," ");
		wordCounter++;
	}

	wordsToReturn = (char**)malloc( (wordCounter + 1) * sizeof( char* ) );	//allocate place for the words
	if(wordsToReturn == NULL)	//check if the allocation was successful, return NULL array
		return wordsToReturn;


	tempWord = strtok(streamLine, " ");
	i = 0;
	while(tempWord != NULL)
	{
		wordsToReturn[i] = (char*)malloc( (strlen(tempWord) + 1) * sizeof(char) );
		if(wordsToReturn[i] == NULL)	//check if the allocation was successful, free the array and return NULL array
		{
			FreeTokens(wordsToReturn);	//free the allocated array
			return NULL;
		}

		greaterSignCheck(tempWord, i);	//check if there are any '>';

		strcpy(wordsToReturn[i],tempWord);	//add the current word to the word array
		tempWord = strtok(NULL," ");	//get the next word

		i++;
	}

	if(greaterSignLoc != -1)
	{
		free(wordsToReturn[greaterSignLoc]);
		wordsToReturn[greaterSignLoc] = '\0';
	}
	else
		wordsToReturn[i] = NULL;	//set the last word to null

	for(i = 0 ; wordsToReturn[i] != NULL ; i++);
	if (i >= 1)
		return wordsToReturn;
	else
		return NULL;
}

//free the tokens
void FreeTokens(char** tokens)
{
	if (tokens == NULL)
		return;

	int i = 0;
	while(tokens[i] != NULL)
	{
		free(tokens[i]);
		tokens[i] = NULL;
		i++;
	}
	free (tokens);
}

//check if there are any '>', if does, change the static verb 'greaterSignLoc' to the '>' location
void greaterSignCheck(char* wordToCheck, int wordLocation)
{
	int wordLength = strlen(wordToCheck);

	if(wordLength == 1 && wordToCheck[0] == '>' && greaterSignLoc == NO_GREATRE_SIGN_LOCATION)	//check if the word contains only '>', and if there ware no '>' before
		greaterSignLoc = wordLocation;
}

//Run the exec command according to the user's input
int runCommand(char** cmd)
{
	int childStatus;

	if(greaterSignLoc != NO_GREATRE_SIGN_LOCATION)	// if the '>' sign appears in the 'cmd' array, opening the file to write
	{
		file = open(cmd[greaterSignLoc+1], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);	//opening the file
		if (file < 0)		//check if it the open file failed - if 'open' return value smaller then '0' its means it failed;
			return OPEN_FILE_ERROR;
	}


	child_pid = fork();	//create process - forking

	if(child_pid < 0)		//========= Fork failed =========
	{
		return FORK_ERROR;
	}

	else if(child_pid == 0)	// ========= Child process =========
	{
		setpgrp();	//Separate the child to its one group

		if(greaterSignLoc != NO_GREATRE_SIGN_LOCATION)
		{
			//redirect standard output to the file
			if(dup2(file, fileno(stdout)) < 0)	//writ exec to file and check if dup2 succeed
				exit(DUP2_ERROR);	//dup2 error
		}

		if(execvp(cmd[0], cmd) == -1)	//run the exec and check if run failed
			exit(EXECVP_ERROR);	//kill the child process with the CMD_ERROR value

	}

	else if(child_pid > 0)	// ========= Father process =========
	{
		if(wait(&childStatus) == -1) //Wait for child process.
			return WAIT_ERROR;

		else	// Check status.
		{
		   if (WIFSIGNALED(childStatus) != 0)	//Child process ended because of signal
				return SIGNAL_KILL_CHILD;

		   else if (WIFEXITED(childStatus) != 0) //if child exit normally
			   return WEXITSTATUS(childStatus);	//return the exit status of the child process

		   else
			  printf("Child process did not end normally \n");

		}
		return childStatus;
	}

	return childStatus;
}


//Handling a signal
void signalHandler(int sig_num)
{
	if(child_pid > 0)
		kill(child_pid, SIGKILL);	//send kill signal to the child
}

//prints the errors and the alerts
void printErrorsAndAlerts(int errorNumber, char **cmd)
{
	switch(errorNumber)
	{
		case EXECVP_ERROR   	:  fprintf(stderr, "%s: command not found		 \n", cmd[0]);	break;
		case FORK_ERROR     	:  fprintf(stderr, "Fork failed! 				 \n");			break;
		case OPEN_FILE_ERROR	:  fprintf(stderr, "Can't open or create file! 	 \n");			break;
		case DUP2_ERROR     	:  fprintf(stderr, "Dup2() fail! 				 \n");			break;
		case CLOSE_FILE_ERROR  	:  fprintf(stderr, "Error while closing file!    \n");			break;
		case SIGNAL_KILL_CHILD  :  fprintf(stderr, "Child killed by  a signal!	 \n");			break;
		case WAIT_ERROR  		:  fprintf(stderr, "Error while father wait! 	 \n");			break;
	}
}
