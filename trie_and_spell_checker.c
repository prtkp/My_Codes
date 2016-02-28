/*------------------------------------------------------------------
Program description	:   Implement Trie and Spell Checker
Acknowledgements	:   This is source file and only contains functions which are needed to implement trie data structure.
--------------------------------------------------------------------*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/* to make the array of alphabates */
tries* makeArray (){
	tries *c;
	int i;
	c = (tries *) malloc (sizeof (tries));
	if (c == NULL){				//warning for memory overflow
		printf ("No space\n");
		exit (0);
		}
	for (i = 0; i < 96; i++)	//initialize array with NULL pointer
		c->arr[i] = NULL;
	return c;
}

/* function to insert the words into dictionary */
void insertion (char *word, tries *dict){
tries *temp, *ptr;
int i, choice, len, t;
	temp = dict;
	for (i = 0; i < strlen(word) ; i++){
		if(word[i] >= 97 && word[i] <= 122)
			t = word[i] - 32;
		else if(word[i] >=123 && word[i] <= 126)
			t = word[i] - 26;
		else
			t = word[i];//tolower(word[i]) - 96;	/* loc of letter */
		
		if (temp->arr[t] == NULL){		/* if nothing is there create new array */
			ptr = makeArray ();
			temp->arr[t] = ptr;
			temp = ptr;
		}//end of if
		else
			temp = temp->arr[t];	/* else go to that location and check for next letter */
	    		}//end of for
		//end of else
		temp->arr[0] = (tries *)word;
}
/* function to search a word from dictionary */
int searchWord (char *word, tries * dict){
	tries *temp = dict;
	int t, i;
	for (i = 0; i < strlen (word); i++){
		if(word[i] >= 97 && word[i] <= 122)	/* convert ASCII value of each character */
			t = word[i] - 32;		/* to integer */
		else if(word[i] >=123 && word[i] <= 126)
			t = word[i] - 26;
		else
			t = word[i];
		if (temp->arr[t] == NULL)	//if it is NULL that means word is not present
			return 0;	//0 for unsuccessful search
		else
			temp = temp->arr[t];
		}
	return  1;	//1 for successful search
}
/* suggestions for group A ie. words obtained from Trie by delete any one letter from word */
void suggestionForGroupA(char *word, tries *dict){		//delete a letter and search
	int status = 0, counter1, counter2, t = 0, len = strlen(word);
	char w[len];
	for(counter1 = 0 ; counter1 < len ; counter1++){	//for tracking position of word
		for(counter2 = 0 ; counter2 < len ; counter2++){	//for copying the word
			if(counter2 != counter1)
				w[t++] = word[counter2];
			}//end of for
			t = 0;
			status = searchWord(w, dict);	//search
			if(status == 1)			//search successful
				printf("%s\n",w);
			status = 0;
		}
}

/* suggestions for group B ie. words obtained from Trie by adding one letter at any position of word*/
void suggestionForGroupB(char *word, tries *dict){		//add a letter and search
	int status = 0, counter1, counter2, counter3, t = 0, len = strlen(word);
	char w[len + 2];
	for(counter1 = 0 ; counter1 < len+1 ; counter1++){	//for tracking position of word
		for(counter2 = 33 ; counter2 < 127 ; counter2++){//for appending a letter into word
			for(counter3 = 0 ; counter3 < len ; counter3++){	//for copying word
				if(counter3 == counter1)
					w[t++] = counter2;
				w[t++] = word[counter3];					
				}//end of for 3
				t = 0;
				if(counter1 == len)
					w[len] = counter2;
				w[len+1] = '\0';
				status = searchWord(w, dict);	//search
				if(status == 1)			//search successful
					printf("%s\n",w);
				status = 0;
			}//end of for 2
		}//end of for 1
}

/* suggestions for group C ie. words obtained from Trie by changing any single letter of word*/
void suggestionForGroupC(char *word, tries *dict){		//change a letter in the word and search
	int status = 0, counter1, counter2, counter3, t = 0, len = strlen(word);
	char w[len + 1];
	for(counter1 = 0 ; counter1 < len ; counter1++){	//for tracking position of word
		for(counter2 = 33 ; counter2 < 127 ; counter2++){	//for changing letter in word
			for(counter3 = 0 ; counter3 < len ; counter3++){	//for copying the word
				if(counter3 == counter1)
					w[t++] = counter2;
				else
					w[t++] = word[counter3];
				}//end of for 3
				w[len] = '\0';
				t = 0;
				status = searchWord(w, dict);	//search
				if(status == 1)			//search successful
					printf("%s\n",w);
				status = 0;
			}//end of for 2
		}//end of for 1
}

/* suggestions for group D ie. words obtained from Trie by swapping any two letters of word*/
void suggestionForGroupD(char *word, tries *dict){		//swap any 2 letters in word and search
	int status = 0, counter1, counter2, len = strlen(word);
	char c;
	for(counter1 = 0 ; counter1 < len ; counter1++){
		for(counter2 = counter1+1 ; counter2 < len ; counter2++){
			c = word[counter1];			//swap two letters
			word[counter1] = word[counter2];
			word[counter2] = c;
				status = searchWord(word, dict);	//search
				if(status == 1)			//search successful
					printf("%s\n",word);
				status = 0;
			c = word[counter1];			//correct the word and again swap with
			word[counter1] = word[counter2];	//different letter
			word[counter2] = c;
			}//end of for 2
		}//end of for 1
}

/* give the suggestion of a word */
void suggestion(char *word, tries *dict){
	printf("Suggested words are\n");
	suggestionForGroupA(word, dict);		//find suggestion of group A
	suggestionForGroupB(word, dict);		//find suggestion of group B
	suggestionForGroupC(word, dict);		//find suggestion of group C
	suggestionForGroupD(word, dict);		//find suggestion of group D
}


/* function to delete the arrays which are created while creating dictionary */
void deleteArrays(tries *dict){
if(dict != NULL){
	int i;
	for(i = 1 ; i < 100 ; i++){
		if(dict->arr[i] != NULL){
			deleteArrays(dict->arr[i]);
			free(dict->arr[i]);
			dict->arr[i] = NULL;}		
		}
	}
}

/* fun to free the memory used ie to delete the dictionary. This function uses above deleteArrays() to delete the memory */
void freeMemory(tries *dict){
	tries *temp = dict;
	int t, i, k;
	char c, *word;
	word = (char*)malloc(sizeof(char)*100);
	FILE *f = fopen("/usr/share/dict/words","r");		//open the file
	c = fscanf(f,"%s",word);	/* take the word one by one from the file */
	while(c != EOF){
		temp = dict;
		for (i = 0 ; word[i] != '\0' ; i++){
			if(word[i] >= 97 && word[i] <= 122)
				t = word[i] - 32;
			else if(word[i] >=123 && word[i] <= 126)
				t = word[i] - 26;
			else
				t = word[i];			/* loc of letter */
			if (temp->arr[t] != NULL)
				temp = temp->arr[t];	
			}
		free(temp->arr[0]);
		temp->arr[0] = NULL;
		for (k = 0 ; k < 100 ; k++)
			word[k] = '\0';
		c = fscanf(f,"%s",word);
		}
		free(word);
		fclose(f);				//close the file
		deleteArrays(dict);
}
