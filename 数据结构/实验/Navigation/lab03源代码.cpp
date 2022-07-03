#include<iostream>
#include<fstream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<algorithm>
#include<queue>
#include <cmath>

#define V_CODE_LEN 4//每个点信息编码长度为4byte
#define E_CODE_LEN 4//每个边长度信息编码长度为4byte

#define MAXEDGE 10000009
#define MAXVEX 10000009
#define INF 1000000009

int head[MAXVEX], tot = 0;
int vex_num = 0;
using namespace std;

struct Edge {
	int v, w, next;
}edge[MAXEDGE * 2];


void add(int u, int v, int w) {
	tot++;
	edge[tot].v = v;
	edge[tot].w = w;
	edge[tot].next = head[u];
	head[u] = tot;
}


void Bit_compress(unsigned char *CH, int u,int Code_len)
{
	int len = 0;
	int _u = u;
	unsigned char _CH[8 * 5] = { '\0' };
	while (u) {
		if (u & 1) _CH[len++] = '0';
		else _CH[len++] = '1';
		u >>= 1;
	}//取反码
	while(len < 8*Code_len) _CH[len++] = '1';//补全位数
	//printf_s("%d %s\n", _u, _CH);
	for (int i = 0; i < Code_len; i++) {
		int k = 7; CH[i] = 0;
		for (int j = (8 * Code_len - 1) - 8 * i; j >= (8 * Code_len - 1) - 8 * (i + 1); j--) {
			CH[i] += (int)(_CH[j] - 48) * (1 << k);
			k--;
		}
		//cout << (int)CH[i] << " ";
	}
	//cout << endl;
	CH[Code_len] = '\0';
	return ;
}

void DOC_deal_line(FILE* fout,int u,int v,int w)
{
	unsigned char CH[5] = {'\0'};
	Bit_compress(CH, u, V_CODE_LEN);
	for (int i = 0; i < V_CODE_LEN; i++) fprintf_s(fout, "%c", CH[i]);
	//fprintf_s(fout, " ");

	Bit_compress(CH, v, V_CODE_LEN);
	for (int i = 0; i < V_CODE_LEN; i++) fprintf_s(fout, "%c", CH[i]);
	//fprintf_s(fout, " ");

	Bit_compress(CH, w, E_CODE_LEN);
	for (int i = 0; i < E_CODE_LEN; i++) fprintf_s(fout, "%c", CH[i]);
	//fprintf_s(fout, "\n");
	//fprintf(fout, "%s\n", CH);
}

void DOC_COMPRESS(const char* sIn, const char* sOut)
{
	FILE* fin, * fout;
	fopen_s(&fin, sIn, "rb");
	fopen_s(&fout, sOut, "wb");
	int u = 0, v = 0, w = 0;
	int ch_len = 0;
	char CH[10];
	bool flag = 0;
	while (!feof(fin)) {
		fread_s(&CH[ch_len++], sizeof(char), 1, 1, fin);
		if (CH[ch_len - 1] == 10 || CH[ch_len - 1] == 32) {
			if (CH[ch_len - 1] == 10) {
				if (flag == 0) break;
				CH[--ch_len] = '\0';
				w = atoi(CH);
				//fprintf_s(fout, "%d %d %d\n", u, v, w);
				DOC_deal_line(fout, u, v, w);
				ch_len = 0;
				flag = 0;
			}
			else {
				CH[--ch_len] = '\0';
				if (flag == 0) u = atoi(CH);
				else v = atoi(CH);
				flag++;
				ch_len = 0;
			}
		}

		if (feof(fin)) break;
	}
		

	fclose(fin);
	fclose(fout);
	return;
}

int Bit_depress(unsigned char* CH, int Byte_num)
{
	int k = 1 << ((Byte_num - 1) * 8), value = 0;
	for (int i = 0; i < Byte_num; i++) {
		value += ((1<<8)-1-(int)CH[i]) * k;
		k >>= 8;
	}//反码
	return value;
}


void DECOMPRESS(const char* sIn, const char* sOut)
{
	FILE* fin, * fout;
	fopen_s(&fin, sIn, "rb");
	fopen_s(&fout, sOut, "wb");

	unsigned char CH[5] = {'\0'},Bin_char = 0;
	int u, v, w;

	while (!feof(fin)) {
		for (int i = 0; i < V_CODE_LEN; i++) fread_s( &CH[i],sizeof(char),1,1,fin);
		if (feof(fin)) break;
		CH[V_CODE_LEN] = '\0';
		u = Bit_depress(CH, V_CODE_LEN);//u编号
		fprintf_s(fout, "%d ", u);
		//fscanf_s(fin,"%c",&Bin_char);//读入无效字符
		vex_num = (vex_num > u) ? vex_num : u;//找出最大编号的点

		for (int i = 0; i < V_CODE_LEN; i++) fread_s(&CH[i], sizeof(char), 1, 1, fin);
		CH[V_CODE_LEN] = '\0';
		v = Bit_depress(CH, V_CODE_LEN);
		vex_num = (vex_num > u) ? vex_num : u;//找出最大编号的点

		fprintf_s(fout, "%d ", v);
		//fscanf_s(fin, "%c", &Bin_char);//读入无效字符

		for (int i = 0; i < E_CODE_LEN; i++) fread_s(&CH[i], sizeof(char), 1, 1, fin);
		CH[E_CODE_LEN] = '\0';
		w = Bit_depress(CH, E_CODE_LEN);
		fprintf_s(fout, "%d\n", w);
		//fscanf_s(fin, "%c", &Bin_char);//读入无效字符

		add(u, v, w);
		add(v, u, w);
	}

	fclose(fin);
	fclose(fout);
	return;
}

void Deal_with_ask();

int main()
{
	const char* sIn = "C:\\Users\\pty\\Desktop\\a1.txt";
	const char* sOut1 = "C:\\Users\\pty\\Desktop\\a2.TXT";
	const char* sOut2 = "C:\\Users\\pty\\Desktop\\b.TXT";
	DOC_COMPRESS(sIn, sOut1);
	DECOMPRESS(sOut1, sOut2);
	//Try(sIn,sOut2);
	printf("\nSuccessfully build the map!\n");
	Deal_with_ask();//处理询问
	return 0;
}

struct VNode {
	int dist = 0, last_node = 0;//到起点的最短距离距离和最短路径前驱节点的编号
	bool vis = false;//是否被标记，Dijkstra算法要求
}node[MAXVEX];

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
	q.push(p);
	while (!q.empty()) {
		Vitality_node t = q.top();
		q.pop();
		int u = t.pos;
		//cout << node[u].dist<<" ";
		if (node[u].vis) continue;
		if (node[u].dist == INF || u == T) break;
		node[u].vis = true;//u是目前到达的最短处，从这个点向外松弛
		for (int i = head[u]; i; i = edge[i].next) {//枚举这个点的所有邻边
			int now = edge[i].v;
			//cout << node[now].dist << " ";
			if ((node[now].dist > node[u].dist + edge[i].w) && !node[now].vis) {
				node[now].dist = node[u].dist + edge[i].w;
				node[now].last_node = u;//记录前驱节点
				p = { now, node[now].dist };
				q.push(p);//入队
			}
		}
	}

	if (node[T].dist == INF) return false;

	printf_s("The shortest path's length from Src to Dst is: %d\n", node[T].dist);

	int t = T;
	printf_s("The shortest path from Dst to Src is:\n");
	while (t != S) {
		printf_s("%d<-", t);
		t = node[t].last_node;
	}
	printf_s("%d\n", t);

	return true;
}

void Deal_with_ask()
{
	//while (1)
	//{
		int Src = 0, Dst = 0;
		printf_s("\nPlease enter the Src and the Dst you wanted to find：\n");
		scanf_s("%d%d", &Src, &Dst);

		bool FLAG = Dijkstra(Src, Dst);
		if (!FLAG) printf("There is no path from Src to Dst!\n");
	//}
	return;
}