#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <pthread.h>

int searchDictionary(FILE *dictionary, char *word);
char *stolower(char *word);
char **commandParser(char *inputstring, int *numofwords);
char *getInput();
int isValidPort(char *testPort);
void addClient(int clientFd);
void consumeClient();
void serviceClient(int clientFd);

#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 30011
FILE *dictionary;

//fdqueue, maxcount, producedcount, and consumedcount are only to be accessed when theyre protected by the mutex countlock
pthread_mutex_t countlock;
pthread_cond_t bufferfull;
pthread_cond_t bufferempty;
int producedcount = 0;
int consumedcount = 0;
int maxcount = 50;
int fdqueue[50];

int main(int argc, char *argv[])
{

	pthread_cond_init(&bufferfull, NULL);
	pthread_cond_init(&bufferempty, NULL);
	pthread_mutex_init(&countlock, NULL);

	//Getting the arguements for potential replacement dictionaries and ports
	int portno = DEFAULT_PORT;


	dictionary = fopen(DEFAULT_DICTIONARY, "r");
	for(int i = 1; i < argc; i ++)
	{
		if(fopen(argv[i], "r") != NULL)
			dictionary = fopen(argv[i], "r");
		else if(isValidPort(argv[i]))
		{
			portno = atoi(argv[i]);
		}

	}

	//Creating 5 worker threads and sending them to the appropriate starting function
	pthread_t workerthread[5];

	for(int i = 0; i < 5; i ++)
		pthread_create(&workerthread[i], NULL, (void *)consumeClient, NULL);
		
	

	//Setting up the server so that it can recieve connections from client processes
	struct sockaddr_in serveraddress;
	int serversocket = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serveraddress, 0, sizeof(struct sockaddr_in));
	serveraddress.sin_family = AF_INET;
	serveraddress.sin_port = htons(portno);
	serveraddress.sin_addr.s_addr = INADDR_ANY;

	bind(serversocket, (struct sockaddr *)&serveraddress, sizeof(struct sockaddr));

	listen(serversocket, 20);

	while(1)	//loops forever, 0 is false, true is anything else
	{
		//getting the file descriptors for the client connections
		socklen_t clientaddrsize;
		struct sockaddr clientaddr;
		clientaddrsize = sizeof(struct sockaddr);
		int clientfd = accept(serversocket, &clientaddr, &clientaddrsize);
		
		//Adds the client to the queue, then accepts another connection after it has done so successfully
		//addClient(clientfd);

		pthread_mutex_lock(&countlock);
		//critical section. Maxcount, produced count, consumed count, and the buffer may only be accessed while in a critical section.
		if(producedcount - consumedcount == maxcount)
		{
			pthread_cond_wait(&bufferfull, &countlock);
		}

		fdqueue[producedcount % maxcount] == clientfd;	
		producedcount++;


		pthread_cond_signal(&bufferempty);
		pthread_mutex_unlock(&countlock);

	}

	return 0;
}


//Checks to see if the arguement entered is the number of a valid port
int isValidPort(char *testport)
{
	int checkrange = atoi(testport);
	if(checkrange <= 0 || checkrange > 65335)
		return 0;
	else
		return 1;
}

//Function that adds the clientFd to the list of clients that are waiting to be processed by a thread
//If there is no room left in the buffer (if producedcount - consumedcount >= 50) for another client file descriptor, the main server thread waits until another thread has consumed from the buffer
void addClient(int clientFd)
{
	pthread_mutex_lock(&countlock);
	//critical section. Maxcount, produced count, consumed count, and the buffer may only be accessed while in a critical section.
	if(producedcount - consumedcount == maxcount)
		pthread_cond_wait(&bufferfull, &countlock);


	fdqueue[producedcount % maxcount] == clientFd;
	producedcount++;


	pthread_cond_signal(&bufferempty);
	pthread_mutex_unlock(&countlock);

	return;

}

//starting function for the thread. It gets a socket from the queue when one is available,releases the lock, then calls the serviceClient function to return the necessary output to the client. COnsumedcount is incremented, and the loop continues until it can get another socket from the quee
void consumeClient()
{
	int obtainedclient;
	while(1)
	{
		pthread_mutex_lock(&countlock);
		//critical section. Maxcount, produced count, consumed count, and the buffer may only be accessed while in a critical section.

		//Obtaining the client from the queue
		if(producedcount - consumedcount == 0)
			pthread_cond_wait(&bufferempty, &countlock);

		obtainedclient = fdqueue[consumedcount % maxcount];
		consumedcount++;


		//Releasing the lock

		pthread_cond_signal(&bufferfull);
		pthread_mutex_unlock(&countlock);

		serviceClient(obtainedclient);
	}
	return;
}

void serviceClient(int clientFd)
{
		int numofwords = 0;
		char *inputstring;
		inputstring = (char *)malloc(sizeof(char) * 5001);
		recv(clientFd, inputstring, 5000, 0);
		char **inputsentence = commandParser(inputstring, &numofwords);

		for(int i = 0; i < numofwords; i++)
		{
			int mistake = searchDictionary(dictionary, inputsentence[i]);
			if(mistake == 0)
			{
				char *wordresponse = strcat(inputsentence[i], " OK");
				//printf("%s OK", inputsentence[i]);
				send(clientFd, wordresponse, sizeof(char) * (strlen(wordresponse) + 1), 0);
			}
			else
			{
				char *wordresponse = strcat(inputsentence[i], " MISTAKE");
				//printf("%s MISTAKE", inputsentence[i]);
					send(clientFd, wordresponse, sizeof(char) * (strlen(wordresponse) + 1), 0);
			}
			rewind(dictionary);
		}

	return;
}


//Takes a string, and converts it into an array of words. The user can find out how many elements are in the array with the numofwords parameter.
char **commandParser(char *inputstring, int *numofwords)
{
	char **parsed = (char**)malloc(sizeof(char*));

	int stringlength = strlen(inputstring);
	int wordnum = 0;	//number of words in the parser
	int wordsize = 0;

	for(int i = 0; i < stringlength; i ++)		//loops through every character in the string
	{
		if(inputstring[i] != ' ' && inputstring[i] != '\t')	//if the character is not a white space
		{
			if(i == 0)	//if the first character is the start of a word, make room for the word
			{
				wordnum++;	//increases the number of known words in the command
				parsed = realloc(parsed, sizeof(char*) * wordnum);	//provides more space for the new word
				wordsize = 0;	//goes to the start of the word
				parsed[wordnum - 1] = (char*)malloc(sizeof(char));	//initializes the memory for the new word
			}
			else if(inputstring[i - 1] == ' ' || inputstring[i - 1] == '\t')
			{		//if the character is the start of a word	
				wordnum++;
				parsed = realloc(parsed, sizeof(char*) * wordnum);	//provides more space for the new word
				wordsize = 0;
				parsed[wordnum - 1] = (char*)malloc(sizeof(char));
			}


			wordsize++;	//makes room for the next letter of the word
			parsed[wordnum -1] = realloc(parsed[wordnum-1], sizeof(char) * wordsize);
			parsed[wordnum - 1][wordsize-1] = inputstring[i];	//stores the letter of the word

			if(i == stringlength - 1 )//if its the end of the word (if theres a whitespace next, or if its the last characer in string input), add a null terminator
			{
				parsed[wordnum -1] = realloc(parsed[wordnum-1], sizeof(char) * wordsize + 1);
				parsed[wordnum - 1][wordsize] = '\0';	//adds a null terminator to the end of the word

			}
			else if(inputstring[i + 1] == ' ' || inputstring[i + 1] == '\t')//if its the end of the word (if theres a whitespace next, or if its the last characer in string input), add a null terminator, corrupted characters appear at the end of the string sometimes otherwise
			{
				parsed[wordnum -1] = realloc(parsed[wordnum-1], sizeof(char) * wordsize + 1);
				parsed[wordnum - 1][wordsize] = '\0';	//adds a null terminator to the end of the word

			}
		}

	}
	parsed = realloc(parsed, sizeof(char*) * wordnum + 1);	//provides more space for the new word
	parsed[wordnum] = NULL;	//ends the array
	*numofwords = wordnum;

	return parsed;

}

int searchDictionary(FILE *dictionary, char *word)
{
	char* lowerword = stolower(word);
	size_t largestword = sizeof(char) * 400;		//the longest word in literature is like 400 letters long, in sanskrit
	char *dictionaryword;
	dictionaryword = (char *)malloc((sizeof(char) * largestword) + 1);
	while((getline(&dictionaryword, &largestword, dictionary)) != -1)
	{
		dictionaryword = strtok(dictionaryword, "\t\r\n");
		char *lowerdict = stolower(dictionaryword);
		if(strcmp(lowerdict, lowerword) == 0)
			return 0;	
	}



	return 1;
}

char* getInput()
{
		char *input = (char *)malloc(sizeof(char) * 5000);
		do{
			fgets(input, sizeof(char) * 5000, stdin);	
			//printf("%s", input);	
			input[strlen(input) - 1] = '\0';
		}
		while(strlen(input) < 1);	//if the user presses enter and nothing else
		return input;
}


//Converts a string to nothing but lower case letters
char *stolower(char *word)
{
	for(int i = 0; i < strlen(word); i++)
		word[i] = tolower(word[i]);
	return word;
}
