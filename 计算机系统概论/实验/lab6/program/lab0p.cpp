#include<iostream>
using namespace std;
int main()
{
    short int r0,r1,r7,flag;
    cin>>r0>>r1;
    if(r0==0) {
        cout<<0;
        return 0;
    }
    r7=0;
    flag=(r0>=0)?0:1;
    if(flag) r0=-r0;
    while(r0){
        if(r0%2) r7+=r1;
        if(r0!=1) r1<<=1;
        r0>>=1;
    }
    if(flag) r7=-r7;
    cout<<r7;
    return 0;
}