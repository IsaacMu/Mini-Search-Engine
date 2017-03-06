/*
	使用了C++ STL中的vector容器和hash_map容器。
	提供了两个搜索模式，一个是根据单词的总频率筛选进入查询的关键词，另一个是根据单词在特定文章中出现的频率来筛选文章内容。
	不支持短语搜索，短语搜索将分解成若干个单词搜索。

	# Use vector and hash_map in C++ STL.
	# Provide two searching mode.One is to search accroding to the total frequency of the keyword, and the other is to search according to the 
	frequency in certain documents.
	# Split a phrase to several words.

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

//该结构体表征某个单词所在对应文件中的信息
//to denote the infomation of a word in certain document.
typedef struct singleFileInfo *singleInfo;
struct singleFileInfo{
	string file;
	vector<int> address;
};

//word结构体表征某个单词所包含的所有信息
//to denote the total information of certain word.
typedef struct word *Word;
struct word{
	int frequency;
	vector<singleInfo> info;
};

//hash_map in STL and its iterator
hash_map<string, Word> inv_index;
hash_map<string, Word>::iterator it;

//to store stop words
vector<string> stop_words;


//to joint the fileID and sceneID to a file name.
string getFileName(int fileID,int sceneID){
	char file[10], scene[10];
	string fileName;
	itoa(fileID, file, 10);
	itoa(sceneID, scene, 10);
	fileName = strcat(strcat(file, " "), scene); //link substring to a file name
	fileName += ".txt";
	return fileName;
}



//update the information of a word
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

//stemming filter
string filterStemming(string words){
	char *str,*filtstr;
	str = (char *)words.c_str();
	filtstr = ConverttoStem(str); //key function ,coming from "stem.h"
	return filtstr;
}


//read stop words from "stopwords.txt"
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

//identify stop words
int identifyStopWords(string words){
	if (find(stop_words.begin(), stop_words.end(), words) != stop_words.end())
		return 1;
	else
		return 0;
}

//create inverted index
void createWordDic(){
	int file, scene, isword = 0, file_begin;
	char ch;
	string fileName,nw="";
	Word word;

	readStopWords();//begin to read words

	for (file = 1001; file <= 1042; file++){
		for (scene = 1;; scene++){
			fileName = getFileName(file, scene);
			ifstream fp("works/" + fileName, ifstream::in | ifstream::binary); //Make sure open file in ios::binary mode
			if (fp.is_open()){
				file_begin = fp.tellg();
				while (!fp.eof()){
					fp.get(ch);
					if (('a'<=ch && ch<='z')||('A'<=ch && ch<='Z')){
						isword = 1;
						nw += ch; //o joint a word one by one
					}
					else{
						if (!isword)
							continue;

						transform(nw.begin(), nw.end(), nw.begin(),tolower);

						//to check whether it is stop word
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

//filter by the frequency of words.
int queryFunc_freq(string keywords,int freq_treshold){
	Word targetWord;
	singleInfo info;
	string fileName,lowerword;
	int flag = 0;

	lowerword = keywords;
	transform(lowerword.begin(), lowerword.end(), lowerword.begin(), tolower);//turn keywords to lowercase
	lowerword = filterStemming(lowerword);

	if (inv_index.find(lowerword) != inv_index.end()){
		targetWord = inv_index[lowerword];
		if (targetWord->frequency < freq_treshold){
			cout <<">["<<keywords << "]出现频率过少"<<endl;
			return 0;
		}
		cout <<">["<< keywords<<"]总共出现了：" << targetWord->frequency << " 次。" << endl;
		for (int i = 0; i < targetWord->info.size(); i++){
			info = targetWord->info[i];
			fileName = info->file;
			cout << ">[" << keywords << "]在" << fileName << "中出现的位置为：";
			for (int j = 0; j < info->address.size(); j++){
				cout << info->address[j] << " ";
			}
			cout << endl;
		}
		flag = 1;
	}
	if(!flag){
		cout << ">" << "没有查找到包含[" << keywords << "]的文档" << endl;
	}

	return flag;
}

//根据文档权重筛选，只返回包含关键词频率较高的文档
//filter by documents which include high frequency of keywords.
int queryFunc_doc(string keywords,int doc_freq){
	Word targetWord;
	singleInfo info;
	string fileName, lowerword;
	int flag = 0;

	lowerword = keywords;
	transform(lowerword.begin(), lowerword.end(), lowerword.begin(), tolower);//turn keywords to lowercase
	lowerword = filterStemming(lowerword);

	if (inv_index.find(lowerword) != inv_index.end()){
		targetWord = inv_index[lowerword];
		if (targetWord->frequency < doc_freq){
			cout << ">[" << keywords << "]出现频率过少" << endl;
			return 0;
		}

		cout << ">[" << keywords << "]总共出现了：" << targetWord->frequency << " 次。" << endl;
		for (int i = 0; i < targetWord->info.size(); i++){
			info = targetWord->info[i];
			fileName = info->file;
			if (info->address.size() >= doc_freq){
				cout << ">[" << keywords << "]在" << fileName << "中出现的位置为：";
				for (int j = 0; j < info->address.size(); j++){
					cout << info->address[j] << " ";
				}
				cout << endl;
			}
		}
		flag = 1;
	}
	if (!flag){
		cout << ">" << "没有查找到包含[" << keywords << "]的文档" << endl;
	}

	return flag;

}

string wordsArr[100];

//split a phrase to severl words
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

	int freq_threshold = 1, //weight of words. (mode 1)
		doc_threshold = 1;//weight of documents. (mode 2)

	cout<<"倒排索引建立中..."<<endl;
	createWordDic();
	cout<<"倒排索引建立完成"<<endl;

	while (1){
		cout << endl;
		cout << "请输入搜索模式： 1、单词频率筛选   2、文档权重筛选" << endl;
		cin >> mode;
		getchar(); //to delete the "enter"
		getRes = 0; //to mark whether there is searching result

		//mode 1
		if (mode == 1){
			cout << "notice:总频次少于[" << freq_threshold << "]的单词将不会返回结果。" << endl;
			cout << "请输入需要查找的单词：";
			getline(cin, keywords);
			split(keywords);
			for (m = 0; wordsArr[m]!= ""; m++){
				if (queryFunc_freq(wordsArr[m],freq_threshold))
					getRes = 1;
			}
			if (getRes){
				cout << "请选择：1、打开相应文件  2、继续搜索" << endl;
				cin >> choice;
				getchar();
				if (choice == 1){
					cout << "请输入相应的文件名：";
					getline(cin, fileName);
					ifstream fp("works/" + fileName, ifstream::in | ifstream::binary);
					if (fp.is_open()){
						while (!fp.eof()){
							fp.get(ch);
							cout << ch;
						}
					}
					else{
						cout << "无法打开指定文件，请确认后再试。";
					}
					cout << endl;
				}
			}
			cout << endl;
		}
		// mode 2
		else{
			cout << "notice:将不会返回包含关键词少于[" << doc_threshold << "]次的文档。" << endl;
			cout << "请输入需要查找的单词：";
			getline(cin, keywords);
			split(keywords);
			for (m = 0; wordsArr[m] != ""; m++){
				if (queryFunc_doc(wordsArr[m],doc_threshold))
					getRes = 1;
			}
			if (getRes){
				cout << "请选择：1、打开相应文件  2、继续搜索" << endl;
				cin >> choice;
				getchar();
				if (choice == 1){
					cout << "请输入相应的文件名：";
					getline(cin, fileName);
					ifstream fp("works/" + fileName, ifstream::in | ifstream::binary);
					if (fp.is_open()){
						while (!fp.eof()){
							fp.get(ch);
							cout << ch;
						}
					}
					else{
						cout << "无法打开指定文件，请确认后再试。";
					}
					cout << endl;
				}
			}
			cout << endl;
		}
	}

}
