#include<iostream>
#include<cstdio>
int main()
{
    short int r0,r1;
    scanf("%hd%hd",&r0,&r1);
    bool flag=r0<0? 1:0;
    if(r0<0) r0=-r0;
    short int _r1=r1;
    while(--r0) r1+=_r1;
    r1 = (flag) ? -r1:r1; 
    printf("%hd",r1);
    return 0;
}