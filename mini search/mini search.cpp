/*
	ʹ����C++ STL�е�vector������hash_map������
	�ṩ����������ģʽ��һ���Ǹ��ݵ��ʵ���Ƶ��ɸѡ�����ѯ�Ĺؼ��ʣ���һ���Ǹ��ݵ������ض������г��ֵ�Ƶ����ɸѡ�������ݡ�
	��֧�ֶ��������������������ֽ�����ɸ�����������
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

//�ýṹ�����ĳ���������ڶ�Ӧ�ļ��е���Ϣ
typedef struct singleFileInfo *singleInfo;
struct singleFileInfo{
	string file;
	vector<int> address;
};

//word�ṹ�����ĳ��������������������Ϣ
typedef struct word *Word;
struct word{
	int frequency;
	vector<singleInfo> info;
};

//STL��Ĺ�ϣ�� ��д��ʽ������Ķ�д��ʽ��ͬ
hash_map<string, Word> inv_index;
hash_map<string, Word>::iterator it;

//�洢stop words������
vector<string> stop_words;


//����fileID��sceneID ƴ���ļ���
string getFileName(int fileID,int sceneID){
	char file[10], scene[10];
	string fileName;
	itoa(fileID, file, 10);
	itoa(sceneID, scene, 10);
	fileName = strcat(strcat(file, " "), scene); //link substring to a file name
	fileName += ".txt";
	return fileName;
}


//����ĳ�����ʵ���Ϣ����
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

//��stemming ���й���
string filterStemming(string words){
	char *str,*filtstr;
	str = (char *)words.c_str();
	filtstr = ConverttoStem(str); //�ؼ�����������ͷ�ļ�"stem.h"
	return filtstr;
}


//��ȡstop words�ʵ� stopwords.txt
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

//ʶ��stop words
int identifyStopWords(string words){
	if (find(stop_words.begin(), stop_words.end(), words) != stop_words.end())
		return 1;
	else
		return 0;
}

//������������
void createWordDic(){
	int file, scene, isword = 0, file_begin;
	char ch;
	string fileName,nw="";
	Word word;

	readStopWords();//��ʼ��stop words �ļ�

	for (file = 1001; file <= 1042; file++){
		for (scene = 1;; scene++){
			fileName = getFileName(file, scene);
			ifstream fp("works/" + fileName, ifstream::in | ifstream::binary); //���ļ���Make sure open file in ios::binary mode
			if (fp.is_open()){
				file_begin = fp.tellg();
				while (!fp.eof()){
					fp.get(ch);
					if (('a'<=ch && ch<='z')||('A'<=ch && ch<='Z')){
						isword = 1;
						nw += ch; //ƴ���ַ�����to joint a word one by one
					}
					else{
						if (!isword)
							continue;

						transform(nw.begin(), nw.end(), nw.begin(),tolower);

						//�ж��Ƿ�Ϊstop words
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

//���ݵ���Ƶ����ɸѡ��ֻ����Ƶ�ʽϸߵĵ�������Ӧ�Ľ��
int queryFunc_freq(string keywords,int freq_treshold){
	Word targetWord;
	singleInfo info;
	string fileName,lowerword;
	int flag = 0;

	lowerword = keywords;
	transform(lowerword.begin(), lowerword.end(), lowerword.begin(), tolower);//ת��ΪСд��turn keywords to lowercase

	if (inv_index.find(lowerword) != inv_index.end()){
		targetWord = inv_index[lowerword];
		if (targetWord->frequency < freq_treshold){
			cout <<">["<<keywords << "]����Ƶ�ʹ���"<<endl;
			return 0;
		}
		cout <<">["<< keywords<<"]�ܹ������ˣ�" << targetWord->frequency << " �Ρ�" << endl;
		for (int i = 0; i < targetWord->info.size(); i++){
			info = targetWord->info[i];
			fileName = info->file;
			cout << ">[" << keywords << "]��" << fileName << "�г��ֵ�λ��Ϊ��";
			for (int j = 0; j < info->address.size(); j++){
				cout << info->address[j] << " ";
			}
			cout << endl;
		}
		flag = 1;
	}
	if(!flag){
		cout << ">" << "û�в��ҵ�����[" << keywords << "]���ĵ�" << endl;
	}

	return flag;
}

//�����ĵ�Ȩ��ɸѡ��ֻ���ذ����ؼ���Ƶ�ʽϸߵ��ĵ�
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
			cout << ">[" << keywords << "]����Ƶ�ʹ���" << endl;
			return 0;
		}

		cout << ">[" << keywords << "]�ܹ������ˣ�" << targetWord->frequency << " �Ρ�" << endl;
		for (int i = 0; i < targetWord->info.size(); i++){
			info = targetWord->info[i];
			fileName = info->file;
			if (info->address.size() >= doc_freq){
				cout << ">[" << keywords << "]��" << fileName << "�г��ֵ�λ��Ϊ��";
				for (int j = 0; j < info->address.size(); j++){
					cout << info->address[j] << " ";
				}
				cout << endl;
			}
		}
		flag = 1;
	}
	if (!flag){
		cout << ">" << "û�в��ҵ�����[" << keywords << "]���ĵ�" << endl;
	}

	return flag;

}

string wordsArr[100];  //���ò���ȫ�ֱ��������ͨ��������ֵ���ɹ������⡣��

//�ָ� phrase �����ɸ�����
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

	//thresholds ������ģ���������
	int freq_threshold = 20, //���ʳ��ֵ���Ƶ���ż�(mode 1)
		doc_threshold = 5;//�����ĵ��е��ʳ���Ƶ�ʵ��ż���mode 2)

	cout<<"��������������..."<<endl;
	createWordDic();
	cout<<"���������������"<<endl;

	while (1){
		cout << endl;
		cout << "����������ģʽ�� 1������Ƶ��ɸѡ   2���ĵ�Ȩ��ɸѡ" << endl;
		cin >> mode;
		getchar(); //ȥ������ĩβ�Ļس���
		getRes = 0; //����Ƿ����������

		//mode 1
		if (mode == 1){
			cout << "notice:��Ƶ������[" << freq_threshold << "]�ĵ��ʽ����᷵�ؽ����" << endl;
			cout << "��������Ҫ���ҵĵ��ʣ�";
			getline(cin, keywords);
			split(keywords);
			for (m = 0; wordsArr[m]!= ""; m++){
				if (queryFunc_freq(wordsArr[m],freq_threshold))
					getRes = 1;
			}
			if (getRes){
				cout << "��ѡ��1������Ӧ�ļ�  2����������" << endl;
				cin >> choice;
				getchar();
				if (choice == 1){
					cout << "��������Ӧ���ļ�����";
					getline(cin, fileName);
					ifstream fp("works/" + fileName, ifstream::in | ifstream::binary);
					if (fp.is_open()){
						while (!fp.eof()){
							fp.get(ch);
							cout << ch;
						}
					}
					else{
						cout << "�޷���ָ���ļ�����ȷ�Ϻ����ԡ�";
					}
					cout << endl;
				}
			}
			cout << endl;
		}
		// mode 2
		else{
			cout << "notice:�����᷵�ذ����ؼ�������[" << doc_threshold << "]�ε��ĵ���" << endl;
			cout << "��������Ҫ���ҵĵ��ʣ�";
			getline(cin, keywords);
			split(keywords);
			for (m = 0; wordsArr[m] != ""; m++){
				if (queryFunc_doc(wordsArr[m],doc_threshold))
					getRes = 1;
			}
			if (getRes){
				cout << "��ѡ��1������Ӧ�ļ�  2����������" << endl;
				cin >> choice;
				getchar();
				if (choice == 1){
					cout << "��������Ӧ���ļ�����";
					getline(cin, fileName);
					ifstream fp("works/" + fileName, ifstream::in | ifstream::binary);
					if (fp.is_open()){
						while (!fp.eof()){
							fp.get(ch);
							cout << ch;
						}
					}
					else{
						cout << "�޷���ָ���ļ�����ȷ�Ϻ����ԡ�";
					}
					cout << endl;
				}
			}
			cout << endl;
		}
	}

}
