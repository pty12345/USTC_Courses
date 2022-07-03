#include<iostream>
#include<cstdio>
using namespace std;
short int r0,r1;
void Solve_Mod(short int &r1)
{
    if(r1<8) return;
    short int r0=r1>>3;
    short int r2=r1 & 0x7;
    r1=r0+r2;
    Solve_Mod(r1);
}
int main()
{
    cin>>r0;
    r1=r0;
    Solve_Mod(r1);
    cout<<"r1="<<r1<<endl;
    return 0;
}