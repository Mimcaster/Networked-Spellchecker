//client selects a random paragraph from a book, connects to the server, sends out the word that needs to be checked from the paragraph, and prints out what the server returns

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void sendMessage();
char *randomPassage();
char **commandParser(char *inputstring, int *numofwords);

int main()
{
	pthread_t threads[20];

	for(int i = 0; i < 20; i ++)
	{
		sleep(1);
		pthread_create(&threads[i], NULL, (void *)sendMessage, NULL);	
	}
	for(int j = 0; j < 20; j ++)
		pthread_join(threads[j], NULL);

	return 0;
}

void sendMessage()
{
	int clientsocket;
	clientsocket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serveraddress;

	memset(&serveraddress, 0, sizeof(struct sockaddr_in));
	serveraddress.sin_addr.s_addr=inet_addr("129.32.95.123");	//loopback address for testing
	serveraddress.sin_port = htons(30011);
	serveraddress.sin_family = AF_INET;

	if(connect(clientsocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) == -1)
	{
		printf("connection error");
	}

	char *words;
	words = randomPassage();

	send(clientsocket, words, sizeof(char) * (strlen(words) + 1), 0);

	char *response;

	response = (char *)malloc(sizeof(char) * 500);	//longest word is around 400 letters long
	int numofwords;
	commandParser(words, &numofwords);

	for(int i = 0; i < numofwords; i ++)
	{
		recv(clientsocket, response, 500, 0);
		printf("%s\n", response);
	}

	printf("hheuheuheuhe");

	return;
}

//Takes a string, and converts it into an array of words. The user can find out how many elements are in the array with the numofwords parameter. Improperly placed black spaces are ignored.
char **commandParser(char *inputstring, int *numofwords)
{
	char **parsed = (char**)malloc(sizeof(char*));

	int stringlength = strlen(inputstring);
	int wordnum = 0;	//number of words in the parser
	int wordsize = 0;

	//Loops through every character in the string. If the string starts off with white spaces, then those white spaces are ignored. It adds each character one at a time into an array of characters until it finds a white space. Then it searches for the next non whitespace character and puts that one in a new array of characters, creating a new word. This keeps on going until the whole string has beened parsed into an array of strings.
	
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


char *randomPassage()
{
	char *passage;
	srand(time(NULL));	//seeds the rng
	int randomnum = rand() % 5;

	if(randomnum == 0)
	{
		passage = (char *)malloc(sizeof(char) * strlen("ll rejoice to heer that no disaster has accompanied the commencement of an enterprise which you have regrded with such evil frtion, by discovering a passage near the pole to those countries, to reach which at present so many months ar") + 1);
		passage = "ll rejoice to heer that no disaster has accompanied the commencement of an enterprise which you have regrded with such evil frtion, by discovering a passage near the pole to those countries, to reach which at present so many months ar";
	}
	else if(randomnum == 1)
	{
		passage = (char *)malloc(sizeof(char) * strlen(" some encouraging voice would answer in the affirmative! My courage and my resolution is firm; bun through the seas which surround the pole. You may remember that a history of all the voyages made for purposes of discovery composed the whole of our good Uncle Thomas' library. My education was neglected, yet I w") + 1);
		passage = " some encouraging voice would answer in the affirmative! My courage and my resolution is firm; bun through the seas which surround the pole. You may remember that a history of all the voyages made for purposes of discovery composed the whole of our good Uncle Thomas' library. My education was neglected, yet I w";
	}	
	else if(randomnum == 2)
	{
		passage = (char *)malloc(sizeof(char) * strlen("ll rejoich such evil frosing all these conjectures to be false, you cannot contest the inestimable benefit which I shall confer on all mankind, to the last generation, by discovering a passage near the pole to those countries, to r") + 1);
		passage = "ll rejoich such evil frosing all these conjectures to be false, you cannot contest the inestimable benefit which I shall confer on all mankind, to the last generation, by discovering a passage near the pole to those countries, to r";
	}	
	else if(randomnum == 3)
	{
		passage = (char *)malloc(sizeof(char) * strlen("onjectures to be false, you cnot contassst the inestimable benefit which I shall confer on all manthe latter town in a fortnight or three weeks; and my intention is to hire a ship there, which can easily be done by paying the insurance for the owner, and to engage as many sailors as I think necessary am") + 1);
		passage = "onjectures to be false, you cnot contassst the inestimable benefit which I shall confer on all manthe latter town in a fortnight or three weeks; and my intention is to hire a ship there, which can easily be done by paying the insurance for the owner, and to engage as many sailors as I think necessary am";
	}
	else if(randomnum == 4)
	{
		passage = (char *)malloc(sizeof(char) * strlen("of beeuty and delargaret, the sun is forever visible, its broad disk just skirting the hod frost are banished; and, sailing over a calm sea, we may be wafted to a land su several expeditions to the North Sea; I voluntarily endured cold, famine, thirst, and want of sleep; I often worked harder than the common sailors during the day and devoted my nights to the study of mathemat") + 1);
		passage = "of beeuty and delargaret, the sun is forever visible, its broad disk just skirting the hod frost are banished; and, sailing over a calm sea, we may be wafted to a land su several expeditions to the North Sea; I voluntarily endured cold, famine, thirst, and want of sleep; I often worked harder than the common sailors during the day and devoted my nights to the study of mathemat";
	}

	return passage;
}


/* Text taken from Frankenstein by Mary Shelly
 You will rejoice to hear that no disaster has accompanied the commencement of an enterprise which you have regarded with such evil forebodings. I arrived here yesterday, and my first task is to assure my dear sister of my welfare and increasing confidence in the success of my undertaking.

I am already far north of London, and as I walk in the streets of Petersburgh, I feel a cold northern breeze play upon my cheeks, which braces my nerves and fills me with delight. Do you understand this feeling? This breeze, which has travelled from the regions towards which I am advancing, gives me a foretaste of those icy climes. Inspirited by this wind of promise, my daydreams become more fervent and vivid. I try in vain to be persuaded that the pole is the seat of frost and desolation; it ever presents itself to my imagination as the region of beauty and delight. There, Margaret, the sun is forever visible, its broad disk just skirting the horizon and diffusing a perpetual splendour. There—for with your leave, my sister, I will put some trust in preceding navigators—there snow and frost are banished; and, sailing over a calm sea, we may be wafted to a land surpassing in wonders and in beauty every region hitherto discovered on the habitable globe. Its productions and features may be without example, as the phenomena of the heavenly bodies undoubtedly are in those undiscovered solitudes. What may not be expected in a country of eternal light? I may there discover the wondrous power which attracts the needle and may regulate a thousand celestial observations that require only this voyage to render their seeming eccentricities consistent forever. I shall satiate my ardent curiosity with the sight of a part of the world never before visited, and may tread a land never before imprinted by the foot of man. These are my enticements, and they are sufficient to conquer all fear of danger or death and to induce me to commence this laborious voyage with the joy a child feels when he embarks in a little boat, with his holiday mates, on an expedition of discovery up his native river. But supposing all these conjectures to be false, you cannot contest the inestimable benefit which I shall confer on all mankind, to the last generation, by discovering a passage near the pole to those countries, to reach which at present so many months are requisite; or by ascertaining the secret of the magnet, which, if at all possible, can only be effected by an undertaking such as mine.

These reflections have dispelled the agitation with which I began my letter, and I feel my heart glow with an enthusiasm which elevates me to heaven, for nothing contributes so much to tranquillize the mind as a steady purpose—a point on which the soul may fix its intellectual eye. This expedition has been the favourite dream of my early years. I have read with ardour the accounts of the various voyages which have been made in the prospect of arriving at the North Pacific Ocean through the seas which surround the pole. You may remember that a history of all the voyages made for purposes of discovery composed the whole of our good Uncle Thomas' library. My education was neglected, yet I was passionately fond of reading. These volumes were my study day and night, and my familiarity with them increased that regret which I had felt, as a child, on learning that my father's dying injunction had forbidden my uncle to allow me to embark in a seafaring life.

These visions faded when I perused, for the first time, those poets whose effusions entranced my soul and lifted it to heaven. I also became a poet and for one year lived in a paradise of my own creation; I imagined that I also might obtain a niche in the temple where the names of Homer and Shakespeare are consecrated. You are well acquainted with my failure and how heavily I bore the disappointment. But just at that time I inherited the fortune of my cousin, and my thoughts were turned into the channel of their earlier bent.

Six years have passed since I resolved on my present undertaking. I can, even now, remember the hour from which I dedicated myself to this great enterprise. I commenced by inuring my body to hardship. I accompanied the whale-fishers on several expeditions to the North Sea; I voluntarily endured cold, famine, thirst, and want of sleep; I often worked harder than the common sailors during the day and devoted my nights to the study of mathematics, the theory of medicine, and those branches of physical science from which a naval adventurer might derive the greatest practical advantage. Twice I actually hired myself as an under-mate in a Greenland whaler, and acquitted myself to admiration. I must own I felt a little proud when my captain offered me the second dignity in the vessel and entreated me to remain with the greatest earnestness, so valuable did he consider my services. And now, dear Margaret, do I not deserve to accomplish some great purpose? My life might have been passed in ease and luxury, but I preferred glory to every enticement that wealth placed in my path. Oh, that some encouraging voice would answer in the affirmative! My courage and my resolution is firm; but my hopes fluctuate, and my spirits are often depressed. I am about to proceed on a long and difficult voyage, the emergencies of which will demand all my fortitude: I am required not only to raise the spirits of others, but sometimes to sustain my own, when theirs are failing.

This is the most favourable period for travelling in Russia. They fly quickly over the snow in their sledges; the motion is pleasant, and, in my opinion, far more agreeable than that of an English stagecoach. The cold is not excessive, if you are wrapped in furs—a dress which I have already adopted, for there is a great difference between walking the deck and remaining seated motionless for hours, when no exercise prevents the blood from actually freezing in your veins. I have no ambition to lose my life on the post-road between St. Petersburgh and Archangel. I shall depart for the latter town in a fortnight or three weeks; and my intention is to hire a ship there, which can easily be done by paying the insurance for the owner, and to engage as many sailors as I think necessary among those who are accustomed to the whale-fishing. I do not intend to sail until the month of June; and when shall I return? Ah, dear sister, how can I answer this question? If I succeed, many, many months, perhaps years, will pass before you and I may meet. If I fail, you will see me again soon, or never. Farewell, my dear, excellent Margaret. Heaven shower down blessings on you, and save me, that I may again and again testify my gratitude for all your love and kindness. 
*/
