#include<iostream>
#include<cstdio>
using namespace std;
int main()
{
    short int Mod=1024;
    short int r0=1,r1=1,r7=2,n;
    cin>>n;
    if(n==0||n==1||n==2) {
        if(n==2) cout<<"r7=2"<<endl;
        else cout<<"r7=1"<<endl;
        return 0;
    }
    for(int i=3;i<=n;i++){
        short int r6=r7;
        r7=(r7+2*r0)%Mod;
        r0=r1;r1=r6;
    }
    cout<<"r7="<<r7<<endl;
}