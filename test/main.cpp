/*
	???C++ STL??vector???hash_map???
	???????????????????????????????????????????????????????????
	????????????????????????
*/

#include <iostream>
#include <hash_map>
#include <iterator>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "stem.h"
using namespace std;

//????????????????????
typedef struct singleFileInfo *singleInfo;
struct singleFileInfo{
	string file;
	vector<int> address;
};

//word?????????????????
typedef struct word *Word;
struct word{
	int frequency;
	vector<singleInfo> info;
};

//STL????? ??????????????
hash_map<string, Word> inv_index;
hash_map<string, Word>::iterator it;

//??stop words???
vector<string> stop_words;


//??fileID?sceneID ?????
string getFileName(int fileID,int sceneID){
	char file[10], scene[10];
	string fileName;
	itoa(fileID, file, 10);
	itoa(sceneID, scene, 10);
	fileName = strcat(strcat(file, " "), scene); //link substring to a file name
	fileName += ".txt";
	return fileName;
}


//???????????
void updateWordInfo(Word word,string fileName,int position){
	
	int isExit = 0,i;
	singleInfo info;
	for (i = 0; i < word->info.size(); i++){
		info = word->info[i];
		if (info->file == fileName){
			info->address.push_back(position);
			isExit = 1;
			break;
		}
	}
	if (!isExit){
		info = new struct singleFileInfo;
		info->file = fileName;
		info->address.push_back(position);
		word->info.push_back(info);
	}

}

//?stemming ????
string filterStemming(string words){
	char *str,*filtstr;
	str = (char *)words.c_str();
	filtstr = ConverttoStem(str); //??????????"stem.h"
	return filtstr;
}


//??stop words?? stopwords.txt
void readStopWords(){
	string nw = "";
	char ch;
	int isword;
	ifstream fp("dict/stopwords.txt", ifstream::in | ifstream::binary);
	if (fp.is_open()){
		while (!fp.eof()){
			fp.get(ch);
			if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')){
				isword = 1;
				nw += ch;
			}
			else{
				if (!isword)
					continue;
				stop_words.push_back(nw);
				nw = "";
				isword = 0;
			}
		}
	}
}

//??stop words
int identifyStopWords(string words){
	if (find(stop_words.begin(), stop_words.end(), words) != stop_words.end())
		return 1;
	else
		return 0;
}

//??????
void createWordDic(){
	int file, scene, isword = 0, file_begin;
	char ch;
	string fileName,nw="";
	Word word;

	readStopWords();//???stop words ??

	for (file = 1001; file <= 1042; file++){
		for (scene = 1;; scene++){
			fileName = getFileName(file, scene);
			ifstream fp("works/" + fileName, ifstream::in | ifstream::binary); //?????Make sure open file in ios::binary mode
			if (fp.is_open()){
				file_begin = fp.tellg();
				while (!fp.eof()){
					fp.get(ch);
					if (('a'<=ch && ch<='z')||('A'<=ch && ch<='Z')){
						isword = 1;
						nw += ch; //??????to joint a word one by one
					}
					else{
						if (!isword)
							continue;

						transform(nw.begin(), nw.end(), nw.begin(),tolower);

						//?????stop words
						if (identifyStopWords(nw)){
							nw = "";
							isword = 0;
							continue;
						}

						nw = filterStemming(nw);
						//if the word already exits
						if (inv_index.find(nw) != inv_index.end()){
							word = inv_index[nw];
							word->frequency++;
						}
						//if not exits, create a word kv
						else{
							word = new struct word;
							word->frequency = 1;
						}
						
						updateWordInfo(word, fileName, (int)fp.tellg()-file_begin-nw.length());
						inv_index[nw] = word;
						nw = "";
						isword = 0;
					}
				}
				fp.close();
			}
			else{
				fp.close();
				break;
			}
		}
	}

}

//??????????????????????????
int queryFunc_freq(string keywords,int freq_treshold){
	Word targetWord;
	singleInfo info;
	string fileName,lowerword;
	int flag = 0;

	lowerword = keywords;
	transform(lowerword.begin(), lowerword.end(), lowerword.begin(), tolower);//??????turn keywords to lowercase

	if (inv_index.find(lowerword) != inv_index.end()){
		targetWord = inv_index[lowerword];
		if (targetWord->frequency < freq_treshold){
			cout <<">["<<keywords << "]??????"<<endl;
			return 0;
		}
		cout <<">["<< keywords<<"]??????" << targetWord->frequency << " ??" << endl;
		for (int i = 0; i < targetWord->info.size(); i++){
			info = targetWord->info[i];
			fileName = info->file;
			cout << ">[" << keywords << "]?" << fileName << "????????";
			for (int j = 0; j < info->address.size(); j++){
				cout << info->address[j] << " ";
			}
			cout << endl;
		}
		flag = 1;
	}
	if(!flag){
		cout << ">" << "???????[" << keywords << "]???" << endl;
	}

	return flag;
}

//????????????????????????
int queryFunc_doc(string keywords,int doc_freq){
	Word targetWord;
	singleInfo info;
	string fileName, lowerword;
	int flag = 0;

	lowerword = keywords;
	transform(lowerword.begin(), lowerword.end(), lowerword.begin(), tolower);//turn keywords to lowercase

	if (inv_index.find(lowerword) != inv_index.end()){
		targetWord = inv_index[lowerword];
		if (targetWord->frequency < doc_freq){
			cout << ">[" << keywords << "]??????" << endl;
			return 0;
		}

		cout << ">[" << keywords << "]??????" << targetWord->frequency << " ??" << endl;
		for (int i = 0; i < targetWord->info.size(); i++){
			info = targetWord->info[i];
			fileName = info->file;
			if (info->address.size() >= doc_freq){
				cout << ">[" << keywords << "]?" << fileName << "????????";
				for (int j = 0; j < info->address.size(); j++){
					cout << info->address[j] << " ";
				}
				cout << endl;
			}
		}
		flag = 1;
	}
	if (!flag){
		cout << ">" << "???????[" << keywords << "]???" << endl;
	}

	return flag;

}

string wordsArr[100];  //?????????????????????????

//?? phrase ??????
void split(string keywords){
	char ch;
	int i,k=0,begin=0; 
	memset(wordsArr, 0, sizeof(string)* 100);
	for (i = 0; i < keywords.length(); i++){
		ch = keywords[i];
		if (ch == ' '){
			wordsArr[k++] = keywords.substr(begin, i - begin);
			begin = i+1;
		}
		if (i == keywords.length() - 1){
			wordsArr[k++] = keywords.substr(begin, i - begin + 1);
		}
	}
	wordsArr[k] = "";
}


int main(){
	int m;
	string keywords,fileName;
	char ch;
	int choice, mode, getRes = 0;

	//thresholds ?????????
	int freq_threshold = 20, //??????????(mode 1)
		doc_threshold = 5;//???????????????mode 2)

	cout<<"???????..."<<endl;
	createWordDic();
	cout<<"????????"<<endl;

	while (1){
		cout << endl;
		cout << "???????? 1???????   2???????" << endl;
		cin >> mode;
		getchar(); //??????????
		getRes = 0; //?????????

		//mode 1
		if (mode == 1){
			cout << "notice:?????[" << freq_threshold << "]???????????" << endl;
			cout << "???????????";
			getline(cin, keywords);
			split(keywords);
			for (m = 0; wordsArr[m]!= ""; m++){
				if (queryFunc_freq(wordsArr[m],freq_threshold))
					getRes = 1;
			}
			if (getRes){
				cout << "????1???????  2?????" << endl;
				cin >> choice;
				getchar();
				if (choice == 1){
					cout << "??????????";
					getline(cin, fileName);
					ifstream fp("works/" + fileName, ifstream::in | ifstream::binary);
					if (fp.is_open()){
						while (!fp.eof()){
							fp.get(ch);
							cout << ch;
						}
					}
					else{
						cout << "????????????????";
					}
					cout << endl;
				}
			}
			cout << endl;
		}
		// mode 2
		else{
			cout << "notice:????????????[" << doc_threshold << "]?????" << endl;
			cout << "???????????";
			getline(cin, keywords);
			split(keywords);
			for (m = 0; wordsArr[m] != ""; m++){
				if (queryFunc_doc(wordsArr[m],doc_threshold))
					getRes = 1;
			}
			if (getRes){
				cout << "????1???????  2?????" << endl;
				cin >> choice;
				getchar();
				if (choice == 1){
					cout << "??????????";
					getline(cin, fileName);
					ifstream fp("works/" + fileName, ifstream::in | ifstream::binary);
					if (fp.is_open()){
						while (!fp.eof()){
							fp.get(ch);
							cout << ch;
						}
					}
					else{
						cout << "????????????????";
					}
					cout << endl;
				}
			}
			cout << endl;
		}
	}

}

