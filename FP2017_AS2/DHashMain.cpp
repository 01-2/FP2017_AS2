#pragma warning(disable: 4996)

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>

#define MIN_ORDER 2
#define KEY_LENGTH 9
#define BUCKET_SIZE 128

using namespace std;

unsigned int LIST_SIZE;

class StElement {
	char name[20];
	unsigned int studentID;
	float score;
	unsigned int advisorID;
public:
	// set class elements
	void setName(char* _name) { strcpy( name, _name ); }
	void setSID(unsigned int _studentID) { studentID = _studentID; }
	void setScore(float _score) { score = _score; };
	void setAID(unsigned int _advisorID) { advisorID = _advisorID; }

	// get class elements
	char* getName() { return name; }
	unsigned int getSID() { return studentID; }
	float getScore() { return score; }
	unsigned int getAID() { return advisorID; }

	// functions
	void setElement(char* , unsigned int , float , unsigned int );
};

void StElement::setElement(char* _name, unsigned int _studentID, float _score, unsigned int _advisorID) {
	setName(_name);
	setSID(_studentID);
	setScore(_score);
	setAID(_advisorID);
}

int pow_2(int p){
	int i;
	int x = 1;
	for (i = 0; i<p; i++)
		x *= 2;
	return x;
}

//  pseudokey�� ���� return �Ѵ�. 
int makePseudokey(int key){
	return key % pow_2(KEY_LENGTH);
}

//  ���� n bit�� �����ش�. 
int foreget(int k, int n){
	return k / pow_2(KEY_LENGTH - n);
}

typedef struct Leaf
{
	int header;
	int count;  // ����� record�� ����
	StElement** pRecord; // ����� record�� �ּҸ� ����Ű�� pointer�� �迭
} leaf;

typedef struct Directory
{
	int header;
	int divCount;  // directory�� header�� ���� header�� ���� leaf pair�� ����
	leaf** entry;
} directory;

StElement* retrieval(int key, directory* dir)
{
	int i;
	int pseudokey = makePseudokey(key);    // pseudokey ����
	int index = foreget(pseudokey, dir->header);  // entry index
	leaf* bucket = dir->entry[index];
	for (i = 0; i<bucket->count; i++)     // �ش� leaf���� ã�����ϴ� key�� ã�´�. 
	{
		StElement* ptr = dir->entry[index]->pRecord[i];
		if (ptr->getSID() == key)
			return ptr;        // ã���� record�� �ּҸ� ����
	}
	return NULL;         // ������ NULL�� ����
}

int insertRecord(StElement* rec, directory* dir){
	int i, ind;
	int key = rec->getSID();        // ���ԵǴ� record�� key
	int pseudokey = makePseudokey(key);    // pseudokey
	int index = foreget(pseudokey, dir->header); // entry�� ��ȣ�� ã�´�. 
	leaf* bucket = dir->entry[index];    // �����ϰ��� �ϴ� leaf


										 ////////// 1. bucket�� Ȯ���ؼ� �����ϰ��� �ϴ� Ű�� �̹� �����ϴ��� Ȯ��
	for (i = 0; i<bucket->count; i++){
		if (bucket->pRecord[i]->getSID() == key){
			printf("\n\nThe key is already exist!!!\n");
			return 0;  // ������ ����
		}
	}

	////////// 2. bucket�� ������ ������ �����ִٸ� ��ĭ�� �����Ѵ�. 
	if (bucket->count < BUCKET_SIZE){
		bucket->pRecord[bucket->count] = rec;
		bucket->count++;
		return 1;   // ��������
	}

	////////// 3. bucket�� �� á�ٸ� overflow ó��
	while (1){
		int n;
		leaf* newBucket;  // ���� ���� bucket;
						  //////  3-1. d < t+1�ΰ�� => ���� ���丮�� �ι� �ø��� 
		if (dir->header < bucket->header + 1){
			int numEntry;    // ���� ������� entry�� ��
			leaf** newEntry;
			dir->header++;
			numEntry = pow_2(dir->header);
			newEntry = (leaf**)malloc(sizeof(leaf*)*numEntry);
			for (i = 0; i<numEntry / 2; i++)  // entry�� �þ ��ŭ �й�
				newEntry[i * 2] = newEntry[i * 2 + 1] = dir->entry[i];
			dir->divCount = 0;
			free(dir->entry);
			dir->entry = newEntry;
		}
		////// 3-2. ���� overflow�� ���� leaf�� split�Ѵ�. 
		// 3-2-1. ���ο� bucket ����
		newBucket = (leaf*)malloc(sizeof(leaf));
		newBucket->header = bucket->header;
		newBucket->count = 0;
		newBucket->pRecord = (StElement**)malloc(sizeof(StElement*) * BUCKET_SIZE);

		// 3-2-2. bucket���� record �й�
		bucket->count = 0;
		for (i = 0; i<BUCKET_SIZE; i++)
		{
			if (foreget(makePseudokey(bucket->pRecord[i]->getSID()), bucket->header + 1) % 2 == 0)
			{
				bucket->pRecord[bucket->count] = bucket->pRecord[i];
				bucket->count++;
			}
			else
			{
				newBucket->pRecord[newBucket->count] = bucket->pRecord[i];
				newBucket->count++;
			}
		}
		bucket->header++;
		newBucket->header++;
		if (bucket->header == dir->header)
			dir->divCount++;

		// 3-2-3. entry -> leaf ���� pointer ����
		n = pow_2(dir->header - bucket->header + 1);      // �������� entry ��
		ind = foreget(makePseudokey(key), bucket->header - 1) * n;   // �������� entry�� ù��° entry�� index

		for (i = 0; i<n / 2; i++, ind++)        // �ΰ��� bucket���� ����
			dir->entry[ind] = bucket;
		for (i = 0; i<n / 2; i++, ind++)
			dir->entry[ind] = newBucket;

		// 3-2-4. ���� �ٽ� ���Ե� ��带 �����Ѵ�. 
		index = foreget(pseudokey, dir->header);
		bucket = dir->entry[index];
		if (bucket->count < BUCKET_SIZE) // ������ ���� bucket�� full�� �ƴϸ� ����
		{
			bucket->pRecord[bucket->count] = rec;
			bucket->count++;
			return 1;   // ��������
		}
		// ������ ���� full�� �ƴϸ� 2�� ó������ ���ư��� �ٽ� directory�� �ø���. 
	}
}


void printTable(directory* dir, ofstream& os)
{
	int i, k, j;
	leaf* cBucket;
	leaf* pBucket = NULL;
	for (i = 0; i<pow_2(dir->header); i++)
	{
		int c = i;
		for (k = pow_2(dir->header - 1); k>0; k /= 2)
		{
			printf("%d", c / k);
			os << c / k;
			c = c%k;
		}
		printf("\t");
		os << "\t";
		cBucket = dir->entry[i];
		if (cBucket != pBucket)
		{
			printf("%d\t", cBucket->header);
			printf("%d\t", cBucket->count);
			os << cBucket->header << "\t";
			os << cBucket->count << "\t" << endl;
			for (j = 0; j<cBucket->count; j++)
			{
				printf("%d:", cBucket->pRecord[j]->getSID());
				os << cBucket->pRecord[j]->getSID() << " ";
				printf("%s, ", cBucket->pRecord[j]->getName());
				os << cBucket->pRecord[j]->getName() << endl;
			}
		}
		printf("\n");
		os << endl;
		pBucket = cBucket;
	}
}



StElement getToken(char* convStr) {
	StElement bufElement;
	char* tokList;

	tokList = strtok(convStr, ",");
	bufElement.setName(tokList);

	tokList = strtok(NULL, ",");
	bufElement.setSID(atoi(tokList));

	tokList = strtok(NULL, ",");
	bufElement.setScore(atof(tokList));

	tokList = strtok(NULL, ",");
	bufElement.setAID(atoi(tokList));

	return bufElement;
}
string GetBinaryFromHex(string sHex){
	string sReturn = "";
	for (int i = 0; i < sHex.length(); ++i){
		switch (sHex[i]){
			case '0': sReturn.append("0000"); break;
			case '1': sReturn.append("0001"); break;
			case '2': sReturn.append("0010"); break;
			case '3': sReturn.append("0011"); break;
			case '4': sReturn.append("0100"); break;
			case '5': sReturn.append("0101"); break;
			case '6': sReturn.append("0110"); break;
			case '7': sReturn.append("0111"); break;
			case '8': sReturn.append("1000"); break;
			case '9': sReturn.append("1001"); break;
			case 'a': sReturn.append("1010"); break;
			case 'b': sReturn.append("1011"); break;
			case 'c': sReturn.append("1100"); break;
			case 'd': sReturn.append("1101"); break;
			case 'e': sReturn.append("1110"); break;
			case 'f': sReturn.append("1111"); break;
		}
	}
	return sReturn;
}

int main() {
	
	ifstream is;
	ofstream os;
	is.open("sampleData.csv");
	os.open("result.txt");
	if (is.is_open() == false){
		cout << "File open Failed" << endl; return 1;
	}

	char enter;
	char* convStr;
	string bufStr;
	StElement bufElement;
	vector<StElement> oData;
	vector<unsigned int> hashList;

	// get LIST_SIZE
	getline(is, bufStr);
	convStr = new char[sizeof(bufStr)];
	
	strcpy(convStr, bufStr.c_str());
	convStr = strtok(convStr, ",");
	LIST_SIZE = atoi(convStr);

	for (int i = 0; i < LIST_SIZE; i++) {
		getline(is, bufStr);
		strcpy(convStr, bufStr.c_str());
		bufElement = getToken(convStr);
		oData.push_back(bufElement);
	}
	
	directory* dir;
	dir = (directory*)malloc(sizeof(directory));
	dir->header = MIN_ORDER;
	dir->divCount = pow_2(MIN_ORDER - 1);
	dir->entry = (leaf**)malloc(sizeof(leaf*) * pow_2(MIN_ORDER));
	for (int i = 0; i<pow_2(MIN_ORDER); i++)
	{
		dir->entry[i] = (leaf*)malloc(sizeof(leaf));
		dir->entry[i]->header = 2;
		dir->entry[i]->count = 0;
		dir->entry[i]->pRecord = (StElement**)malloc(sizeof(StElement*)*BUCKET_SIZE);
	}

	for(int i = 0; i < LIST_SIZE; i++){
		StElement* rec = &oData[i];
		insertRecord(rec, dir);
		printf("\n***  key : %d, name : %s �� record�� ����, pseudokey : %d ***\n",
			rec->getSID(), rec->getName(), makePseudokey(rec->getSID()));
//		printf("\n\nPress Enter!!\n");
//		cin >> enter;
	}
	printTable(dir, os);

	is.close();
	os.close();
	return 0;
}
