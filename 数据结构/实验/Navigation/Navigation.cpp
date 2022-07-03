#include<iostream>
#include<fstream>
#include<cstdio>
#include<string.h>
#include<algorithm>
#include<queue>
#include<map>

#define INF 10000009

#define MAXVEX 1000009
#define MAXEDGE 10000009
#define MAX_CODE_LEN 16
#define MAX_LEN_COMPRESS 10000009//压缩文本最大长度限制
#define MAX_MESSAGE_LEN 20//输入最长连续信息，如节点编号、距离等

using namespace std;


int tot = 0, head[MAXVEX] = { 0 };//建图边的编号
int vex_num = 0;//解压缩时能够计算出节点个数
struct VNode {
	char V_name[MAX_MESSAGE_LEN] = {'\0'};//点标签
	int dist = 0, last_node = 0;//到起点的最短距离距离和最短路径前驱节点的编号
	bool vis = false;//是否被标记，Dijkstra算法要求
}node[MAXVEX];

struct Edge {
	int v, w, next;
}edge[MAXEDGE*2];



void add(int u, int v, int w) {
	tot++;
	edge[tot].v = v;
	edge[tot].w = w;
	edge[tot].next = head[u];
	head[u] = tot;
}


enum {Branch = 0,Leave= 1} Huffman_node_type;
typedef struct Huffman_node
{
	Huffman_node* lchild, * rchild;
	bool Tag;
	int key,serial_num;//serial_num记录其作为叶子节点所指向的Char_array序号
};
struct cmp
{
	bool operator() (const Huffman_node* a, const Huffman_node* b)
	{
		return a->key > b->key;
	}
};

int MAXNUM = 256;

typedef struct Char_array
{
	int Count_num = 0;//统计出现次数
	int ASCII_code;
	bool operator < (Char_array  c) const {
		return Count_num > c.Count_num;
	}//按照出现次数降序排序
	char s_code[MAX_CODE_LEN] = { '\0' };//压缩编码
};


void DFS_huffman_tree(Huffman_node* p, Char_array* CH, char* CH_array ,int dep)//DFS_huffman_tree
{
	if (p->Tag == Leave) {
		int num = p->serial_num;
		strcpy_s((CH + num)->s_code, CH_array);
		(CH + num)->s_code[dep+1] = '\0';
		return;
	}
	*(CH_array + dep) = '0';
	DFS_huffman_tree(p->lchild, CH, CH_array, dep + 1);
	*(CH_array + dep) = '1';
	DFS_huffman_tree(p->rchild, CH, CH_array, dep + 1);
	*(CH_array + dep) = '\0';
}//dep为编码长度

void Build_Huffman(Char_array* CH)//BuildHuffmanTree
{
	if ((CH+1)->Count_num == 0) return;//若只有一个结束字符，则无需编码
	priority_queue<Huffman_node *,vector<Huffman_node *>,cmp> Q1;
	int Need_to_code_num;//需要编码的个数
	Char_array* _CH = CH;
	for (Need_to_code_num = 0;; Need_to_code_num++,_CH++){
			if (_CH -> Count_num == 0) break;
			Huffman_node *s = new(Huffman_node);
			s->key = _CH->Count_num; 
			s->serial_num = Need_to_code_num ;s->Tag = Leave;
			Q1.push(s);
		}

	while (Q1.size() > 1)
	{
		Huffman_node *ch1 = Q1.top(); Q1.pop();
		Huffman_node *ch2 = Q1.top(); Q1.pop();
		Huffman_node* s = new(Huffman_node);
		s->key = ch1->key+ch2->key;
		s->serial_num = -1; s->Tag = Branch;
		s->lchild = ch1; s->rchild = ch2;
		Q1.push(s);
	}
	Huffman_node* root = Q1.top(); Q1.pop();
	char CH_array[MAX_CODE_LEN] = { 0 };
	DFS_huffman_tree(root, CH, CH_array, 0);
	return;
}

void DOC_COMPRESS(const char* sIn,const char* sOut)//文件压缩函数
{
	FILE *fin,*fout;
	fopen_s(&fin, sIn, "r");
	fopen_s(&fout, sOut, "w");
	
	Char_array CharSeq[256];

	for (int i = 0; i < MAXNUM; i++) {
		CharSeq[i].ASCII_code = i;
		CharSeq[i].s_code[0] = '\0';
		CharSeq[i].Count_num = 0;
	}
		

	char ch;
	while (!feof(fin)) {
		fscanf_s(fin, "%c", &ch);
		if (feof(fin)) break;
		++CharSeq[ch].Count_num;
	}
	++CharSeq[42].Count_num;//*作为结束字符插入，ASCII为42
	stable_sort(CharSeq, CharSeq + MAXNUM - 1);
	Build_Huffman(CharSeq);

	int k = 0;//k records the max_place of appeared char 
	while (k<MAXNUM &&  CharSeq[k].Count_num>0) k++;
	int CH_place[256] = { '\0' }, _k = k;
	while (_k!=-1)
	{
		CH_place[CharSeq[_k].ASCII_code] = _k;
		_k--;
	}

	queue<char> bit_queue;

	fprintf_s(fout, "%d\n", k);//输出文本译码字符数
	for (int i = 0; i < k; i++)
		fprintf_s(fout, "%c %s\n", CharSeq[i].ASCII_code, CharSeq[i].s_code);//输出解码钥匙

	rewind(fin);

	int CH_num = 0, len = 0;

	while (!feof(fin)) {
		fscanf_s(fin, "%c", &ch);
		if (feof(fin)) break;
		//printf("%c", ch);
		CH_num = CH_place[ch];
		len = strlen(CharSeq[CH_num].s_code);
		for (int i = 0; i < len; i++) 
			bit_queue.push(CharSeq[CH_num].s_code[i]);
		while (bit_queue.size() >= MAX_CODE_LEN) {
			int H_code = 0;
			for (int i = 3; i >= 0; i--) {
				H_code += ((int)bit_queue.front() - 48) << i;
				//cout << (int)bit_queue.front() - 48;
				bit_queue.pop();
			}
			fprintf_s(fout,"%X", H_code);//输出编码结果
		}
	}//编码文本文件

	CH_num = CH_place[42];
	len = strlen(CharSeq[CH_num].s_code);
	for (int i = 0; i < len; i++)
		bit_queue.push(CharSeq[CH_num].s_code[i]);
	while (bit_queue.size() % 8) bit_queue.push('1');//添加哨兵，并补全为4字节

	while (bit_queue.size() >= 4) {
		char H_code = 0;
		for (int i = 3; i >= 0; i--) {
			H_code += ((int)bit_queue.front() - 48) << i;
			bit_queue.pop();
		}
		fprintf_s(fout, "%X", H_code);//以16进制输出
	}
	
	fclose(fin);
	fclose(fout);
	return;
}

Huffman_node* Rebuild_tree_node[256];
//为树的建立提前开辟空间，防止局部变量的销毁
int Rebuild_tree_node_code = 0;
void Rebuild_huffman_tree(Huffman_node* R,char *s_code,char ch)
{
	Huffman_node* root = R;
	int len = strlen(s_code);
	//printf_s("%c %s\n", ch, s_code);
	for (int i = 0; i < len ; i++)
	{
		if (s_code[i] == '0') {
			if (!root->lchild) {
				Huffman_node* s = Rebuild_tree_node[Rebuild_tree_node_code++];
				//printf_s("%d %d\n", s, Rebuild_tree_node[Rebuild_tree_node_code - 1]);
				s->lchild = s->rchild = NULL;
				s->Tag = Branch; s->serial_num = '0';
				root->lchild = Rebuild_tree_node[Rebuild_tree_node_code-1];
			}
			root = root->lchild;
		}
		else {
			if (!root->rchild) {
				Huffman_node* s = Rebuild_tree_node[Rebuild_tree_node_code++];
				s->lchild = s->rchild = NULL;
				s->Tag = Branch; s->serial_num = '1';
				root->rchild = Rebuild_tree_node[Rebuild_tree_node_code - 1];
			}
			root = root->rchild;
		}

		if (i == len - 1) {
			root->Tag = Leave;
			root->key = (int)ch;
			//printf_s("%c ", ch);
		}
	}
	return;
	//Huffman_node中key记录对应ASCII码，serial_num记录每个点所表示的0/1
}

char Decode_bit_queue(queue <char> &bit_stack,Huffman_node * R)
{
	Huffman_node* root = R;
	while (root->Tag == Branch) {
		//if (bit_stack.empty()) return 0;//test
		char value = bit_stack.front();//wrong_place
		if (value == '0') root = root->lchild;
		else root = root->rchild;
		bit_stack.pop();
	}
	return (char) root->key;
}

int cnt = 0;


int count_dist(char* CH_que)//计算距离
{
	int len = strlen(CH_que),sum=0;
	int pow = 1;
	for (int i = len - 1; i >= 0; i--)
	{
		sum += pow * ((int)CH_que[i] - 48);
		pow *= 10;
	}
	return sum;
}

int e[3] = { 0 }, flag = 0;//每条边所含信息及压缩后顶点个数
int  Message_len = 0;//当前有效信息长度
char Message_que[MAX_MESSAGE_LEN] = { '\0' };//记录每次信息
std::map<string, int> VNode_construct;//构建节点名称与编号的映射
void Deal_with_ch(char Decode_ch)//进行解码并hash建图
{
	map<string, int>::iterator iter;
	if (Decode_ch == 32) {
		iter = VNode_construct.find(Message_que);

		if (iter != VNode_construct.end()) e[flag] = iter->second;//说明查找成功
		else {
			VNode_construct.insert(pair<string, int>(Message_que, ++vex_num));//否则给新顶点编号
			e[flag] = vex_num;
			strcpy_s(node[vex_num].V_name, Message_que);
		}
			
		for (int i = 0; i < Message_len; i++) Message_que[i] = '\0';
		Message_len = 0; flag++;
	}//说明是空格
	else {
		if (Decode_ch == 10) {
			if (!flag) return;//说明出现两个或多个回车，结束输入
			e[flag] = count_dist(Message_que);
			for (int i = 0; i < Message_len; i++) Message_que[i] = '\0';
			Message_len = 0;flag = 0;
			//cout << e[0] << " " << e[1] << " " << e[2] << endl;//test
			add(e[0], e[1], e[2]);
			add(e[1], e[0], e[2]);
		}//说明是换行
		else Message_que[Message_len++] = Decode_ch;//说明啥也不是
	}
}

void detective(Huffman_node* root)
{
	if (root->Tag == Leave) {
		//cout << (char)root->key<<" ";
		return;
	}
	if (root->lchild) detective(root->lchild);
	if (root->rchild) detective(root->rchild);
	return;
}

Huffman_node* DE_root = new (Huffman_node);
void DECOMPRESS(const char* sIn, const char* sOut)//Big Problem!
{
	FILE* fin, * fout;
	fopen_s(&fin, sIn, "r");
	fopen_s(&fout, sOut, "w");

	int k,len;
	char ash_bin='\0', ch='\0', s_code[20] = {'\0'};

	DE_root->Tag = Branch; DE_root->lchild = DE_root->rchild = NULL;
	DE_root->key = -1;

	fscanf_s(fin, "%d", &k); fscanf_s(fin, "%c", &ash_bin);
	for (int i = 1; i <= k; i++) {
		fscanf_s(fin, "%c", &ch); fscanf_s(fin, "%c",& ash_bin);
		len = 0;
		do {
			fscanf_s(fin, "%c", &s_code[len++]);
		} while (s_code[len-1]!=10);
		s_code[--len] = '\0';
		Rebuild_huffman_tree(DE_root, s_code, ch);
	}
	//detective(DE_root);//检测树的建立 test


	queue<char> bit_queue;
	//while (!bit_queue.empty()) bit_queue.pop();

	char _value = 0,Decode_ch=0;
	int value = 0;

	while (!feof(fin)) {
		//printf_s("%c ", _value);
		fscanf_s(fin, "%c", &_value);
		if (feof(fin)) break;
		if (_value < 48 || _value > 70) break;//超出编码范围
		if (_value > 64) value =_value - (64 - 9);
		else value =_value - 48;
		//cout << value << " ";
		for (int i = 3; i >= 0; i--) {
			bit_queue.push(((value >> i) % 2)+48);
			//cout << (int)bit_queue.back()-48;
		}
		//cout << endl;
			
		while (bit_queue.size() > 2*MAX_CODE_LEN) {
			Decode_ch = Decode_bit_queue(bit_queue, DE_root);
			if (Decode_ch == 42) break;
			Deal_with_ch(Decode_ch);
			//fprintf_s(fout,"%c",Decode_ch);
		}
	}//解码
	if (Decode_ch != 42) {
		while (bit_queue.size()) {
			Decode_ch = Decode_bit_queue(bit_queue, DE_root);
			if (Decode_ch == 42) break;//哨兵，终止输出
			Deal_with_ch(Decode_ch);
			//fprintf_s(fout, "%c", Decode_ch);
		}//解码
	}
	

	fclose(fin);
	fclose(fout);
	return;
}

void Deal_with_ask();

int main()
{
	for (int i = 0; i < 255; i++) {
		Rebuild_tree_node[i] = new Huffman_node;
		Rebuild_tree_node[i]->lchild = Rebuild_tree_node[i]->rchild = NULL;
	}
		
	const char* sIn = "C:\\Users\\pty\\Desktop\\a1.TXT";
	const char* sOut1 = "C:\\Users\\pty\\Desktop\\a2.TXT";
	const char* sOut2 = "C:\\Users\\pty\\Desktop\\b.TXT";
	DOC_COMPRESS(sIn, sOut1);
	DECOMPRESS(sOut1, sOut2);//解压并建好图
	printf("\nSuccessfully build the map!\n");
	Deal_with_ask();//处理询问
	
	return 0;
}

typedef struct Vitality_node {
	int pos;
	int dis;
	bool operator <(const Vitality_node& x)const {
		return x.dis < dis;
	}
};//定义临时结构体变量，便于排序

bool Dijkstra(int S, int T)
{

	priority_queue<Vitality_node>q;//记录节点编号
	//cout << vex_num << "*" << endl;;
	for (int i = 1; i <= vex_num; i++) {
		node[i].dist = INF; node[i].vis = false;
		//cout << head[i] << " ";
	}//初始化点信息
	node[S].dist = 0; 

	Vitality_node p = { S,0 };
	q.push( p );
	while (!q.empty()) {
		Vitality_node t = q.top();
		q.pop();
		int u = t.pos;
		//cout << node[u].dist<<" ";
		if (node[u].vis) continue;
		if(node[u].dist == INF || u==T) break;
		node[u].vis = true;//u是目前到达的最短处，从这个点向外松弛
		for (int i = head[u]; i; i = edge[i].next) {//枚举这个点的所有邻边
			int now = edge[i].v;
			//cout << node[now].dist << " ";
			if ((node[now].dist > node[u].dist + edge[i].w) && !node[now].vis) {
				node[now].dist = node[u].dist + edge[i].w;
				node[now].last_node = u;//记录前驱节点
				p = { now, node[now].dist };
				q.push( p );//入队
			}
		}
	}

	if (node[T].dist == INF) return false;
	
	printf_s("The shortest path's length from Src to Dst is: %d\n",node[T].dist);

	int t = T;
	printf_s("The shortest path from Dst to Src is:\n");
	while (t != S) {
		printf_s("%s->", node[t].V_name);
		t = node[t].last_node;
	}
	printf_s("%s\n", node[t].V_name);

	return true;
}

void Deal_with_ask()
{
	char s[MAX_CODE_LEN] = { '\0' }, t[MAX_CODE_LEN] = { '\0' };
	int s_len = 0, t_len = 0;
	int Src, Dst;
	char ch = 0;
	map<string, int>::iterator iter;
	while (1)
	{
		s_len = 0, t_len = 0;
		printf_s("\nPlease enter the Src and the Dst you wanted to find：\n");
		while (1) {
			scanf_s("%c", &ch, 1);if (ch == 32) break;
			s[s_len++] = ch;
		}
		while (1) {
			scanf_s("%c", &ch, 1); if (ch == 10) break;
			t[t_len++] = ch;
		}
		s[s_len] = t[t_len] = '\0';

		iter = VNode_construct.find(s);
		if (iter == VNode_construct.end()) {
			printf_s("The Src does't exist,please enter again.\n");
			continue;
		}
		else Src = iter->second;

		iter = VNode_construct.find(t);
		if (iter == VNode_construct.end()) {
			printf_s("The Dst does't exist,please enter again.\n");
			continue;
		}
		else Dst = iter->second;
		//cin >> Src >> Dst;
		//cout << Src << " " << Dst << endl;
		bool FLAG = Dijkstra(Src, Dst);
		if (!FLAG) printf("There is no path from Src to Dst!\n");
	}
	return;
}