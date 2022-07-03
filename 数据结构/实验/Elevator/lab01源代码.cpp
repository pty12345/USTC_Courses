#include<iostream>
#include<algorithm>
#include<ctime>
#include<string>
#include<cstdio>
#include<windows.h>
#include<stdlib.h>

//是否出电梯需要重新判断
//E2转E1出问题。本层人会消失

#define Lift_capacity 5//电梯容量

#define T_gap 50//单位时间间隔(ms) 
#define N 1000//电梯栈大小 
#define INF 1e9
using namespace std;
int Cur_time = 0;//系统时间 
int psg_sum = 0;
int nex_out_person = -1, nex_out_time = INF;
int Lift_people_num = 0;//电梯中现有人数
struct ST_E1 {
	int To_E3_time;//记录切换到E3的时间
	int To_E6_time;
	int Nex_parameter;//下一状态
	void reset() {
		To_E3_time = 10;
		To_E6_time = 10;//*需要改
		Nex_parameter = 0;
	}
}St_E1;

struct ST_E3 {
	int To_E9_time;//记录切换到E3的时间
	int To_E5_time;
	int To_E4_time;
	bool E9_IsPreset;//E9是否被预置
	bool E5_IsPreset;//E5是否被预置
	void reset() {
		To_E9_time = 300;
		To_E5_time = 1;//开始检测关门
		To_E4_time = 10;
		E9_IsPreset = true;
		E5_IsPreset = true;
	}
}St_E3;

struct ST_E4 {
	//int To_E3_time;//记录切换到E3的时间
	int Rest_out_time;//当前出去的人的剩余时间
	int Rest_in_time;//当前进来的人的剩余时间
	int Nex_parameter;//下一状态
	void reset() {
		//	To_E3_time = 20;
		//	To_E6_time = 20;
		Rest_out_time = 0;//初始为0，等待判断
		Rest_in_time = 0;
		Nex_parameter = 0;
	}
	void nex_in() {
		Rest_in_time = 10;//*
	}
	void nex_out() {
		Rest_out_time = 10;//*
	}
}St_E4;

struct ST_E5 {
	bool Pro_to_E6 = false;//是否正在关门
	int To_E6_time = 20;

	void Start_TFDoor() {
		Pro_to_E6 = true;
		To_E6_time = 20;
	}//开始关门倒计时
	void reset() {
		Pro_to_E6 = false;
		To_E6_time = 20;
	}//停止关门
}St_E5;

struct ST_E6 {
	int Nex_parameter = 0;
	int To_E7_time = 0;
	int To_E8_time = 0;
	void reset() {
		Nex_parameter = 0;
		To_E7_time = 15;
		To_E8_time = 15;
	}
}St_E6;

struct ST_E7 {
	int Nex_parameter = 0;
	int To_E2_time = 0;
	int Up_time = 0;
	void reset() {
		Nex_parameter = 0;
		Up_time = 10;
		To_E2_time = 5;
	}
}St_E7;

struct St_E8 {
	int Nex_parameter = 0;
	int To_E2_time = 0;
	int Down_time = 0;
	void reset() {
		Nex_parameter = 0;
		Down_time = 10;
		To_E2_time = 5;
	}
}St_E8;//*需要重新改回

struct Passenger {
	int InFloor = -1, OutFloor = -1;
	int GiveupTime = -1;//离开系统的时间
	int Nex_Intertime = -1;//以系统时间记录
	int psg_num = 0;//乘客编号，不存在则为0

	void crt_new_psg();
}n_psg;

Passenger Outing_person, Getting_in_person;

void Join_in(Passenger _psg);//进入的人都会按按钮
void Get_in_lift(Passenger _psg);//人员进入电梯;

enum Ele_state { Idle = -1, Go_Down = 0, Go_Up = 1 };//-1为停留，0为下降，1为上升

enum Ele_work_state { E1 = 1, E2, E3, E4, E5, E6, E7, E8, E9 };
struct Elevator {
	bool D1, D2, D3;
	int State, Floor;//电梯停留状态及位置;
	Passenger st[5][N];//每层目标楼层对应的电梯栈
	int st_top[5];
	bool CallCar[5] = { 0 };//有人按下目标楼层
	int Work_state;//电梯活动状态（E1 to E9)
	bool Work_state_change;//电梯活动状态是否刚改变

	void Work_ST_Change(int NEX_P) {
		Work_state = NEX_P;
		Work_state_change = true;
	}//更新下一状态
}Lift;

void State_reset();

typedef struct LNode {

	Passenger psg;
	struct LNode* next;
}LNode, * LineList;

struct Level {
	LineList L[5][2];//电梯等候队列链表，共5层，每层上、下各一个链表;0为下降
	bool Level_PushBotton[5][2] = { 0 };//楼层上下亮灯,1表示有人按
	int Level_que_len[5][2];//等候队列长度 
	bool Leave_judge[5][2] = { 0 };//判断该层楼的乘客是否会离开，1表示不能离开

	void ListInsert(LineList& L, Passenger _psg);
}L_que;

void St_E6_Reset()
{
	int Now_l = Lift.Floor;
	/*	Lift.CallCar[Now_l] = 0;
		if (Lift.State != Go_Down)
			L_que.Level_PushBotton[Now_l][1] = 0;
		else if (Lift.State != Go_Up)
			L_que.Level_PushBotton[Now_l][0] = 0;*/
	St_E6.reset();
}//E6初始化操作

void Level::ListInsert(LineList& L, Passenger _psg)
{
	++Level_que_len[_psg.InFloor][_psg.InFloor < _psg.OutFloor ? 1 : 0];
	LineList p = L;
	while (p->next)  p = p->next;
	LineList s = (LineList)malloc(sizeof(LNode));
	s->next = NULL;
	s->psg = _psg; p->next = s;
	return;
}

void Deal_new_in()
{
	bool up = n_psg.InFloor < n_psg.OutFloor ? 1 : 0;
	if (n_psg.GiveupTime < nex_out_time && !L_que.Leave_judge[n_psg.InFloor][up]) //更新下一出队人员 
		nex_out_person = n_psg.psg_num, nex_out_time = n_psg.GiveupTime;
}

void Passenger::crt_new_psg()
{
	InFloor = rand() % 5;
	OutFloor = rand() % 5;
	while (OutFloor == InFloor)
	{
		InFloor = rand() % 5;
		OutFloor = rand() % 5;
	}
	GiveupTime = (rand() % 10) + 30;
	Nex_Intertime = (rand() % 20) + 5;
	GiveupTime += Cur_time;//转化为系统时间
	psg_num = psg_sum++;//利用编号生成名字 
	Nex_Intertime += Cur_time;//转化为系统时间
	Deal_new_in();
	Join_in(*this);
	return;
}

void State_reset()//重置电梯系统
{
	Lift.Work_ST_Change(E1);
	Outing_person.psg_num = -1;
	Getting_in_person.psg_num = -1;
	L_que.Leave_judge[1][0] = L_que.Leave_judge[1][1] = 1;//初始在一楼的人均不会离开
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 2; j++)
		{
			L_que.L[i][j] = (LineList)malloc(sizeof(LNode));
			L_que.L[i][j]->next = NULL;
			//cout<<L_que.L[i][j]->next<<endl;
		}
	Lift.State = Idle; Lift.Floor = 1;
	n_psg.crt_new_psg();//让第一个人进去 
	return;
}

void Join_in(Passenger _psg)//进入的人都会按按钮
{
	if (_psg.InFloor < _psg.OutFloor)
	{
		L_que.ListInsert(L_que.L[_psg.InFloor][1], _psg);//上升队列
		L_que.Level_PushBotton[_psg.InFloor][1] = 1;//按下按钮 
	}
	else
	{
		L_que.ListInsert(L_que.L[_psg.InFloor][0], _psg);//下降队列
		L_que.Level_PushBotton[_psg.InFloor][0] = 1;//按下按钮
	}
	return;
}

void Draw_sketch_map()//输出函数
{
	printf("CurTime=%d   电梯楼层：%d\n", Cur_time, Lift.Floor);

	for (int i = 4; i >= 0; i--)
	{
		printf("%d", L_que.Level_PushBotton[i][1]);
		printf(" \t %d_Up_Q:", i);
		LineList p = L_que.L[i][1]->next;
		while (p)
		{
			printf("%c%d ", (p->psg.psg_num % (26 * 9)) / 9 + 97, ((p->psg.psg_num % (26 * 9)) % 10) + 1);
			p = p->next;
		}
		printf("\n");

		printf("%d", L_que.Level_PushBotton[i][0]);
		printf("\t%d_Down_Q:", i);
		p = L_que.L[i][0]->next;
		while (p)
		{
			printf("%c%d ", (p->psg.psg_num % (26 * 9)) / 9 + 97, ((p->psg.psg_num % (26 * 9)) % 10) + 1);
			p = p->next;
		}
		printf("\n"); printf("\n");
	}
	char c = (nex_out_person % (26 * 9)) / 9 + 97;
	int d = ((nex_out_person % (26 * 9)) % 10) + 1;

	if (nex_out_person != -1) {
		printf("下一可能放弃等待乘客：%c%d，", c, d);
		printf("其放弃时间预计在%d后\n", nex_out_time - Cur_time);
	}
	else printf("下一可能放弃等待乘客：NULL\n");//*需要改

	printf("正在出电梯的乘客：");
	if (Outing_person.psg_num == -1) printf("NULL");
	else printf("%c%d ", (Outing_person.psg_num % (26 * 9)) / 9 + 97, ((Outing_person.psg_num % (26 * 9)) % 10) + 1);
	printf("\n");

	printf("正在进电梯的乘客：");
	if (Getting_in_person.psg_num == -1) printf("NULL");
	else printf("%c%d ", (Getting_in_person.psg_num % (26 * 9)) / 9 + 97, ((Getting_in_person.psg_num % (26 * 9)) % 10) + 1);
	printf("\n\n电梯内乘客：");
	if (!Lift_people_num) printf("NULL");
	else 
		for (int i = 4; i >= 0; i--) {
			for(int j=1;j<=Lift.st_top[i];j++)
				printf("%c%d ", (Lift.st[i][j].psg_num % (26 * 9)) / 9 + 97, ((Lift.st[i][j].psg_num % (26 * 9)) % 10) + 1);
		}
		
	printf("\n\n");
	printf("电梯当前状态：%d\n\n", Lift.Work_state);

	printf("电梯内按钮状态：\n");
	for (int j = 4; j >= 0; j--)
	{
		if (Lift.CallCar[j])
			printf("1 ");
		else printf("0 ");
	}

	return;
}

void Delete_nex(LNode* p)
{
	LNode* q = (LNode*)malloc(sizeof(LNode));
	q = p->next;
	p->next = q->next; std::free(q);
	q = NULL;
	return;
}

void rst_nex_out_person()
{
	nex_out_time = INF;
	for (int i = 4; i >= 0; i--)
	{
		for (int j = 1; j >= 0; j--)
		{
			if (L_que.Leave_judge[i][j]) continue;//只在可能离去的人中选择下一个出去的人
			LineList p = L_que.L[i][j]->next;
			while (p)
			{
				if (p->psg.GiveupTime < nex_out_time) //更新下一出队人员 
					nex_out_person = p->psg.psg_num, nex_out_time = p->psg.GiveupTime;
				p = p->next;
			}
		}
	}
	if (nex_out_time == INF) nex_out_person = -1;//说明没人进出
}

void Reset_queue()
{
	if (nex_out_time > Cur_time) return;
	for (int i = 4; i >= 0; i--)
	{
		for (int j = 1; j >= 0; j--)
		{
			if (L_que.Leave_judge[i][j]) continue;
			LineList p = L_que.L[i][j];
			while (p->next)
			{
				if (p->next->psg.GiveupTime <= Cur_time) {
					L_que.Level_que_len[i][j]--;
					if (!L_que.Level_que_len[i][j])
						L_que.Level_PushBotton[i][j] = 0;//最后一个离开，灭灯
					Delete_nex(p);
				}
					
				else p = p->next;
			}
		}
	}
	rst_nex_out_person();
}

void Rst_state_parameter()//电梯状态改变时，重置状态参数
{
	switch (Lift.Work_state) {
	case E1:St_E1.reset(); break;
	case E3:St_E3.reset(); break;
	case E4:St_E4.reset(); break;
	case E6:St_E6_Reset(); break;//E6的更新不一样
	case E7: {St_E7.reset(), ++Lift.Floor; }; break; //cout << "-1"; Sleep(5000);
	case E8: {St_E8.reset(), --Lift.Floor; }; break;
		//case E9:St_E9.reset();
	default:break;
	}

	Lift.Work_state_change = false;
}

void E6_Controler()
{
	int nex_min_floor = -1;
	for (int j = 0; j <= 4 && nex_min_floor == -1; j++)
	{
		if (j == Lift.Floor) continue;
		if (Lift.CallCar[j] || ((L_que.Level_PushBotton[j][0] || L_que.Level_PushBotton[j][1]) && Lift_people_num < Lift_capacity))
			nex_min_floor = j;
	}
	if (nex_min_floor == -1) nex_min_floor = 1;

	if (Lift.Floor > nex_min_floor) Lift.State = Go_Down;
	if (Lift.Floor < nex_min_floor) Lift.State = Go_Up;
	return;
}

void Deal_with_ele_state()
{
	if (Lift.Work_state_change) Rst_state_parameter();

	int switch_flag = false;
	switch (Lift.Work_state) {
	case E1:
	{
		if (switch_flag) break;
		switch_flag = true;

		int Now_l = Lift.Floor;
		L_que.Leave_judge[Now_l][0] = 1;
		L_que.Leave_judge[Now_l][1] = 1;//本层人都不会离开

		if (!St_E1.Nex_parameter)//确定下一状态
		{
			if (L_que.Level_PushBotton[Now_l][0] || L_que.Level_PushBotton[Now_l][1] || Lift.CallCar[Now_l])
				St_E1.Nex_parameter = E3;
			else {
				int j;
				for (j = 0; j <= 5; j++)
				{
					if (j == 5) break;
					if (L_que.Level_PushBotton[j][0] || L_que.Level_PushBotton[j][1] || Lift.CallCar[j])
						break;
				}
				if (j == 5) { Lift.Work_ST_Change(E1); break; }//没有符合对象？则继续保持E1状态
				else {
					//if (j < Lift.Floor) Lift.Work_state = Go_Down;
					//if (j > Lift.Floor) Lift.Work_state = Go_Up;
					St_E1.Nex_parameter = E6;//确定下一状态
				}
			}
		}
		else {
			if (St_E1.Nex_parameter == E3)
			{
				if (St_E1.To_E3_time)//说明未切换至E3
				{
					St_E1.To_E3_time--;
					break;
				}
				Lift.Work_ST_Change(E3);
				break;
			}
			else {
				if (St_E1.To_E6_time)//说明未切换至E6
				{
					St_E1.To_E6_time--;
					break;
				}
				Lift.Work_ST_Change(E6);
				break;
			}
		}
	}; break;

	case E2:
	{
		if (switch_flag) break;
		switch_flag = true;

		int Now_l = Lift.Floor, j;
		for (j = 0; j <= 5; j++)
		{
			if (j == Now_l || j == 5) continue;
			if (Lift.CallCar[j] || L_que.Level_PushBotton[j][0] || L_que.Level_PushBotton[j][1])
				break;
		}
		if (Lift.CallCar[Now_l] || L_que.Level_PushBotton[Now_l][0] || L_que.Level_PushBotton[Now_l][1] || j == 5)
		{
			Lift.State = Idle;
			Lift.Work_ST_Change(E1);
			break;
		}//说明电梯应该在这层停候
		else {
			if (Lift.State == Go_Up) {
				Lift.State = Go_Down; Lift.Work_ST_Change(E8);
			}
			else {
				Lift.State = Go_Up; Lift.Work_ST_Change(E7);
			}
		}//更改行进方向
	}; break;
	case E3:
	{
		if (switch_flag) break;
		switch_flag = true;

		Lift.D1 = 1; Lift.D2 = 1;
		if (St_E3.To_E4_time)
		{
			St_E3.To_E4_time--;
			break;
		}
		else {
			Lift.Work_ST_Change(E4);
			break;
		}
	}; break;
	case E4:
	{
		if (switch_flag) break;
		switch_flag = true;

		int Now_l = Lift.Floor;
		L_que.Leave_judge[Now_l][0] = 1;
		L_que.Leave_judge[Now_l][1] = 1;//本层人都不会离开

		if (St_E4.Rest_in_time || St_E4.Rest_out_time) {
			if (St_E4.Rest_in_time) St_E4.Rest_in_time--;
			else St_E4.Rest_out_time--;
			break;
		}//正在进来或出来
		if (!St_E4.Rest_in_time) Getting_in_person.psg_num = -1;
		if (!St_E4.Rest_out_time) Outing_person.psg_num = -1;

		if (Lift.st_top[Now_l] && !St_E4.Rest_out_time) {
			Outing_person = Lift.st[Now_l][Lift.st_top[Now_l]];
			Lift.st_top[Now_l]--;
			if (!Lift.st_top[Now_l]) Lift.CallCar[Now_l] = 0;
			Lift_people_num--;
			St_E4.nex_out();
			Lift.D1 = 1; Lift.D3 = 0;//有人进出
			break;
		}//有人要出去并且没人正在出去
		if ((L_que.Level_que_len[Now_l][1] || L_que.Level_que_len[Now_l][0]) && !St_E4.Rest_out_time && Lift_people_num<Lift_capacity ) {
			if (L_que.Level_que_len[Now_l][1]) {
				Lift_people_num++;
				LNode* p = (LNode*)malloc(sizeof(LNode));
				p = L_que.L[Now_l][1]->next;
				Getting_in_person = p->psg;
				L_que.L[Now_l][1]->next = p->next;
				L_que.Level_que_len[Now_l][1]--;
				if (!L_que.Level_que_len[Now_l][1]) //按钮灭灯
					L_que.Level_PushBotton[Now_l][1] = 0;
				Get_in_lift(p->psg);
				std::free(p);
			}
			else {
				Lift_people_num++;
				LNode* p = (LNode*)malloc(sizeof(LNode));
				p = L_que.L[Now_l][0]->next;
				Getting_in_person = p->psg;
				L_que.L[Now_l][0]->next = p->next;
				L_que.Level_que_len[Now_l][0]--;
				if (!L_que.Level_que_len[Now_l][0]) //按钮灭灯
					L_que.Level_PushBotton[Now_l][0] = 0;
				Get_in_lift(p->psg);
				std::free(p);
			}
			Lift.D1 = 1; Lift.D3 = 0;//有人进出
			if (St_E5.Pro_to_E6) St_E5.reset();//有人出入，重置关门计数
			St_E4.nex_in();
			break;
		}//有人要进来并且没人正在出去、没人正在进来

		Lift.D1 = 0; Lift.D3 = 1;//此时没人进出，更改状态

		if (St_E3.E5_IsPreset) break;//说明还未检测关门
		else if (!Lift.D1) {
			Lift.D3 = 0;
			if (St_E5.Pro_to_E6 && !St_E5.To_E6_time) {
				St_E3.E5_IsPreset = false;//关门检测停止
				Lift.Work_ST_Change(E6);
				break;
			}//说明关门时间已到

			if (!St_E5.Pro_to_E6) St_E5.Start_TFDoor();
			else St_E5.To_E6_time--;//推动关门进程
		}//说明检测关门时没人进出,启动关门程序

		else
			St_E3.To_E5_time = 10;//更新关门检测
	}; break;
	case E6:
	{
		int Now_l = Lift.Floor;
		L_que.Leave_judge[Now_l][0] = 0;
		L_que.Leave_judge[Now_l][1] = 0;//本层人都会离开
		if (switch_flag) break;
		switch_flag = true;
		if (!St_E6.Nex_parameter)
		{
			E6_Controler();
			if (Lift.State == Idle)
			{
				Lift.Work_ST_Change(E1);
				break;
			}

			if (!Lift.D2) St_E3.E9_IsPreset = false;//停止计E9

			St_E6.Nex_parameter = (Lift.State == Go_Up ? E7 : E8);
		}
		else {
			if (St_E6.Nex_parameter == E7) {
				//keep working
				if (St_E6.To_E7_time) St_E6.To_E7_time--;
				else {
					Lift.Work_ST_Change(E7);
					break;
				}
			}
			else {
				if (St_E6.To_E8_time) St_E6.To_E8_time--;
				else {
					Lift.Work_ST_Change(E8);
					break;
				}
			}
		}
	}; break;
	case E7:
	{
		if (switch_flag) break;
		switch_flag = true;

		if (St_E7.Up_time) { St_E7.Up_time--; break; }
		int Now_l = Lift.Floor;
		if (!St_E7.Nex_parameter) {
			bool To_E2_flag = false;//判断是否会转到E2

			if (Lift.CallCar[Now_l] || (L_que.Level_PushBotton[Now_l][1] && Lift_people_num<Lift_capacity))
				To_E2_flag = true;
			else {
				bool flag = false;
				for (int j = Now_l + 1; j <= 4; j++)
					if (Lift.CallCar[j] || L_que.Level_PushBotton[j][0] || L_que.Level_PushBotton[j][1])
						flag = true;
				if ((Now_l == 1 || L_que.Level_PushBotton[Now_l][0]) && !flag)//说明准备转E2
					To_E2_flag = true;
			}

			if (To_E2_flag) St_E7.Nex_parameter = E2;
			else Lift.Work_ST_Change(E7);//否则重置E7
		}//判断下一状态

		else {
			if (St_E7.To_E2_time) St_E7.To_E2_time--;
			else Lift.Work_ST_Change(E2);
		}//否则说明下一状态必为E2
	}; break;
	case E8:
	{
		if (switch_flag) break;
		switch_flag = true;

		if (St_E8.Down_time) { St_E8.Down_time--; break; }
		int Now_l = Lift.Floor;
		if (!St_E8.Nex_parameter) {
			bool To_E2_flag = false;//判断是否会转到E2

			if (Lift.CallCar[Now_l] || (L_que.Level_PushBotton[Now_l][0] && Lift_people_num < Lift_capacity))
				To_E2_flag = true;
			else {
				bool flag = false;
				for (int j = 0; j < Now_l; j++)
					if (Lift.CallCar[j] || L_que.Level_PushBotton[j][0] || L_que.Level_PushBotton[j][1])
						flag = true;
				if ((Now_l == 1 || L_que.Level_PushBotton[Now_l][1]) && !flag)//说明准备转E2
					To_E2_flag = true;
			}

			if (To_E2_flag) St_E8.Nex_parameter = E2;
			else Lift.Work_ST_Change(E8);//否则重置E8
		}//判断下一状态

		else {
			if (St_E8.To_E2_time) St_E8.To_E2_time--;
			else Lift.Work_ST_Change(E2);
		}//否则说明下一状态必为E2
	}; break;
	case E9:
	{
		if (switch_flag) break;
		switch_flag = true;
	}; break;

	default:break;
	}
	if (St_E3.E9_IsPreset) St_E3.To_E9_time--;
	else Lift.D2 = 0;

	if (St_E3.E5_IsPreset) St_E3.To_E5_time--;
	else if (Lift.D1) St_E3.To_E5_time = 5, St_E3.E5_IsPreset = true;//*有人出入，重置关门检测

	if (!St_E3.To_E5_time) St_E3.E5_IsPreset = false;
	//
	return;
}
void Acc_time_process(int Clock)
{
	if (Clock == n_psg.Nex_Intertime) n_psg.crt_new_psg();//新加入人员
	rst_nex_out_person();//*每轮检查一遍下个离开人
	Reset_queue();
	Deal_with_ele_state();

	//wait to write
	return;
}//*需要改回
void Get_in_lift(Passenger _psg)//人员进入电梯
{
	Lift.st[_psg.OutFloor][++Lift.st_top[_psg.OutFloor]] = _psg;
	Lift.CallCar[_psg.OutFloor] |= 1;
}
int main()
{
	srand(time(0));
	State_reset();
	Cur_time = 0;//设置准备时间 
	while (Cur_time)
	{
		Cur_time--;
		printf("电梯将在%d后开始工作\n", Cur_time + 1);
		printf("CurTime=0   电梯楼层：1\n");
		Sleep(T_gap);
		system("Cls");
	}
	while (1)
	{
		++Cur_time;
		Acc_time_process(Cur_time);
		Draw_sketch_map();
		Sleep(T_gap);
		system("Cls");
	}
	return 0;
}