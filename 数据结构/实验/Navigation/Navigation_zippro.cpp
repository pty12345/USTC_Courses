# include<iostream>
# include<cstring>
# include<vector>
# include<utility>

# define MAXVEX 23947350
//1 23947347
//2 264346
using namespace std;

int GetNumber(FILE *fp)
{    
    string str;
    char ch=0;
    int dec;
    while((ch!=' ')&&(ch != 10))
    {
        fread(&ch,1,1,fp);
        str += ch;
        if(feof(fp))
            break;
    }
    str.erase(str.length()-1);//Delete the bin-char
    int num = atoi(str.c_str());
    return num; 
}

int main()
{
    FILE *fin,*fout;
    int u,v,w;
    int i,length;
    int input;

    vector<vector<int>> node(MAXVEX+1, vector<int>());
    fin = fopen("C:\\Users\\pty\\Desktop\\distance_info.TXT","r");
    fout = fopen("C:\\Users\\pty\\Desktop\\b.TXT","wb");
    while (!feof(fin))
    {
        u = GetNumber(fin);
        v = GetNumber(fin);
        w = GetNumber(fin);
        node[u].push_back(v);
        node[u].push_back(w);
    }
    fclose(fin);
    input = MAXVEX;
    fwrite(&input,sizeof(int),1,fout);//写入总节点数
    for(i=1;i<=MAXVEX;i++)
    {
        length = node[i].size()/2;
        fwrite(&length,sizeof(int),1,fout);//写入该节点边数
        for(auto j=node[i].begin();j<node[i].end();j++)
        {
            input = *j;
            fwrite(&input,sizeof(int),1,fout);
        }
    }
    fclose(fout);
    printf("Compression is over!");
    return 0;
}