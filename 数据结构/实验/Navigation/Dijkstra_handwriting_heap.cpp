#include<iostream>
#include<algorithm>
#include<cstdio>
#include<cstring>
using namespace std;
struct node{
    long long sum,num;
}h[1000000];
long long n,m,s,ui,vi,wi,nex[400060],to[400060],go[400060],fir[400060],tot,dis[100030],vis[200030],op;
void put(long long x,long long w);
void Dijkstra();
long long get();
int main()//有向图的Dijkstra，输出s为根的最短路树
{
    cin>>n>>m>>s;//输入点数、边数、起点
    for(long long i=1;i<=m;i++)
    {
        scanf("%lld%lld%lld",&ui,&vi,&wi);//输入有向边的起点、终点、权值
        nex[++tot]=fir[ui];fir[ui]=tot;to[tot]=vi;go[tot]=wi;
    }
    memset(dis,0x3f,sizeof(dis));
    dis[s]=0;
    put(s,dis[s]);
    Dijkstra();
    for(long long i=1;i<=n;i++)
    printf("%lld ",dis[i]);//依次输出各点到起点的最短距离
    return 0;
}
void Dijkstra()
{
    while(op)
    {
        long long u=get();
        while(vis[u]&&op)
        u=get();
        vis[u]=1;
        for(long long e=fir[u];e;e=nex[e])
        {
            long long v=to[e],w=go[e];	
            if(dis[v]>dis[u]+w&&!vis[v])
            {
                dis[v]=dis[u]+w;
                put(v,dis[v]);
            }
        }
    }
    return;
}
long long get()
{
    long long res=h[1].num;
    h[1].num=h[op].num;
    h[1].sum=h[op--].sum;
    long long now=1,nex1;
    while((now<<1)<=op)
    {
        nex1=now<<1;
        if(h[nex1].sum>h[nex1+1].sum&&nex1+1<=op)
        nex1++;
        if(h[nex1].sum>=h[now].sum)
        return res;
        swap(h[nex1].sum,h[now].sum);
        swap(h[nex1].num,h[now].num);
        now=nex1;
    }
    return res;
}
void put(long long x,long long w)
{
    h[++op].num=x;
    h[op].sum=w;
    long long nex1,now=op;
    while(now!=1)
    {
        nex1=now>>1;
        if(h[nex1].sum<=h[now].sum)
        break;
        swap(h[nex1].sum,h[now].sum);
        swap(h[nex1].num,h[now].num);
        now=nex1;
    }
    return;
}
