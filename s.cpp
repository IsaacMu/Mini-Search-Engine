#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include "stemming.cpp"

typedef struct listNode *Node;
typedef struct Table *HashTable;

long long stop_count;//the amount of stopwords in the set 
long long total_count;// the total amount of words in the set

/*data struct for saving the positions of the word in the page*/
struct position{
	int place;
	struct position *Next;
};

/*data struct for saving the pages the word appeared in
 and there is a pointer to save the positions where the word appeared at*/
struct page{
	int countNumber;
	char *PAGE;
	struct position *Position;
	struct page *Next;
};
/*data struct for saving all information of the word*/
struct listNode{
	char word[30];
	struct page *Page;
	Node Next;
};

/*
We find a hashing value and its correspongding index file--"f_hashivalue.txt" is saved as elements
*/
struct Table{
	int Tablesize;
	char **table;
};

/*when we build the index, we will use this function
we inset the word into the correspongding index file accroding to the hashing value*/
int InsertWord(char *word, char *FileName, int count,HashTable T);

/*build a list from the index file named "FileName", and return the address of the list*/
listNode* ReadListNode( char* FileName );

/*build a initial hash table and return its address*/
HashTable InitializeTable( int TableSize ); 

/*judge whether the string is a stop word, yes---1, no-----0*/
bool CheckStop(char *p);

/*read the file named "FileName", and build index with all words except for the stopping words
  meanwhile, we static the amounts of words and stop words*/
void ReadFile(char *FileName,HashTable T);

/*a hash function to caculate a word's hash value*/
unsigned int BKDRHash(char *str);

/*this function transfer all capital letters to small letters, and cut the not-letter,such as ';'*/
void cut(char *s);

/*used in function "InitializeTable" to get the file name of every index file*/
void to_string(int Hash_res, char *p);

/*get the names of all the files in the foder"dir_name", anf save them in the array"file_names"*/
void load_fname(char dir_name[], char (*file_names)[25]);

/*store the list into a file*/
void WriteListNode( listNode* head, char* FileName );

/*find the word from the index file
if the word exist,return a pointer to the word
else return null*/
Node FindWord(char *word, HashTable T, Node* headAdr);

/*search for words, which are inputed during the function is running*/
void Search(HashTable T);

/*print the position of the word in the page*/
void PrintPos(struct page *p);

int main() {
	char file_name[860][25];
	int n;
    HashTable T;
    int judge=0;
    long long start,finish;
    double duration;
	load_fname("shake", file_name);
	T=InitializeTable(1000003); 
	char word[30];
	printf("如果没有建立过索引，输入666.\n");
	printf("如果已经建立过索引，随意输入一个数字.\n");
	scanf("%d",&judge);
	if(judge==666)
	{
		start=clock();
		for(n=0;n<850;n++)
	    {
	    	printf("%s\n", file_name[n]);
			ReadFile(file_name[n],T);
	    	printf("%d\n", n);
		}
		finish=clock();
		duration=(double)(finish-start)/CLOCKS_PER_SEC;
		printf("There are %lld words in the Shakespeare set.\n",total_count);
		printf("There are %lld stopwords in the Shakespeare set.\n",stop_count);
		printf("time : %.0f ms\n",duration*1000);
	}
    Search(T);
}


/*build a initial hash table and return its address*/
HashTable InitializeTable( int TableSize ){
    HashTable H;
    int i;
    H = (HashTable)malloc(sizeof(struct Table)); //Apply for the position of the table 
    if (H == NULL){
    	printf( "Out of space!!!" );
    	return NULL;
	}
    H->Tablesize = TableSize;
    H->table =(char**)malloc(sizeof(char*)*H->Tablesize);
    if( H->table == NULL ){
    	printf( "Out of space!!!" );
    	return NULL;
	}
    for( i = 0; i < H->Tablesize; i++ )  
    {
        H->table[ i ] = (char*)calloc(15, sizeof(char));
        if ( H->table[ i ] == NULL ){
	    	printf( "Out of space!!!" );
	    	return NULL;
	    }
        else to_string(i,H->table[ i ]); //translate the hash value into the filename \
											and store them into the tabe as elements
        
    }
    return H;  
}

/*judge whether the string is a stop word, yes---1, no-----0*/
bool CheckStop(char *p){
	FILE *fp;
	fp = fopen("stop_words_eng.txt", "r");  //open the file which stores the stop words 
	char s[20];  
	fscanf(fp, "%s", s);
	while(!feof(fp)){
        if(strcmp(p,s)==0)   //if p is found in the file
        {
        	fclose(fp);
        	return 1;		//then p is a stop word
		}
		
		fscanf(fp, "%s", s);
     }
     if(strcmp(p,s)==0){
        	fclose(fp);
        	return 1;
		}
     fclose(fp);
     return 0;			//if p has never been found in the file \
     						then it's not a stop word
}

/*read the file named "FileName", and build index with all words except for the stopping words
  meanwhile, we static the amounts of words and stop words*/
void ReadFile(char *FileName,HashTable T){
	int count=0;//record the place
	FILE *fp;
	fp = fopen(FileName, "r");
	char s[40];
	fscanf(fp, "%s", s);//read a word from the file"FileName"
	count++;
	while(!feof(fp)){
        cut(s);//transfer all capital letters to small letters, and cut the not-letter,such as ';'
        total_count ++;//count the total words of the set
        if(strlen(s) > 0){//if s is a word
        	s[stem(s,0,strlen(s)-1)+1]=0;//stem the word
			if(CheckStop(s)==0){//if s is not a stop word
				InsertWord(s,FileName,count,T);//insert this word to table
		    }
		    else{//count all stopwords of the set
		    	stop_count ++;
			}
		}
		//read next word
		fscanf(fp, "%s", s);
		count++;//move the place to next
     }
    s[stem(s,0,strlen(s)-1)+1]=0;//stem the word
    if(CheckStop(s)==0){//if s is not a stop word
		InsertWord(s,FileName,count,T);//insert this word to table
    }
    else{//count all stopwords of the set
    	stop_count ++;
	}
	total_count++;
	fclose(fp);
}
/*a hash function to caculate a word's hash value*/
unsigned int BKDRHash(char *str){
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
 
    while (*str)
    {
        hash = hash * seed + (*str++);
    }
 
    return (hash & 0x7FFFFFFF)%1000003;
}

/*used in function "InitializeTable" to get the file name of every index file*/
/*file name: Index\\f_hashvalue.txt*/
void to_string(int Hash_res, char *p)
{
	int pos=14;
	int j, k;
 	char tmp[15] = {0};
	int i;
	strcat(p,"Index\\f");
	do{
 		tmp[pos--] = (Hash_res%10)+'0';
 		Hash_res /= 10;
	}while(Hash_res>0);
	
	for(j = 7,pos++; tmp[pos] != '\0'; j++,pos++ ){
	 	p[j] = tmp[pos];
	}
	strcat(p, ".txt");	
}

/*get the names of all the files in the foder"dir_name", anf save them in the array"file_names"*/
void load_fname(char dir_name[], char (*file_names)[25])
{
	struct dirent *entry;
	int i=0;
	DIR *p;
	p =opendir(dir_name);     //open the foder
	while((entry = readdir(p))!=NULL){
		if(strlen(entry-> d_name)>4){
			strcpy(file_names[i],"shake\\"); //add the route to the file name
			strcat(file_names[i],entry-> d_name);
			i++;
		}
	}
	closedir(p);   //close the foder
}

/*this function transfer all capital letters to small letters, and cut the not-letter,such as ';'*/
void cut(char *s)
{
	int i;
	for(i = 0; i < strlen(s); ){
		if(s[i]<'A'||(s[i]>'Z'&&s[i]<'a')||s[i]>'z')//if s[i] is not a letter 
			strcpy(s+i,s+i+1);                      //we just cut it
		else{
			if(s[i]>='A'&&s[i]<='Z')                 //transfer all capital letters to small letters
				s[i] = s[i] -'A' +'a';
			i++;
		}
			
	}
}

/*build a list from the index file named "FileName", and return the address of the list*/
listNode* ReadListNode( char* FileName )
{
    FILE* fp;
    listNode *result, *wordP;
    page* pageP;
    position* placeP;
    char bound[30], FileNameStr1[30], FileNameStr2[30];
    int cou, i;
    
    result = (listNode*)malloc(sizeof(listNode));//the head of the list
    strcpy(result->word, "");        //set the word of the head empty
    result->Page = NULL;
    wordP = result;
    
    if( ( fp=fopen(FileName, "r") ) == NULL ) //open the target file
    {
        result->Next = NULL;
		return result;
    }

    fscanf(fp, "%s", bound);  //write a start flag into the file
    if(strcmp(bound,"#START") == 0 )//have word, can be read
    {
        do{							   //reade word list
            cou = 0;
            wordP->Next = (listNode*)malloc(sizeof(listNode));
            wordP = wordP->Next;
            wordP->Next = NULL;
            fscanf(fp, "%s", (wordP->word));
            fscanf(fp, "%s", bound);
            if(strcmp(bound,"#NEXT") == 0 )//this word has been finished, begin the next
            {
                cou = 1;
                wordP->Page = NULL;
            }
            else if(strcmp(bound,"#END") == 0 )//all the words have been read
            {
                cou = 0;
                wordP->Page = NULL;
            }
            else if(strcmp(bound,"#BEGIN") == 0 )//start read page
            {
                wordP->Page = pageP = (page*)malloc( sizeof(page) );
                pageP->PAGE = NULL;
                pageP->countNumber = NULL;            
                do{
                    pageP->Next = (page*)malloc( sizeof(page) );
					pageP = pageP->Next;
					pageP->Next = NULL;
                    fscanf(fp, "%s%s", FileNameStr1, FileNameStr2);
            		strcat(FileNameStr1, " ");
            		strcat(FileNameStr1, FileNameStr2);   //read the name of the page
            		pageP->PAGE = (char*)malloc(sizeof(char)*30); 
            		strcpy(pageP->PAGE, FileNameStr1);   //then store it into the node in the list
                    fscanf(fp, "%d", &(pageP->countNumber) );
                    if(pageP->countNumber != 0)//read place
                    {
                        pageP->Position = placeP = (position*)malloc(sizeof(position));//then store it into the node in the list
                        fscanf(fp, "%d", &(placeP->place));
                        for(i = 0; i < pageP->countNumber - 1; i++)
                        {
                            placeP->Next = (position*)malloc(sizeof(position));
                            placeP = placeP->Next;
                            fscanf(fp, "%d", &(placeP->place));
                        }
                        placeP->Next = NULL;
                    }			//read place end
                    fscanf(fp, "%s", bound);//if bound = ";" there is page next
                }while(strcmp(bound,";") == 0 );
               
                
                if(strcmp(bound,"#END") == 0 )//this is last word, read is over
                {
                    cou = 0;
                }
                else//next word
                {
                    cou = 1;
                }
            }
        }while(cou);
    }
    wordP->Next = NULL;
    
    if(fclose(fp))//close file
    {
        printf("Can not close the word file!\n");
        return NULL;
    }
    return result;
}

/*store the list into a file*/
void WriteListNode( listNode* head, char* FileName )
{
    FILE* fp;
    listNode* wordP, *tmp_l;
    page* pageP, *tmp_pa;
    position* placeP, *tmp_po;
    int i;
    if( (fp=fopen(FileName, "w") )== NULL )
    {
        printf("Write List Node error!\n");
        printf("%d",errno);
        return;
    }

    wordP = head->Next;  //skip over the empty head pointer
    if(wordP != NULL)//if have word
    {
        fprintf(fp, "%s ", "#START");
        while(wordP != NULL)//there is word next
        {
            fprintf(fp, "%s ", wordP->word);
            if( wordP->Page != NULL )   //there is page
            {
                fprintf(fp, "%s ", "#BEGIN");		//make a flag to show that we are beginning a new word.
                pageP = wordP->Page;
                while(pageP != NULL && pageP->Next != NULL)  //traverse the list of the pages
                {
                	tmp_pa = pageP;
                    pageP = pageP->Next;
                    free(tmp_pa);
                    fprintf(fp, "%s %d ", pageP->PAGE, pageP->countNumber); //store the page name and \
															the times the word appears in the page into the doc
                    placeP = pageP->Position;
                    for(i = 0; i < pageP->countNumber; i++)
                    {
                        fprintf(fp, "%d ", placeP->place); //store the places of the word in the page to a doc
                        tmp_po = placeP;
                        placeP = placeP->Next;
                        free(tmp_po);
                    }
                    if(pageP->Next != NULL)
                    {
                        fprintf(fp, "%s ", ";"); //make a flag to show that we are reading next file.
                    }
                }
            }   
            if(wordP->Next != NULL)
            {
                fprintf(fp, "%s ", "#NEXT");  //make a flag to show that we are storing the list of next word
                tmp_l = wordP;
                wordP = wordP->Next;
                free(tmp_l);
            }
            else
            {
                fprintf(fp, "%s ", "#END");
                tmp_l = wordP;
                wordP = wordP->Next;
                free(tmp_l);
            }
        }
    }
    else//there is no word
    {
        fprintf(fp, "%s ", "#END");
    }

    if(fclose(fp))//close file
    {
        printf("Can not close the word file!\n");
        return;
    }   
}

/*find the word from the index file
if the word exist,return a pointer to the word
else return null*/
Node FindWord(char *word, HashTable T, Node* headAdr){
	word[stem(word,0,strlen(word)-1)+1]=0;//stem the word
	unsigned int hash_res=BKDRHash(word);//caculate the hash value of the word
	Node p; 
	p = *headAdr = ReadListNode( T->table[hash_res] ); //build a list corresponding to the hash value
	p=p->Next;
	while(p!=NULL&&strcmp(p->word,word)!=0){//traverse the list, see if the word exist or not
		p=p->Next;
	}
	return p;
}

/*when we build the index, we will use this function
we inset the word into the correspongding index file accroding to the hashing value*/
int InsertWord(char *word, char *FileName, int count,HashTable T){
    Node p, tmp, head;
    int i;
	p=FindWord(word, T, &head); //first we build a list of the words with the same hashing value
					//then let head point to the head of the list
					//then return the position of the word if exist in the list, else return NULL.
	if(p==NULL){		//if the word has never appeared
		p = head;        //build a new list 
		tmp = (Node)malloc(sizeof(struct listNode));
		strcpy(tmp->word, word);
		tmp->Page = (struct page *)calloc(1, sizeof(struct page));
		tmp->Page->Next = (struct page *)malloc(sizeof(struct page));
		tmp->Page->Next->countNumber = 1;
		tmp->Page->Next->Next = NULL;
		tmp->Page->Next->PAGE=(char*)malloc(sizeof(FileName)+1);
		strcpy(tmp->Page->Next->PAGE, FileName);
		tmp->Page->Next->Position = (struct position *)malloc(sizeof(struct position));
		tmp->Page->Next->Position->place = count;
		tmp->Page->Next->Position->Next=NULL;
		tmp->Next = p->Next;
		p->Next = tmp;
}
	else{   //if the word has ever appeared
		
		struct page *_PAGE;

		_PAGE = p->Page->Next;
		while(NULL != _PAGE && strcmp( _PAGE->PAGE, FileName) != 0){ 
		//look for the page we are building index for in the word
			_PAGE = _PAGE->Next;
		}
		if(_PAGE == NULL){     //if the word hasn't appeared in the part \
								we staticed, add a new page to the word
			_PAGE = (struct page *)malloc(sizeof(struct page));
			_PAGE->countNumber = 1;
			_PAGE->Next = p->Page->Next;
			p->Page->Next = _PAGE;
			_PAGE->PAGE = (char*)malloc(sizeof(char)*30);
			strcpy(_PAGE->PAGE, FileName);
			_PAGE->Position = (struct position *)malloc(sizeof(struct position));
			_PAGE->Position->Next = NULL;
			_PAGE->Position->place = count;
		}
		else{      //if it has appeared, update the list
			struct position *_POS;
			_POS = (struct position *)malloc(sizeof(struct position));//store the new position the word appears
			_POS->Next = _PAGE->Position;
			_POS->place = count;
			_PAGE->countNumber++;
			_PAGE->Position=_POS;
			
		}	
	}
	WriteListNode(head, T->table[BKDRHash(word)]);  //write this word into the index file
}

/*search for words, which are inputed during the function is running*/
void Search(HashTable T)
{
	int i, j, times, flag, flag2;
	struct page **p;   //p[i] is used to point the head of the list of the pages contains the word[i] 
	struct page **pp;  //pp[i] is used to traverse the list of the pages contains the word[i] 
	struct position *potmp, *pofree; //usd to free the memory
	struct page *ptmp, *pfree;       //usd to free the memory
	Node pNode, ntmp, nfree;
	char words[20][40];
	do{                    //the search won't stop unless the user input an integer which is no more than 0
		flag = 1;
		printf("\n请输入查询单词个数(<20)：");
		scanf("%d", &times);
		printf("请输入单词\n");
		p = (struct page **)calloc(times, sizeof(struct page *));
		j=0;
		for(i = 0; i < times; i++){
			scanf("%s", words[j]);
			cut(words[j]);  //transfer all capital letters to small letters, and cut the not-letter,such as ';'
			words[j][stem(words[j],0,strlen(words[j])-1)+1]=0;  //stem the word
			if(CheckStop(words[j]) == 0){						//if s is not a stop word
				pNode = FindWord(words[j], T, &ntmp);	//build a list of the words and get the posiion of words[i]
				if(NULL != pNode){						//if the word is found
					p[j] = pNode->Page;
					j++;								
				}
				else{							//the word doesn't exist in the index
					flag = 0;					//make a flag
				}
					
				while(ntmp != NULL){			//free the excrescent memory
					if(ntmp != pNode){
						ptmp = ntmp->Page;
						while(ptmp != NULL){
							potmp = ptmp->Position;
							while(potmp != NULL){
								pofree = potmp;
								potmp = potmp->Next;
								free(pofree);
							}
							pfree = ptmp;
							ptmp = ptmp->Next;
							free(pfree);
						}
						nfree = ntmp;
						ntmp = ntmp->Next;
						free(nfree);
					}
					else{
						ntmp = ntmp->Next;
					}
				}
			}
			
		}
		if(flag == 0)					//the word doesn't exist in the index
			printf("关键词不存在！\n");
		else 
		{
			
			pp = (struct page **)calloc(j, sizeof(struct page *));
			flag2 = 1;
			if(NULL != p[0]){
				pp[0] = p[0]->Next;
				while(pp[0]!=NULL){			//search for the pages contain all the words except the stopping words\
											if there aren't any such files, then search failed
					flag = 0;
					 
					for(i = 1; i < j; i++){		//traverse the lists of all the target words
						pp[i] = p[i]->Next;
						while(NULL != pp[i] && strcmp(pp[0]->PAGE, pp[i]->PAGE)!=0){
							pp[i] = pp[i]->Next;
						}
						if(pp[i] == NULL){
							flag = 1;
							break;
						}
					}
					if(0 == flag){					//print the search result
						flag2 = 0;
						printf("Page  %s:\n", pp[0]->PAGE);
						for(i = 0; i < j; i++){
							printf("%s 出现次数：%d 出现位置：", words[i], pp[i]->countNumber);
							PrintPos(pp[i]);
							printf("\n");
						}
						printf("\n");	
					}
					pp[0] = pp[0]->Next;
				}
				
				
			}
			if(flag2 != 0){			//the word doesn't exist in the index
				printf("关键词不存在！\n");
			}
		} 
		for(i = 0; i < times; i++){  //free the memory we used
			while(NULL != p[i]){
				ptmp = p[i];
				p[i]= p[i]->Next;
				free(ptmp);
			}
		}

	}while(times > 0);
	
}

/*print the position of the word in the page*/
void PrintPos(struct page *p) 
{
	struct position *ppos, *ptmp;
	if(p){				//traverse the list 
		ppos = p->Position;
		while(ppos != NULL){
			printf("%d ", ppos->place);
			ptmp = ppos;
			ppos = ppos->Next;
			free(ptmp);
		}
	}
}
