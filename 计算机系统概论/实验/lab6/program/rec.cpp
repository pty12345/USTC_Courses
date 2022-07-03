#include<iostream>
#include<cstdio>
using namespace std;
void count(short int &r0,short int &r1){
    if(!r1) return;
    r0+=1;r1-=1;
    count(r0,r1);
    return;
}//Recursive to solve
short int r0=0,r1;
int main()
{
    cin>>r1;
    count(r0,r1);
    cout<<"r0="<<r0<<" r1="<<r1<<endl;
    return 0;
}