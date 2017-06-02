#pragma warning(disable: 4996)

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

using namespace std;

#define MIN_ORDER 2
#define KEY_LENGTH 6
#define BUCKET_SIZE 128
#define DEFAULT_BUCKET_SIZE 2
#define DEFAULT_SLOT_SIZE 128
#define MM 64 // B+Ʈ���� ����
#define M (MM*2+1) // Ȧ�� ����,key���� 128���� ����
#define MAX 500 // ť�� ũ��
#define TR 500 // ������ ũ��

typedef struct Node {
	int count;
	int sid[M - 1];//�л�id �ߺ���쿡 ���� ������ Ȯ���ϱ� ����
	double score[M - 1];//score
	int p[M]; //�������� �����ġ ����
	struct Node* branch[M];//�ּ�
}node;

node* root;//��Ʈ node
int front = 0, rear = 0; //ť�� �����ϱ� ����
node* queue[MAX];
int findcnt;
int leafnum = 0;//leaf��� �������ϱ� ����

int* search(double k); //key Ž�� �Լ�
int* insertItem(double k, int sid, int blocknum);
node* bput(node* k);//ť �Է� �Լ�
node* get();
int isEmpty(); //ť ���� üũ
void sequencialSearch();
void indexNodePrint(node* t);

unsigned int LIST_SIZE;

class students {
public:
	char name[20];
	unsigned sid;
	float score;
	unsigned aid;
};

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
// ������ ���ִ� �Լ�
int pow_2(int p){
	int i;
	int x = 1;
	for (i = 0; i<p; i++)
		x *= 2;
	return x;
}
// pseudokey�� ���� return �Ѵ�.
// KEY ���̸�ŭ ���� �� mod ������ ���� ���� KEY_LENGTH Ű��ŭ �߶󳽴�
int makePseudokey(int key){
	return key % pow_2(KEY_LENGTH);
}
//  ���� n bit�� �����ش�.
int nreturn(int k, int n){
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
	int index = nreturn(pseudokey, dir->header);  // entry index
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
	int index = nreturn(pseudokey, dir->header); // entry�� ��ȣ�� ã�´�.
	leaf* bucket = dir->entry[index];    // �����ϰ��� �ϴ� leaf

	// bucket�� Ȯ���ؼ� �����ϰ��� �ϴ� Ű�� �̹� �����ϴ��� Ȯ��
	for (i = 0; i<bucket->count; i++){
		if (bucket->pRecord[i]->getSID() == key){
			printf("\n\nThe key is already exist!!!\n");
			return 0;  // ������ ����
		}
	}

	// bucket�� ������ ������ �����ִٸ� ��ĭ�� �����Ѵ�.
	if (bucket->count < BUCKET_SIZE){
		bucket->pRecord[bucket->count] = rec;
		bucket->count++;
		return 1;   // ��������
	}

	// bucket�� �� á�ٸ� overflow ó��
	while (1){
		int n;
		leaf* newBucket;  // ���� ���� bucket;
						  //////  d < t+1�ΰ�� => ���� ���丮�� �ι� �ø���
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
		// overflow�� ���� leaf�� split�Ѵ�.
		// ���ο� bucket ����
		newBucket = (leaf*)malloc(sizeof(leaf));
		newBucket->header = bucket->header;
		newBucket->count = 0;
		newBucket->pRecord = (StElement**)malloc(sizeof(StElement*) * BUCKET_SIZE);

		// bucket���� record �й�
		bucket->count = 0;
		for (i = 0; i<BUCKET_SIZE; i++)
		{
			if (nreturn(makePseudokey(bucket->pRecord[i]->getSID()), bucket->header + 1) % 2 == 0)
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

		// entry -> leaf ���� pointer ����
		n = pow_2(dir->header - bucket->header + 1);      // �������� entry ��
		ind = nreturn(makePseudokey(key), bucket->header - 1) * n;   // �������� entry�� ù��° entry�� index

		for (i = 0; i<n / 2; i++, ind++)        // �ΰ��� bucket���� ����
			dir->entry[ind] = bucket;
		for (i = 0; i<n / 2; i++, ind++)
			dir->entry[ind] = newBucket;

		// �ٽ� ���Ե� ��带 �����Ѵ�.
		index = nreturn(pseudokey, dir->header);
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
void printDB(directory* dir, ofstream& os){
	int i, j;
	leaf* cBucket;
	leaf* pBucket = NULL;
	for (i = 0; i<pow_2(dir->header); i++){
		int c = i;
		os << c;
		cBucket = dir->entry[i];
		if (cBucket != pBucket){
			os << "\t" << cBucket->count << "\t" << endl;
			for (j = 0; j<cBucket->count; j++){
				os << cBucket->pRecord[j]->getName() << ", ";
				os << cBucket->pRecord[j]->getSID() << ", ";
				os << cBucket->pRecord[j]->getScore() << ", ";
				os << cBucket->pRecord[j]->getAID() << endl;
			}
		}
		else {
			os << "\t" << "P" << "\t" << endl;
		}
		pBucket = cBucket;
	}
}

void printTable(vector<StElement> origin){
	FILE *fp;
	fp = fopen("Students.hash", "wb");
	int getInt;
	int getPseudo;
	for (int i = 0; i < LIST_SIZE; i++) {
		StElement* rec = &origin[i];
		getInt = rec->getSID();
		fwrite(&getInt, sizeof(int), 1, fp);
		fwrite(" ", sizeof(" "), 1, fp);
		getPseudo = makePseudokey(rec->getSID());
		fwrite(&getPseudo, sizeof(int), 1, fp);
		fwrite("\n", sizeof("\n"), 1, fp);
	}
	fclose(fp);
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

int* search(double k) {//key���� ��� ��ġ�ؾ��ϴ��� ã�´�
	node* p = root;
	int path;
	if (p == NULL)
		return NULL;
	while (1) {
		int j;
		for (j = 0; j < p->count%M; j++) {
			if (p->score[j] >= k)
			{
				path = j;
				break;
			}
		}
		if (j == p->count%M) path = p->count%M;
		if (p->count / M != 1)findcnt++;
		if (p->count / M == 1) break;
		p = p->branch[j];
	}
	if (p->score[path] == k&& p->count%M != path) {
		return (int*)p->branch[path + 1];
	}
	else return NULL;
}

void sequencialSearch()
{
	int path, j, k;
	node* p = root;
	findcnt = 0;

	if (p != NULL)
	{
		while (1)	// p�� leaf��� �϶����� Ž��
		{
			int j;
			for (j = 0; j < p->count%M; j++)//������ ���� ���� �� leaf���� ����
			{
				if (p->score[j] >= 0)
				{
					path = j;
					break;
				}
			}
			if (j == p->count%M)
				path = p->count%M;
			if (p->count / M == 1)//leaf����� while�� ���������鼭 p�� leaf��� ���� ���� ����
				break;
			p = p->branch[j];
		}
		cout << "enter the k-th leafnode (you can enter 0~" << leafnum << "): ";
		while (1) {
			cin >> k;
			if (k > leafnum) {
				cout << "please enter the right k-th num" << endl;
				continue;
			}
			break;
		}
		for (int h = 0; h < k; h++) {//k��° ��带 ã�ư���
			p = p->branch[0];
		}
		cout << p->count%M << endl;
		for (j = 0; j < p->count%M; j++)//k��° ��� �� �Ѹ���
		{
			if (j == p->count%M - 1) {
				printf("ID : %d", p->sid[j]);
				printf("  Score: %.1f", p->score[j]);
				printf("  BlockNum: %d\n", p->p[j]);

			}
			else {
				printf("ID : %d", p->sid[j]);
				printf("  Score: %.1f", p->score[j]);
				printf("  BlockNum: %d\n", p->p[j]);

			}
			//			str = (char*)*(int*)p->branch[j+1];
		}
		//p = p->branch[0];
	}
}

int* insertItem(double score, int sid, int blocknum)
{
	node* trace[TR];// ���Ե� ��θ� ������ ���ÿ뵵�� �迭
	int dir[TR];
	int i;
	int Block;
	double Key;
	int Sid;

	node* upRight, *p;
	int* insertFileLocation = (int*)malloc(sizeof(int));
	i = 0; // trace[]�� index
	p = root;//p�� ������ ���Ե� ��ġ�� Ž��

			 //*(int*)upRight = k;
	if (score <= 0)
	{
		printf("key error");
		return NULL;
	}

	if (root == NULL) {
		root = (node*)malloc(sizeof(node));
		root->branch[0] = NULL;
		root->score[0] = score;
		root->sid[0] = sid;
		root->p[0] = blocknum;
		root->count = M + 1;
		return insertFileLocation;
	}

	while (1) {
		int j;
		trace[i] = p;
		for (j = 0; j<p->count%M; j++)
			if (p->score[j] >= score)
			{
				dir[i] = j;
				break;
			}
		if (j == p->count%M) dir[i] = p->count%M;
		if (p->count / M == 1) break;
		p = p->branch[j];
		i++;
	}//�� �������� ������ p�� key���� ���Ե� ���

	 //���� ����
	Key = score;
	Sid = sid;
	Block = blocknum;
	while (i != -1) {
		int path = dir[i];
		p = trace[i];
		if (p->count%M != M - 1)//�����ص� overflow������ ����
		{
			int m;
			for (m = p->count%M; m > path; m--)//���Ե� ĭ���� ������ ��ĭ�� BACK
			{
				p->score[m] = p->score[m - 1];
				p->sid[m] = p->sid[m - 1];
				p->p[m] = p->p[m - 1];
				p->branch[m + 1] = p->branch[m];
			}
			p->score[path] = Key;//key ����
			p->sid[path] = Sid;
			p->p[path] = Block;
			p->branch[path + 1] = upRight; // branch ����
			p->count++;
			break;
		}
		else //���Խ� overflow �߻�
		{
			double nodeKey[M];
			int nodeSid[M];
			int nodep[M];
			node* nodeBranch[M + 1];
			node* newNode;
			int j, j2;
			newNode = (node*)malloc(sizeof(node));

			nodeBranch[0] = p->branch[0];
			for (j = 0, j2 = 0; j < M; j++, j2++)//�ӽ÷� ũ�� M+1�� ��忡 ������� ����
			{
				if (j == path)
				{
					nodeKey[j] = Key;
					nodeSid[j] = Sid;
					nodep[j] = Block;
					nodeBranch[j + 1] = upRight;
					j++;
					if (j >= M) break;
				}
				nodeKey[j] = p->score[j2];
				nodeSid[j] = p->sid[j2];
				nodep[j] = p->p[j2];
				nodeBranch[j + 1] = p->branch[j2 + 1];
			}
			for (j = 0; j < M / 2; j++)
			{
				p->score[j] = nodeKey[j];
				p->sid[j] = nodeSid[j];
				p->p[j] = nodep[j];
				p->branch[j + 1] = nodeBranch[j + 1];
			}
			newNode->branch[0] = nodeBranch[M / 2 + 1];
			for (j = 0; j < M / 2; j++)//��� key �������ʹ� ���λ��� ��忡 ����
			{
				newNode->score[j] = nodeKey[M / 2 + 1 + j];
				newNode->sid[j] = nodeSid[M / 2 + 1 + j];
				newNode->p[j] = nodep[M / 2 + 1 + j];
				newNode->branch[j + 1] = nodeBranch[M / 2 + 2 + j];
			}
			//���� p�� ��������̸� �ణ ����
			if (p->count / M == 1)
			{
				newNode->branch[0] = p->branch[0]; //sequencial pointer����
				p->branch[0] = newNode;
				p->score[M / 2] = nodeKey[M / 2]; // �ø� key���� ������忡�� �����.
				p->sid[M / 2] = nodeSid[M / 2];
				p->p[M / 2] = nodep[M / 2];
				p->branch[M / 2 + 1] = nodeBranch[M / 2 + 1];
				p->count = M + M / 2 + 1;
				newNode->count = M + M / 2;
				leafnum++;
			}
			else
			{
				p->count = newNode->count = M / 2;
				p->branch[0] = nodeBranch[0];
			}
			Key = nodeKey[M / 2];//��� Ű�� �ø��� key��
			Sid = nodeSid[M / 2];
			upRight = newNode;//���� ���� node�� �ø��� ���� ������ �ڽ�����
		}
		i--;
	}
	if (i == -1)//root���� overflow�� ������
	{
		root = (node*)malloc(sizeof(node));
		root->count = 1;
		root->branch[0] = trace[0];
		root->branch[1] = upRight;
		root->score[0] = Key;
		root->sid[0] = Sid;
		root->p[0] = Block;
	}
	return insertFileLocation;
}

node* bput(node* k)
{
	queue[rear] = k;
	rear = ++rear % MAX;
	return k;
}

node* get()
{
	node* i;
	if (front == rear)
	{
		printf("\n Queue underflow.");
		return NULL;
	}
	i = queue[front];
	front = ++front % MAX;
	return i;
}

int isEmpty()
{
	return (front == rear);
}

void indexNodePrint(node* t)
{
	if (t == NULL)
	{
		printf(" NULL�Դϴ�.");
	}
	else
	{
		ofstream os;
		os.open("Student_score.idx");
		bput(t);
		while (!isEmpty())
		{
			int i;
			t = get();
			if (t->count / M != 1) {
				printf("(");
				for (i = 0; i < t->count%M - 1; i++)
				{
					printf("%.1f, ", t->score[i]);
					os.write((char*)&t->score[i], sizeof(double));
					os.write((char*)&t->branch[i], sizeof(Node*));
				}
				printf("%.1f)\n ", t->score[t->count%M - 1]);
				os.write((char*)&t->score[t->count%M - 1], sizeof(double));
				os.write((char*)&t->branch[i], sizeof(Node*));
			}
			if (t->count / M != 1)
				for (i = 0; i <= t->count; i++)
					bput(t->branch[i]);
		}
		os.close();
	}
}

int main() {

	ifstream is;
	ofstream os;
	int target = 0;
	is.open("sampleData.csv");
	os.open("Students.DB");
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
	for (int i = 0; i<pow_2(MIN_ORDER); i++)	{
		dir->entry[i] = (leaf*)malloc(sizeof(leaf));
		dir->entry[i]->header = 2;
		dir->entry[i]->count = 0;
		dir->entry[i]->pRecord = (StElement**)malloc(sizeof(StElement*)*BUCKET_SIZE);
	}

	for(int i = 0; i < LIST_SIZE; i++){
		StElement* rec = &oData[i];
		insertRecord(rec, dir);
		/* printf("\n***  key : %d, name : %s �� record�� ����, pseudokey : %d ***\n",
			rec->getSID(), rec->getName(), makePseudokey(rec->getSID())); */
	}
	printDB(dir, os);
	os.close();
	printTable(oData);
	is.close();

	root = NULL; //root��� �ʱ�ȭ
	ifstream fi;
	int num;
	string bnum;
	int sid1;
	int aid1;
	float score1;
	char name1[20];
	fi.open("Students.DB");
	string bufstr;
	while (!fi.eof()) {
		fi >> num >> bnum;
		if (bnum != "P") {
			int h = atoi(bnum.c_str());
			for (int i = 0; i < h; i++) {
				getline(fi, bufstr, ',');
				strcpy(name1, bufstr.c_str());
				getline(fi, bufstr, ',');
				sid1 = atoi(bufstr.c_str());
				getline(fi, bufstr, ',');
				score1 = atof(bufstr.c_str());
				getline(fi, bufstr);
				aid1 = atoi(bufstr.c_str());
				insertItem(score1, sid1, num);
			}
		}
		else continue;
	}
	indexNodePrint(root);
	sequencialSearch();
	fi.close();

	system("pause");

	return 0;
}
