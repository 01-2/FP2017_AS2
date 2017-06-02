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
#define MM 64 // B+트리의 차수
#define M (MM*2+1) // 홀수 차수,key개수 128개로 만듬
#define MAX 500 // 큐의 크기
#define TR 500 // 스택의 크기

typedef struct Node {
	int count;
	int sid[M - 1];//학생id 중복경우에 누구 것인지 확인하기 위함
	double score[M - 1];//score
	int p[M]; //리프노드면 블록위치 저장
	struct Node* branch[M];//주소
}node;

node* root;//루트 node
int front = 0, rear = 0; //큐를 구성하기 위함
node* queue[MAX];
int findcnt;
int leafnum = 0;//leaf노드 개수구하기 위함

int* search(double k); //key 탐색 함수
int* insertItem(double k, int sid, int blocknum);
node* bput(node* k);//큐 입력 함수
node* get();
int isEmpty(); //큐 공백 체크
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
// 제곱을 해주는 함수
int pow_2(int p){
	int i;
	int x = 1;
	for (i = 0; i<p; i++)
		x *= 2;
	return x;
}
// pseudokey를 만들어서 return 한다.
// KEY 길이만큼 제곱 후 mod 연산을 통해 하위 KEY_LENGTH 키만큼 잘라낸다
int makePseudokey(int key){
	return key % pow_2(KEY_LENGTH);
}
//  상위 n bit를 돌려준다.
int nreturn(int k, int n){
	return k / pow_2(KEY_LENGTH - n);
}
typedef struct Leaf
{
	int header;
	int count;  // 저장된 record의 갯수
	StElement** pRecord; // 저장된 record의 주소를 가리키는 pointer의 배열
} leaf;
typedef struct Directory
{
	int header;
	int divCount;  // directory의 header와 같은 header를 가진 leaf pair의 갯수
	leaf** entry;
} directory;
StElement* retrieval(int key, directory* dir)
{
	int i;
	int pseudokey = makePseudokey(key);    // pseudokey 생성
	int index = nreturn(pseudokey, dir->header);  // entry index
	leaf* bucket = dir->entry[index];
	for (i = 0; i<bucket->count; i++)     // 해당 leaf에서 찾고자하는 key를 찾는다.
	{
		StElement* ptr = dir->entry[index]->pRecord[i];
		if (ptr->getSID() == key)
			return ptr;        // 찾으면 record의 주소를 리턴
	}
	return NULL;         // 없으면 NULL을 리턴
}

int insertRecord(StElement* rec, directory* dir){
	int i, ind;
	int key = rec->getSID();        // 삽입되는 record의 key
	int pseudokey = makePseudokey(key);    // pseudokey
	int index = nreturn(pseudokey, dir->header); // entry의 번호를 찾는다.
	leaf* bucket = dir->entry[index];    // 삽입하고자 하는 leaf

	// bucket을 확인해서 삽입하고자 하는 키가 이미 존재하는지 확인
	for (i = 0; i<bucket->count; i++){
		if (bucket->pRecord[i]->getSID() == key){
			printf("\n\nThe key is already exist!!!\n");
			return 0;  // 비정상 종료
		}
	}

	// bucket에 저장할 공간이 남아있다면 빈칸에 저장한다.
	if (bucket->count < BUCKET_SIZE){
		bucket->pRecord[bucket->count] = rec;
		bucket->count++;
		return 1;   // 정상종료
	}

	// bucket이 다 찼다면 overflow 처리
	while (1){
		int n;
		leaf* newBucket;  // 새로 만들 bucket;
						  //////  d < t+1인경우 => 먼저 디렉토리를 두배 늘린다
		if (dir->header < bucket->header + 1){
			int numEntry;    // 새로 만들어질 entry의 수
			leaf** newEntry;
			dir->header++;
			numEntry = pow_2(dir->header);
			newEntry = (leaf**)malloc(sizeof(leaf*)*numEntry);
			for (i = 0; i<numEntry / 2; i++)  // entry를 늘어난 만큼 분배
				newEntry[i * 2] = newEntry[i * 2 + 1] = dir->entry[i];
			dir->divCount = 0;
			free(dir->entry);
			dir->entry = newEntry;
		}
		// overflow가 생긴 leaf를 split한다.
		// 새로운 bucket 생성
		newBucket = (leaf*)malloc(sizeof(leaf));
		newBucket->header = bucket->header;
		newBucket->count = 0;
		newBucket->pRecord = (StElement**)malloc(sizeof(StElement*) * BUCKET_SIZE);

		// bucket내의 record 분배
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

		// entry -> leaf 로의 pointer 조절
		n = pow_2(dir->header - bucket->header + 1);      // 나누어질 entry 수
		ind = nreturn(makePseudokey(key), bucket->header - 1) * n;   // 나누어질 entry중 첫번째 entry의 index

		for (i = 0; i<n / 2; i++, ind++)        // 두개의 bucket으로 나눔
			dir->entry[ind] = bucket;
		for (i = 0; i<n / 2; i++, ind++)
			dir->entry[ind] = newBucket;

		// 다시 삽입될 노드를 저장한다.
		index = nreturn(pseudokey, dir->header);
		bucket = dir->entry[index];
		if (bucket->count < BUCKET_SIZE) // 저장할 곳의 bucket이 full이 아니면 저장
		{
			bucket->pRecord[bucket->count] = rec;
			bucket->count++;
			return 1;   // 정상종료
		}
		// 저장할 곳이 full이 아니면 2의 처음으로 돌아가서 다시 directory를 늘린다.
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

int* search(double k) {//key값이 어디에 위치해야하는지 찾는다
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
		while (1)	// p가 leaf노드 일때까지 탐색
		{
			int j;
			for (j = 0; j < p->count%M; j++)//무조건 왼쪽 제일 밑 leaf노드로 가기
			{
				if (p->score[j] >= 0)
				{
					path = j;
					break;
				}
			}
			if (j == p->count%M)
				path = p->count%M;
			if (p->count / M == 1)//leaf노드라면 while문 빠져나가면서 p가 leaf노드 제일 왼쪽 도착
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
		for (int h = 0; h < k; h++) {//k번째 노드를 찾아가기
			p = p->branch[0];
		}
		cout << p->count%M << endl;
		for (j = 0; j < p->count%M; j++)//k번째 노드 값 뿌리기
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
	node* trace[TR];// 삽입될 경로를 저장할 스택용도의 배열
	int dir[TR];
	int i;
	int Block;
	double Key;
	int Sid;

	node* upRight, *p;
	int* insertFileLocation = (int*)malloc(sizeof(int));
	i = 0; // trace[]의 index
	p = root;//p를 가지고 삽입될 위치를 탐색

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
	}//이 루프에서 나오면 p는 key값이 삽입될 노드

	 //본격 삽입
	Key = score;
	Sid = sid;
	Block = blocknum;
	while (i != -1) {
		int path = dir[i];
		p = trace[i];
		if (p->count%M != M - 1)//삽입해도 overflow생기지 않음
		{
			int m;
			for (m = p->count%M; m > path; m--)//삽입될 칸부터 끝까지 한칸씩 BACK
			{
				p->score[m] = p->score[m - 1];
				p->sid[m] = p->sid[m - 1];
				p->p[m] = p->p[m - 1];
				p->branch[m + 1] = p->branch[m];
			}
			p->score[path] = Key;//key 삽입
			p->sid[path] = Sid;
			p->p[path] = Block;
			p->branch[path + 1] = upRight; // branch 관리
			p->count++;
			break;
		}
		else //삽입시 overflow 발생
		{
			double nodeKey[M];
			int nodeSid[M];
			int nodep[M];
			node* nodeBranch[M + 1];
			node* newNode;
			int j, j2;
			newNode = (node*)malloc(sizeof(node));

			nodeBranch[0] = p->branch[0];
			for (j = 0, j2 = 0; j < M; j++, j2++)//임시로 크기 M+1인 노드에 순서대로 복사
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
			for (j = 0; j < M / 2; j++)//가운데 key 다음부터는 새로생긴 노드에 복사
			{
				newNode->score[j] = nodeKey[M / 2 + 1 + j];
				newNode->sid[j] = nodeSid[M / 2 + 1 + j];
				newNode->p[j] = nodep[M / 2 + 1 + j];
				newNode->branch[j + 1] = nodeBranch[M / 2 + 2 + j];
			}
			//만약 p가 리프노드이면 약간 수정
			if (p->count / M == 1)
			{
				newNode->branch[0] = p->branch[0]; //sequencial pointer관리
				p->branch[0] = newNode;
				p->score[M / 2] = nodeKey[M / 2]; // 올릴 key값을 리프노드에도 남긴다.
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
			Key = nodeKey[M / 2];//가운데 키를 올리는 key로
			Sid = nodeSid[M / 2];
			upRight = newNode;//새로 만든 node를 올리는 값의 오른쪽 자식으로
		}
		i--;
	}
	if (i == -1)//root에서 overflow가 생긴경우
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
		printf(" NULL입니다.");
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
		/* printf("\n***  key : %d, name : %s 인 record를 저장, pseudokey : %d ***\n",
			rec->getSID(), rec->getName(), makePseudokey(rec->getSID())); */
	}
	printDB(dir, os);
	os.close();
	printTable(oData);
	is.close();

	root = NULL; //root노드 초기화
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
