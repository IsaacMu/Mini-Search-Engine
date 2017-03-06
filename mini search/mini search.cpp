/*
	使用了C++ STL中的vector容器和hash_map容器。
	提供了两个搜索模式，一个是根据单词的总频率筛选进入查询的关键词，另一个是根据单词在特定文章中出现的频率来筛选文章内容。
	不支持短语搜索，短语搜索将分解成若干个单词搜索。
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
typedef struct singleFileInfo *singleInfo;
struct singleFileInfo{
	string file;
	vector<int> address;
};

//word结构体表征某个单词所包含的所有信息
typedef struct word *Word;
struct word{
	int frequency;
	vector<singleInfo> info;
};

//STL里的哈希表 读写方式与数组的读写方式相同
hash_map<string, Word> inv_index;
hash_map<string, Word>::iterator it;

//存储stop words的容器
vector<string> stop_words;


//利用fileID和sceneID 拼接文件名
string getFileName(int fileID,int sceneID){
	char file[10], scene[10];
	string fileName;
	itoa(fileID, file, 10);
	itoa(sceneID, scene, 10);
	fileName = strcat(strcat(file, " "), scene); //link substring to a file name
	fileName += ".txt";
	return fileName;
}


//更新某个单词的信息容器
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

//对stemming 进行过滤
string filterStemming(string words){
	char *str,*filtstr;
	str = (char *)words.c_str();
	filtstr = ConverttoStem(str); //关键函数，出自头文件"stem.h"
	return filtstr;
}


//读取stop words词典 stopwords.txt
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

//识别stop words
int identifyStopWords(string words){
	if (find(stop_words.begin(), stop_words.end(), words) != stop_words.end())
		return 1;
	else
		return 0;
}

//建立倒排索引
void createWordDic(){
	int file, scene, isword = 0, file_begin;
	char ch;
	string fileName,nw="";
	Word word;

	readStopWords();//开始读stop words 文件

	for (file = 1001; file <= 1042; file++){
		for (scene = 1;; scene++){
			fileName = getFileName(file, scene);
			ifstream fp("works/" + fileName, ifstream::in | ifstream::binary); //打开文件。Make sure open file in ios::binary mode
			if (fp.is_open()){
				file_begin = fp.tellg();
				while (!fp.eof()){
					fp.get(ch);
					if (('a'<=ch && ch<='z')||('A'<=ch && ch<='Z')){
						isword = 1;
						nw += ch; //拼接字符串。to joint a word one by one
					}
					else{
						if (!isword)
							continue;

						transform(nw.begin(), nw.end(), nw.begin(),tolower);

						//判断是否为stop words
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

//根据单词频率来筛选，只返回频率较高的单词所相应的结果
int queryFunc_freq(string keywords,int freq_treshold){
	Word targetWord;
	singleInfo info;
	string fileName,lowerword;
	int flag = 0;

	lowerword = keywords;
	transform(lowerword.begin(), lowerword.end(), lowerword.begin(), tolower);//转换为小写。turn keywords to lowercase

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

string wordsArr[100];  //不得不用全局变量，解决通过函数传值不成功的问题。。

//分割 phrase 成若干个单词
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

	//thresholds 在这里改！！！！！
	int freq_threshold = 20, //单词出现的总频率门槛(mode 1)
		doc_threshold = 5;//单个文档中单词出现频率的门槛（mode 2)

	cout<<"倒排索引建立中..."<<endl;
	createWordDic();
	cout<<"倒排索引建立完成"<<endl;

	while (1){
		cout << endl;
		cout << "请输入搜索模式： 1、单词频率筛选   2、文档权重筛选" << endl;
		cin >> mode;
		getchar(); //去除输入末尾的回车符
		getRes = 0; //标记是否有搜索结果

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
