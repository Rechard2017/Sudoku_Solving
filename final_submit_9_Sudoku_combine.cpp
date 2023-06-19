#include <iostream>
#include<windows.h>
#include <cstring>
#include <fstream>
#include <queue>
#include <vector>
#include<algorithm>
#include <functional>

using namespace std;

int deep;//深度检测
unsigned int state;
bool change=true;
int boxid[81];//单元格所在宫的索引*
int cellbox[81];//数独单元格全局编号到宫中单元格局部编号的索引*
int boxcell[9][9];//宫中单元格局部编号到数独单元格全局编号的索引*
int dig2bit[10];//数字 k 到对应位数字 1 << （k -1）的索引*
int bit2dig[1<<9];//对应位数字 1 << （k -1）到数字 k 的索引*
int popbit[1<<9];//二进制数中"1"的个数*
double time=0;
double counts=0;
LARGE_INTEGER nFreq;
LARGE_INTEGER nBeginTime;
LARGE_INTEGER nEndTime;


class Suduko
{
public:
    bool fixedCell[81];// 单元格中的数字是否确定**
    int fixedNumber[9];// 全局中每个数字已确定的个数**
    int rowFixed[9];//该行空单元格数量*
    int colFixed[9];//该列空单元格数量*
    int boxFixed[9];//该宫空单元格数量*
    int bitFixRow[9];//一行中的已知数*/
    int bitFixCol[9];//一列中的已知数*/
    int bitFixBox[9];//一宫中的已知数*/
    int bitLiveCell[81];// 单元格的候选数,一位对应一个数字**
    int bitLivRowDig[9][9];//一行中某个候选数的分布,一位(定位，数字)对应一列**
    int bitLivColDig[9][9];//一列中某个候选数的分布,一位对应一行**
    int bitLivBoxDig[9][9];//一宫中某个候选数的分布,一位对应宫内索引**
    int numLiveCell[81];// 单元中候选数个数**
    int numLivRowDig[9][9];// 一行中某个候选数的个数**
    int numLivColDig[9][9];// 一列中某个候选数的个数**
    int numLivBoxDig[9][9];// 一宫中某个候选数的个数**
    bool fixRowDig[9][9];// 一行中某个候选数是否确定**
    bool fixColDig[9][9];// 一列中某个候选数是否确定**
    bool fixBoxDig[9][9];// 一宫中某个候选数是否确定**


    int values[81];// 单元格中填入的数字*
    int ini_num;
    int numFixed;// 已经确定的单元格数量*
    bool solved;// 数独是否成功求解*
    bool failed; // 数独是否求解失败*
    Suduko(string temp)
    {
        int w,b,cont,val;
        numFixed=0;
        ini_num=0;
        solved=false;
        failed=false;

        //0赋值
        for(int i=0; i<9; i++) //数字初始化
        {
            fixedNumber[i] = 0;
            rowFixed[i] = 0;
            colFixed[i] = 0;
            boxFixed[i] = 0;
            bitFixRow[i] = 0;
            bitFixCol[i] = 0;
            bitFixBox[i] = 0;

            for(int j=0; j<9; j++)
            {
                fixedCell[i*9+j] = false;
                bitLiveCell[i*9+j] = 0;
                bitLivRowDig[i][j] = 0;
                bitLivColDig[i][j] = 0;
                bitLivBoxDig[i][j] = 0;
                numLiveCell[i*9+j] = 0;
                numLivRowDig[i][j] = 0;
                numLivColDig[i][j] = 0;
                numLivBoxDig[i][j] = 0;
                fixRowDig[i][j] = false;
                fixColDig[i][j] = false;
                fixBoxDig[i][j] = false;
            }

        }


        for(int i=0; i<81; i++) //数字初始化
        {
            if (temp[i]!='.')
            {
                values[i]=int(temp[i])-48;
                numFixed++;
                fixedCell[i]=true;
                rowFixed[i/9]++;
                colFixed[i%9]++;
                boxFixed[boxid[i]]++;
                ini_num++;
                fixedNumber[values[i]-1]++;

                w=1<<(values[i]-1);
                bitFixRow[i/9]+=w;
                bitFixCol[i%9]+=w;
                bitFixBox[boxid[i]]+=w;

                fixRowDig[i/9][values[i]-1]=true;// 一行中某个候选数是否确定**
                fixColDig[i%9][values[i]-1]=true;// 一列中某个候选数是否确定**
                fixBoxDig[boxid[i]][values[i]-1]=true;

            }
            else
            {
                values[i]=0;
                fixedCell[i]=false;

            }
        }
        //cout<<"数字初始化成功！"<<endl;

        for(int i=0; i<81; i++) //侯选数初始化
        {
            b=0;
            cont=0;
            if(fixedCell[i]!=true)
            {
                bitLiveCell[i]= ((bitFixRow[i/9]|bitFixCol[i%9])|bitFixBox[boxid[i]])^((1<<9)-1);
                b = bitLiveCell[i];
            }

            while(b>0)
            {
                cont++;
                val = bit2dig[(b & ~ (b -1))];// 提取最右端的 1代表的数字
                bitLivRowDig[i/9][val-1]+=dig2bit[i%9+1];//行该侯选数分布
                numLivRowDig[i/9][val-1]++;//行该侯选数个数
                bitLivColDig[i%9][val-1]+=dig2bit[i/9+1];//列该侯选数分布
                numLivColDig[i%9][val-1]++;//列该侯选数个数
                bitLivBoxDig[boxid[i]][val-1]+=dig2bit[cellbox[i]+1];//宫该侯选数分布

                numLivBoxDig[boxid[i]][val-1]++;//宫该侯选数个数
                b = b & (b -1);// 删除最右端的 1
            }
            numLiveCell[i] = cont;

        }
        //cout<<"侯选数初始化成功！"<<endl;
    }

    int remCellLive(int cell,int v)//删除一个方格中的候选数
    {
        int result=0;

        if((bitLiveCell[cell]&dig2bit[v])!=0)
        {
            if(fixedCell[cell]==false && numLiveCell[cell]==1)
            {
                failed=true;
                return 0;
            }
            /*if(fixRowDig[cell/9][v-1]==false && numLivRowDig[cell/9][v-1]==1){failed=true;return 0;}
            if(fixColDig[cell%9][v-1]==false && numLivColDig[cell%9][v-1]==1){failed=true;return 0;}
            if(fixBoxDig[boxid[cell]][v-1]==false && numLivBoxDig[boxid[cell]][v-1]==1){failed=true;return 0;}*/
            bitLiveCell[cell]=bitLiveCell[cell]-dig2bit[v];//移除该格特定侯选数
            bitLivRowDig[cell/9][v-1]=bitLivRowDig[cell/9][v-1]-dig2bit[cell%9+1];//移除该行特定侯选数分布
            bitLivColDig[cell%9][v-1]=bitLivColDig[cell%9][v-1]-dig2bit[cell/9+1];//移除该列特定侯选数分布
            bitLivBoxDig[boxid[cell]][v-1]=bitLivBoxDig[boxid[cell]][v-1]-dig2bit[cellbox[cell]+1];//移除该宫特定侯选数分布
            numLiveCell[cell]--; //减少该行特定侯选数数量
            numLivRowDig[cell/9][v-1]--;//
            numLivColDig[cell%9][v-1]--;
            numLivBoxDig[boxid[cell]][v-1]--;
            change=true;

            if(numLivRowDig[cell/9][v-1]==1)
            {
                result=result | dig2bit[1];
            }
            if(numLivColDig[cell%9][v-1]==1)
            {
                result=result | dig2bit[2];
            }
            if(numLivBoxDig[boxid[cell]][v-1]==1)
            {
                result=result | dig2bit[3];
            }


        }

        return result;

    }

    bool testLiveCell(int cell)// 检验这个单元格是否可以确定-检测出显式1
    {
        if(failed==true)
        {
            return false;
        }
        if(fixedCell[cell]== false && numLiveCell[cell] == 0)
        {
            failed=true;
            return false;
        }
        int val,type=0,type_val[3]= {0};
        bool tag=false;
        if(numLiveCell[cell] == 1)
        {
            change=true;
            values[cell] = bit2dig[bitLiveCell[cell]];
            if(fixRowDig[cell/9][values[cell]-1]||fixColDig[cell%9][values[cell]-1]||fixBoxDig[boxid[cell]][values[cell]-1])
            {
                failed=true;
                return false;
            }

            fixedCell[cell] = true;
            fixedNumber[values[cell]-1]++;
            numFixed++;
            rowFixed[cell/9]++;
            colFixed[cell%9]++;
            boxFixed[boxid[cell]]++;

            fixRowDig[cell/9][values[cell]-1]=true;// 一行中某个候选数是否确定**
            fixColDig[cell%9][values[cell]-1]=true;// 一列中某个候选数是否确定**
            fixBoxDig[boxid[cell]][values[cell]-1]=true;

            remCellLive(cell,values[cell]);


            if(numLivRowDig[cell/9][values[cell]-1]!=0)//行消除侯选数
            {

                while(bitLivRowDig[cell/9][values[cell]-1]>0)
                {
                    val = bit2dig[(bitLivRowDig[cell/9][values[cell]-1] & ~ (bitLivRowDig[cell/9][values[cell]-1] -1))];// 提取最右端的 1代表的数字
                    type=remCellLive((cell/9*9+val-1),values[cell]);//除掉该行这个位置的特定侯选数
                    testLiveCell((cell/9*9+val-1));//检查是否可继续更新
                    if(failed==true)
                    {
                        return false;
                    }

                    type=type&(~dig2bit[1]);//行内不更新行
                    if(boxid[cell]==boxid[(cell/9*9+val-1)])
                    {
                        type=type&(~dig2bit[3]);//若同一宫内不更新宫
                    }
                    while(type>0)//根据type检验是否需要更新
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[values[cell]];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type((cell/9*9+val-1),type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }

            }
            if(numLivColDig[cell%9][values[cell]-1]!=0)//列消除侯选数
            {
                while(bitLivColDig[cell%9][values[cell]-1]>0)
                {
                    val = bit2dig[(bitLivColDig[cell%9][values[cell]-1] & ~ (bitLivColDig[cell%9][values[cell]-1] -1))];// 提取最右端的 1代表的数字
                    type=remCellLive(((val-1)*9+cell%9),values[cell]);//除掉该列这个位置的特定侯选数
                    testLiveCell(((val-1)*9+cell%9));//检查是否可继续更新
                    if(failed==true)
                    {
                        return false;
                    }

                    type=type&(~dig2bit[2]);//列内不更新列
                    if(boxid[cell]==boxid[((val-1)*9+cell%9)])
                    {
                        type=type&(~dig2bit[3]);//若同一宫内不更新宫
                    }
                    while(type>0)//根据type检验是否需要更新
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[values[cell]];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(((val-1)*9+cell%9),type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }

            }
            if(numLivBoxDig[boxid[cell]][values[cell]-1]!=0)//宫消除侯选数
            {

                while(bitLivBoxDig[boxid[cell]][values[cell]-1]>0)
                {
                    val = bit2dig[(bitLivBoxDig[boxid[cell]][values[cell]-1] & ~ (bitLivBoxDig[boxid[cell]][values[cell]-1] -1))];// 提取最右端的 1代表的数字
                    type=remCellLive((boxcell[boxid[cell]][val-1]),values[cell]);//除掉该宫这个位置的特定侯选数
                    testLiveCell((boxcell[boxid[cell]][val-1]));
                    if(failed==true)
                    {
                        return false;
                    }

                    type=type&(~dig2bit[3]);//宫内不更新宫
                    if(cell/9==(boxcell[boxid[cell]][val-1])/9)
                    {
                        type=type&(~dig2bit[1]);//若同一行内不更新行
                    }
                    if(cell%9==(boxcell[boxid[cell]][val-1])%9)
                    {
                        type=type&(~dig2bit[2]);//若同一列内不更新列
                    }
                    while(type>0)//根据type检验是否需要更新
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[values[cell]];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type((boxcell[boxid[cell]][val-1]),type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }


            }

            return true;

        }
        else
        {
            return false;
        }

    }

    bool testLiveCell_force_in(int cell,int v)//强制加入
    {
        if(fixedCell[cell])
        {
            return false;
        }
        int b,val,type=0,type_val[3]= {0};
        bool tag=false;
        if((bitLiveCell[cell] & dig2bit[v])==0)
        {
            return false;
        }
        b=(bitLiveCell[cell] & ~dig2bit[v]);//在侯选数中暂时移除此数
        while(b>0)
        {
            val = bit2dig[b & ~ (b -1)];// 提取最右端的 1代表的数字
            type=remCellLive(cell,val);//除掉这个位置的特定侯选数
            b = b & (b -1);// 删除最右端的 1

            while(type>0)//根据type检验是否需要更新
            {
                tag=true;
                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                type = type & (type -1);
            }

        }
        bitLiveCell[cell]=dig2bit[v];//仅恢复此候选数
        testLiveCell(cell);// 检验这个单元格是否可以确定
        if(failed==true)
        {
            return false;
        }

        if(tag)
        {
            check_hidden1_type(cell,type_val);    //检测更新情况
            if(failed==true)
            {
                return false;
            }
        }
        return true;
    }

    bool check_hidden1_type(int cell,int* type_val)//检测是否因更新候选数而产生隐式1
    {
        int num,pos;
        bool flag=false;
        while(type_val[0]>0)//行更新
        {
            num=bit2dig[ type_val[0] & ~ ( type_val[0] -1)];//cell/9 row : value : num
            if(fixRowDig[cell/9][num-1]==false && numLivRowDig[cell/9][num-1]==1)
            {
                pos=cell/9*9+bit2dig[bitLivRowDig[cell/9][num-1]]-1;
                testLiveCell_force_in(pos,num);
                if(failed==true)
                {
                    return false;
                }
                flag=true;
            }

            type_val[0] = type_val[0] & (type_val[0] -1);

        }
        while(type_val[1]>0)//列更新
        {
            num=bit2dig[ type_val[1] & ~ ( type_val[1] -1)];//cell%9 col : value : num
            if(fixColDig[cell%9][num-1]== false && numLivColDig[cell%9][num-1]==1)
            {
                pos=cell%9+(bit2dig[bitLivColDig[cell%9][num-1]]-1)*9;
                testLiveCell_force_in(pos,num);
                if(failed==true)
                {
                    return false;
                }
                flag=true;
            }

            type_val[1] = type_val[1] & (type_val[1] -1);


        }
        while(type_val[2]>0)//宫更新
        {
            num=bit2dig[ type_val[2] & ~ ( type_val[2] -1)];//box : value : num
            if(fixBoxDig[boxid[cell]][num-1]==false && numLivBoxDig[boxid[cell]][num-1]==1)
            {
                pos= boxcell[boxid[cell]][bit2dig[bitLivBoxDig[boxid[cell]][num-1]]-1];
                testLiveCell_force_in(pos,num);
                if(failed==true)
                {
                    return false;
                }
                flag=true;
            }
            type_val[2] = type_val[2] & (type_val[2] -1);

        }

        return flag;
    }

    void print()
    {
        for(int i = 0; i < 81; i++)
        {
            cout<<values[i];
        }
        cout<<endl;
    }

    bool fixUniqueNumber()//唯一余数法
    {
        for(int i=0; i<81; i++)
        {
            if(fixedCell[i])
            {
                continue;
            }
            testLiveCell(i);
            if(failed==true)
            {
                return false;
            }

        }

        return true;

    }

    bool hiddenMultiples1()//隐性占位法1
    {
        int cell;
        for(int i = 0; i < 9; i++)
        {
            for(int j = 0; j < 9; j++)
            {
                if(numLivRowDig[i][j]==1)
                {

                    cell=i*9+bit2dig[bitLivRowDig[i][j]]-1;//确定该隐形1位置
                    testLiveCell_force_in(cell,j+1);//填入此数
                    if(failed==true)
                    {
                        return false;
                    }

                }
                if(numLivColDig[i][j]==1)
                {

                    cell=i+(bit2dig[bitLivColDig[i][j]]-1)*9;//确定该隐形1位置
                    testLiveCell_force_in(cell,j+1);//填入此数
                    if(failed==true)
                    {
                        return false;
                    }

                }
                if(numLivBoxDig[i][j]==1)
                {

                    cell=boxcell[i][bit2dig[bitLivBoxDig[i][j]]-1];//确定该隐形1位置
                    testLiveCell_force_in(cell,j+1);//填入此数
                    if(failed==true)
                    {
                        return false;
                    }
                }

            }


        }

        return true;
    }

    bool hiddenMultiples2()//隐性占位法2
    {
        int ncons=2,b,col1,col2,k,val,type,type_val[3]= {0}; //设置hidden值
        bool tag = false;
        int cell[ncons]= {0},delbit[ncons]= {0};

        for(int n=0; n<9; n++) //包含行列宫三种
        {
            for(int v1 = 0 ; v1 < 8 ; v1++ )
            {
                if(numLivRowDig[n][v1]==2)
                {
                    for(int v2 = v1+1 ; v2<9 ; v2++)
                    {
                        if(numLivRowDig[n][v2]==2)
                        {
                            if (popbit[(bitLivRowDig[n][v1]&bitLivRowDig[n][v2])]==2)
                            {
                                b = bitLivRowDig[n][v1];
                                col1 = bit2dig[(b & ~(b-1))]-1;//提取最右端的1
                                b = b & (b-1);//删除最右端的1
                                col2 = bit2dig[(b & ~(b-1))]-1;//提取最右端的1
                                cell[0]=n*9+col1;//确定两个单元格坐标
                                cell[1]=n*9+col2;
                                delbit[0]=dig2bit[v1+1];
                                delbit[1]=dig2bit[v2+1];
                                for(k=0; k<ncons; k++)
                                {
                                    if(numLiveCell[cell[k]]>2)//若此格候选数数量大于二
                                    {

                                        b=(bitLiveCell[cell[k]] & ~(dig2bit[v1+1]|dig2bit[v2+1]));//在侯选数中暂时移除此2数
                                        while(b>0)
                                        {
                                            val = bit2dig[b & ~ (b -1)];// 提取最右端的 1代表的数字
                                            type=remCellLive(cell[k],val);//除掉这个位置的特定侯选数
                                            b = b & (b -1);// 删除最右端的 1
                                            while(type>0)//根据type检验是否需要更新
                                            {
                                                tag=true;
                                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                                                type = type & (type -1);
                                            }
                                        }
                                        bitLiveCell[cell[k]]=(dig2bit[v1+1]|dig2bit[v2+1]);//仅恢复此2候选数
                                        if(tag)
                                        {
                                            check_hidden1_type(cell[k],type_val);    //检测更新情况
                                            if(failed==true)
                                            {
                                                return false;
                                            }
                                        }
                                        tag=false;
                                        type_val[0]=0;
                                        type_val[1]=0;
                                        type_val[2]=0;

                                    }
                                }

                                if(boxid[cell[0]]==boxid[cell[1]])
                                {
                                    nakedMultiples(cell,delbit,2);    //检测其他限制中是否出现隐式1
                                    if(failed==true)
                                    {
                                        return false;
                                    }
                                }

                            }
                        }
                    }
                }//row


                if(numLivColDig[n][v1]==2)
                {
                    for(int v2 = v1+1 ; v2<9 ; v2++)
                    {
                        if(numLivColDig[n][v2]==2)
                        {
                            if (popbit[(bitLivColDig[n][v1]&bitLivColDig[n][v2])]==2)
                            {
                                b = bitLivColDig[n][v1];
                                col1 = bit2dig[(b & ~(b-1))]-1;//提取最右端的1
                                b = b & (b-1);//删除最右端的1
                                col2 = bit2dig[(b & ~(b-1))]-1;//提取最右端的1
                                cell[0]=n+col1*9;//确定两个单元格坐标
                                cell[1]=n+col2*9;
                                delbit[0]=dig2bit[v1+1];
                                delbit[1]=dig2bit[v2+1];
                                // cout<<"hidden 2 find in "<<n+1<<" col :"<<col1+1<<" row and "<<col2+1<<" row ::"<<" value are "<<bit2dig[delbit[0]]<<" and "<<bit2dig[delbit[1]]<<endl;
                                for(k=0; k<ncons; k++)
                                {
                                    if(numLiveCell[cell[k]]>2)//若此格候选数数量大于二
                                    {

                                        b=(bitLiveCell[cell[k]] & ~(dig2bit[v1+1]|dig2bit[v2+1]));//在侯选数中暂时移除此2数
                                        while(b>0)
                                        {
                                            val = bit2dig[b & ~ (b -1)];// 提取最右端的 1代表的数字
                                            type=remCellLive(cell[k],val);//除掉这个位置的特定侯选数
                                            //cout<<"移除候选数："<<cell[k]/9+1<<" row "<<cell[k]%9+1<<" col, value is "<<val<<endl;
                                            b = b & (b -1);// 删除最右端的 1
                                            while(type>0)//根据type检验是否需要更新
                                            {
                                                tag=true;
                                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                                                type = type & (type -1);
                                            }
                                        }
                                        bitLiveCell[cell[k]]=(dig2bit[v1+1]|dig2bit[v2+1]);//仅恢复此2候选数
                                        if(tag)
                                        {
                                            check_hidden1_type(cell[k],type_val);    //检测更新情况
                                            if(failed==true)
                                            {
                                                return false;
                                            }
                                        }
                                        tag=false;
                                        type_val[0]=0;
                                        type_val[1]=0;
                                        type_val[2]=0;

                                    }
                                }

                                if(boxid[cell[0]]==boxid[cell[1]])
                                {
                                    nakedMultiples(cell,delbit,2);    //检测其他限制中是否出现隐式1
                                    if(failed==true)
                                    {
                                        return false;
                                    }
                                }

                            }
                        }
                    }
                }//col


                if(numLivBoxDig[n][v1]==2)
                {
                    for(int v2 = v1+1 ; v2<9 ; v2++)
                    {
                        if(numLivBoxDig[n][v2]==2)
                        {
                            if (popbit[(bitLivBoxDig[n][v1]&bitLivBoxDig[n][v2])]==2)
                            {
                                b = bitLivBoxDig[n][v1];
                                col1 = bit2dig[(b & ~(b-1))]-1;//提取最右端的1
                                b = b & (b-1);//删除最右端的1
                                col2 = bit2dig[(b & ~(b-1))]-1;//提取最右端的1
                                cell[0]=boxcell[n][col1];//确定两个单元格坐标
                                cell[1]=boxcell[n][col2];
                                delbit[0]=dig2bit[v1+1];
                                delbit[1]=dig2bit[v2+1];
                                //cout<<"hidden 2 find in "<<n+1<<" box :"<<col1+1<<" pos and "<<col2+1<<" pos ::"<<" value are "<<bit2dig[delbit[0]]<<" and "<<bit2dig[delbit[1]]<<endl;
                                for(k=0; k<ncons; k++)
                                {
                                    if(numLiveCell[cell[k]]>2)//若此格候选数数量大于二
                                    {

                                        b=(bitLiveCell[cell[k]] & ~(dig2bit[v1+1]|dig2bit[v2+1]));//在侯选数中暂时移除此2数
                                        while(b>0)
                                        {
                                            val = bit2dig[b & ~ (b -1)];// 提取最右端的 1代表的数字
                                            type=remCellLive(cell[k],val);//除掉这个位置的特定侯选数
                                            //cout<<"移除候选数："<<cell[k]/9+1<<" row "<<cell[k]%9+1<<" col, value is "<<val<<endl;
                                            b = b & (b -1);// 删除最右端的 1
                                            while(type>0)//根据type检验是否需要更新
                                            {
                                                tag=true;
                                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                                                type = type & (type -1);
                                            }
                                        }
                                        bitLiveCell[cell[k]]=(dig2bit[v1+1]|dig2bit[v2+1]);//仅恢复此2候选数
                                        if(tag)
                                        {
                                            check_hidden1_type(cell[k],type_val);    //检测更新情况
                                            if(failed==true)
                                            {
                                                return false;
                                            }
                                        }
                                        tag=false;
                                        type_val[0]=0;
                                        type_val[1]=0;
                                        type_val[2]=0;

                                    }
                                }

                                if(cell[0]/9==cell[1]/9||cell[0]%9==cell[1]%9)
                                {
                                    nakedMultiples(cell,delbit,2);    //检测其他限制中是否出现隐式1
                                    if(failed==true)
                                    {
                                        return false;
                                    }
                                }

                            }
                        }
                    }
                }//box





            }
        }

        return true;
    }

    bool nakedMultiples(int *ijk,int *delbit,int ncons)//显示2,3通用方法
    {
        int boxbit=0,rowbit=0,colbit=0,k,c,cell,type=0,type_val[3]= {0};
        bool tag=false;
        int row=dig2bit[ijk[0]/9+1];
        int col=dig2bit[ijk[0]%9+1];
        int box=dig2bit[boxid[ijk[0]]+1];
        for(int i=0 ; i<ncons ; i++)
        {
            row&=dig2bit[ijk[i]/9+1];
            col&=dig2bit[ijk[i]%9+1];
            box&=dig2bit[boxid[ijk[i]]+1];
            colbit |= dig2bit[ijk[i]%9+1];// 合并单元格对应的列
            rowbit |= dig2bit[ijk[i]/9+1];// 合并单元格对应的行
            boxbit |= dig2bit[cellbox[ijk[i]]+1];// 合并单元格对应的宫
        }

        if(row!=0)//属于同一行
        {

            for(c =0; c <9; c ++ ) // 循环列,进行候选数删除
            {
                if(dig2bit[c+1] & colbit) continue;// 此列被占据
                cell = (bit2dig[row]-1)*9 + c; // 计算单元格编号
                if(fixedCell[cell])continue;// 这个单元格已经确定
                for(k =0; k < ncons; k ++ ) // 循环删除候选数
                {
                    if(bitLiveCell[cell]&delbit[k]) //这个单元格有这个候选数
                    {
                        type=remCellLive(cell,bit2dig[delbit[k]]);// 删除候选数
                        testLiveCell(cell);// 检验这个单元格是否可以确定
                        if(failed==true)
                        {
                            return false;
                        }
                        while(type>0)//根据type检验是否需要更新
                        {
                            tag=true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=delbit[k];
                            type = type & (type -1);
                        }
                    }
                }
                if(tag)
                {
                    check_hidden1_type(cell,type_val);    //检测更新情况
                    if(failed==true)
                    {
                        return false;
                    }
                }
                tag=false;
                type_val[0]=0;
                type_val[1]=0;
                type_val[2]=0;

            }
        }
        if(col!=0)//属于同一列
        {
            for(c =0; c <9; c ++ ) // 循环行,进行候选数删除
            {
                if(dig2bit[c+1] & rowbit) continue;// 此行被占据
                cell = bit2dig[col]-1 + c*9; // 计算单元格编号
                if(fixedCell[cell])continue;// 这个单元格已经确定
                for(k =0; k < ncons; k ++ ) // 循环删除候选数
                {
                    if(bitLiveCell[cell]&delbit[k]) //这个单元格有这个候选数
                    {
                        type=remCellLive(cell,bit2dig[delbit[k]]);// 删除候选数
                        testLiveCell(cell);// 检验这个单元格是否可以确定
                        if(failed==true)
                        {
                            return false;
                        }
                        while(type>0)//根据type检验是否需要更新
                        {
                            tag=true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=delbit[k];
                            type = type & (type -1);
                        }
                    }
                }
                if(tag)
                {
                    check_hidden1_type(cell,type_val);    //检测更新情况
                    if(failed==true)
                    {
                        return false;
                    }
                }
                tag=false;
                type_val[0]=0;
                type_val[1]=0;
                type_val[2]=0;
            }
        }
        if(box!=0)//属于同一宫
        {
            for(c =0; c <9; c ++ ) // 循环位置,进行候选数删除
            {
                if(dig2bit[c+1] & boxbit) continue;// 此位置被占据
                cell = boxcell[bit2dig[box]-1][c]; // 计算单元格编号
                if(fixedCell[cell])continue;// 这个单元格已经确定
                for(k =0; k < ncons; k ++ ) // 循环删除候选数
                {
                    if(bitLiveCell[cell]&delbit[k]) //这个单元格有这个候选数
                    {
                        type=remCellLive(cell,bit2dig[delbit[k]]);// 删除候选数
                        testLiveCell(cell);// 检验这个单元格是否可以确定
                        if(failed==true)
                        {
                            return false;
                        }
                        while(type>0)//根据type检验是否需要更新
                        {
                            tag=true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=delbit[k];
                            type = type & (type -1);
                        }
                    }
                }
                if(tag)
                {
                    check_hidden1_type(cell,type_val);    //检测更新情况
                    if(failed==true)
                    {
                        return false;
                    }
                }
                tag=false;
                type_val[0]=0;
                type_val[1]=0;
                type_val[2]=0;
            }


        }
        return true;

    }
    bool  nakedMultiples2_()
    {
        int length = 0,waitbitnum;
        int temp[81],ijk[2],delbit[2];
        for(int i=0; i<81; i++)
        {
            if(fixedCell[i]!=true && numLiveCell[i]==2)
            {
                temp[length]=i;
                length = length + 1;
            }
        }

        if(length<2)
        {
            return true;
        }

        for(int j1=0; j1<length-1; j1++)
        {
            ijk[0]=temp[j1];
            for(int j2=j1+1; j2<length; j2++)
            {
                ijk[1] = temp[j2];

                if( fixedCell[ijk[0]] || fixedCell[ijk[1]])
                {
                    continue;
                }

                int row=dig2bit[ijk[0]/9+1] & dig2bit[ijk[1]/9+1],
                    col=dig2bit[ijk[0]%9+1] & dig2bit[ijk[1]%9+1],
                    box=dig2bit[boxid[ijk[0]]+1] & dig2bit[boxid[ijk[1]]+1];

                if(row+col+box==0)
                {
                    continue;   //不在任何相同区域中
                }
                waitbitnum = bitLiveCell[ijk[0]] | bitLiveCell[ijk[1]];
                if(popbit[waitbitnum]==2)
                {
                    delbit[0]=waitbitnum & ~(waitbitnum - 1);
                    waitbitnum = waitbitnum & (waitbitnum -1);
                    delbit[1]=waitbitnum & ~(waitbitnum - 1);
                    nakedMultiples(ijk,delbit,2);
                    if(failed==true)
                    {
                        return false;
                    }
                }
            }


        }
        return true;

    }

    bool  nakedMultiples3_()
    {
        int length = 0,waitbitnum;
        int temp[81],ijk[3],delbit[3];
        for(int i=0; i<81; i++)
        {
            if(fixedCell[i]!=true && numLiveCell[i]<=3)
            {
                temp[length]=i;
                length = length + 1;
            }
        }

        if(length<3)
        {
            return true;
        }

        for(int j1=0; j1<length-2; j1++)
        {
            ijk[0]=temp[j1];
            for(int j2=j1+1; j2<length-1; j2++)
            {
                ijk[1] = temp[j2];
                for(int j3=j2+1; j3<length; j3++)
                {
                    ijk[2] = temp[j3];

                    if( fixedCell[ijk[0]] || fixedCell[ijk[1]] || fixedCell[ijk[2]])
                    {
                        continue;
                    }

                    int row=dig2bit[ijk[0]/9+1],col=dig2bit[ijk[0]%9+1],box=dig2bit[boxid[ijk[0]]+1];
                    for(int i=1 ; i<3 ; i++)
                    {
                        row&=dig2bit[ijk[i]/9+1];
                        col&=dig2bit[ijk[i]%9+1];
                        box&=dig2bit[boxid[ijk[i]]+1];
                    }
                    if(row+col+box==0)
                    {
                        continue;   //不在任何相同区域中
                    }

                    waitbitnum = bitLiveCell[ijk[0]] | bitLiveCell[ijk[1]]| bitLiveCell[ijk[2]];
                    if(popbit[waitbitnum]<3)
                    {
                        failed=true;
                        return false;
                    }
                    if(popbit[waitbitnum]==3)
                    {
                        delbit[0]=waitbitnum & ~(waitbitnum - 1);
                        waitbitnum = waitbitnum & (waitbitnum -1);
                        delbit[1]=waitbitnum & ~(waitbitnum - 1);
                        waitbitnum = waitbitnum & (waitbitnum -1);
                        delbit[2]=waitbitnum & ~(waitbitnum - 1);
                        nakedMultiples(ijk,delbit,3);
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                }
            }


        }
        return true;

    }

    bool intersection()
    {
        int b,cell,cell1,cell2,cell3,box,row,col,type=0,type_val[3]= {0};
        bool tag=false;

        for(int i=0; i<9; i++)
        {
            for(int v=0; v<9; v++) //包含行列宫搜索
            {
                if(numLivRowDig[i][v]==2)//此行存在某数只有两个位置
                {
                    b=bitLivRowDig[i][v];//找到分布
                    cell1=bit2dig[b&(~(b-1))]-1+i*9;
                    b=b&(b-1);
                    cell2=bit2dig[b&(~(b-1))]-1+i*9;
                    if(boxid[cell1]==boxid[cell2] && numLivBoxDig[boxid[cell1]][v]>2)
                    {
                        box=boxid[cell1];
                        b=bitLivBoxDig[box][v]&~(dig2bit[cellbox[cell1]+1]|dig2bit[cellbox[cell2]+1]);//去掉这两个位置后的剩余候选数分布
                        while(b>0)
                        {
                            cell=boxcell[box][bit2dig[b&(~(b-1))]-1];//提取最右端1代表的单元格
                            type=remCellLive(cell,v+1);  //移除侯选数
                            testLiveCell(cell);    //检测是否可确定
                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //删除右端一
                            while(type>0)//根据type检验是否需要更新
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //检测更新情况
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                            tag=false;
                            type_val[0]=0;
                            type_val[1]=0;
                            type_val[2]=0;

                        }
                    }
                }//行测试2
                if(numLivRowDig[i][v]==3)//此行存在某数只有三个位置
                {
                    b=bitLivRowDig[i][v];//找到分布
                    cell1=bit2dig[b&(~(b-1))]-1+i*9;
                    b=b&(b-1);
                    cell2=bit2dig[b&(~(b-1))]-1+i*9;
                    b=b&(b-1);
                    cell3=bit2dig[b&(~(b-1))]-1+i*9;

                    if(boxid[cell1]==boxid[cell2] && boxid[cell3]==boxid[cell2] && numLivBoxDig[boxid[cell1]][v]>3)
                    {
                        box=boxid[cell1];
                        b=bitLivBoxDig[box][v]&~(dig2bit[cellbox[cell1]+1]|dig2bit[cellbox[cell2]+1]|dig2bit[cellbox[cell3]+1]);//去掉这三个位置后的剩余候选数分布
                        while(b>0)
                        {
                            cell=boxcell[box][bit2dig[b&(~(b-1))]-1];//提取最右端1代表的单元格
                            type=remCellLive(cell,v+1);  //移除侯选数
                            testLiveCell(cell);    //检测是否可确定

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //删除右端一
                            while(type>0)//根据type检验是否需要更新
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //检测更新情况
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                            tag=false;
                            type_val[0]=0;
                            type_val[1]=0;
                            type_val[2]=0;

                        }
                    }
                }//行测试3
                if(numLivColDig[i][v]==2)//此列存在某数只有两个位置
                {
                    b=bitLivColDig[i][v];//找到分布
                    cell1=(bit2dig[b&(~(b-1))]-1)*9+i;
                    b=b&(b-1);
                    cell2=(bit2dig[b&(~(b-1))]-1)*9+i;
                    if(boxid[cell1]==boxid[cell2] && numLivBoxDig[boxid[cell1]][v]>2)
                    {
                        box=boxid[cell1];
                        b=bitLivBoxDig[box][v]&~(dig2bit[cellbox[cell1]+1]|dig2bit[cellbox[cell2]+1]);//去掉这两个位置后的剩余候选数分布
                        while(b>0)
                        {
                            cell=boxcell[box][bit2dig[b&(~(b-1))]-1];//提取最右端1代表的单元格
                            type=remCellLive(cell,v+1);  //移除侯选数
                            testLiveCell(cell);    //检测是否可确定

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //删除右端一
                            while(type>0)//根据type检验是否需要更新
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //检测更新情况
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                            tag=false;
                            type_val[0]=0;
                            type_val[1]=0;
                            type_val[2]=0;

                        }
                    }
                }//列测试2
                if(numLivColDig[i][v]==3)//此列存在某数只有两个位置
                {
                    b=bitLivColDig[i][v];//找到分布
                    cell1=(bit2dig[b&(~(b-1))]-1)*9+i;
                    b=b&(b-1);
                    cell2=(bit2dig[b&(~(b-1))]-1)*9+i;
                    b=b&(b-1);
                    cell3=(bit2dig[b&(~(b-1))]-1)*9+i;
                    if(boxid[cell1]==boxid[cell2] && boxid[cell3]==boxid[cell2] && numLivBoxDig[boxid[cell1]][v]>3)
                    {
                        box=boxid[cell1];
                        b=bitLivBoxDig[box][v]&~(dig2bit[cellbox[cell1]+1]|dig2bit[cellbox[cell2]+1]|dig2bit[cellbox[cell3]+1]);//去掉这三个位置后的剩余候选数分布
                        while(b>0)
                        {
                            cell=boxcell[box][bit2dig[b&(~(b-1))]-1];//提取最右端1代表的单元格
                            type=remCellLive(cell,v+1);  //移除侯选数
                            testLiveCell(cell);    //检测是否可确定

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //删除右端一
                            while(type>0)//根据type检验是否需要更新
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //检测更新情况
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                            tag=false;
                            type_val[0]=0;
                            type_val[1]=0;
                            type_val[2]=0;

                        }
                    }
                }//列测试3

                if(numLivBoxDig[i][v]==2)//此宫存在某数只有两个位置
                {
                    b=bitLivBoxDig[i][v];//找到分布
                    cell1 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    b=b&(b-1);
                    cell2 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    if(cell1/9==cell2/9 && numLivRowDig[cell1/9][v]>2)//同行
                    {
                        row=cell1/9;
                        b=bitLivRowDig[row][v]&~(dig2bit[cell1%9+1]|dig2bit[cell2%9+1]);//去掉这两个位置后的剩余候选数分布
                        while(b>0)
                        {
                            cell=bit2dig[b&(~(b-1))]-1+row*9;//提取最右端1代表的单元格
                            type=remCellLive(cell,v+1);  //移除侯选数
                            testLiveCell(cell);    //检测是否可确定

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //删除右端一
                            while(type>0)//根据type检验是否需要更新
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //检测更新情况
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                            tag=false;
                            type_val[0]=0;
                            type_val[1]=0;
                            type_val[2]=0;

                        }
                    }

                    if(cell1%9==cell2%9 && numLivColDig[cell1%9][v]>2)//同列
                    {
                        col=cell1%9;
                        b=bitLivColDig[col][v]&~(dig2bit[cell1/9+1]|dig2bit[cell2/9+1]);//去掉这两个位置后的剩余候选数分布
                        while(b>0)
                        {
                            cell=(bit2dig[b&(~(b-1))]-1)*9+col;//提取最右端1代表的单元格
                            type=remCellLive(cell,v+1);  //移除侯选数
                            testLiveCell(cell);    //检测是否可确定

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //删除右端一
                            while(type>0)//根据type检验是否需要更新
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //检测更新情况
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                            tag=false;
                            type_val[0]=0;
                            type_val[1]=0;
                            type_val[2]=0;

                        }
                    }


                }//宫测试2

                if(numLivBoxDig[i][v]==3)//此宫存在某数只有三个位置
                {
                    b=bitLivBoxDig[i][v];//找到分布
                    cell1 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    b=b&(b-1);
                    cell2 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    b=b&(b-1);
                    cell3 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    if(cell1/9==cell2/9 && cell3/9==cell2/9 && numLivRowDig[cell1/9][v]>3)//同行
                    {
                        row=cell1/9;
                        b=bitLivRowDig[row][v]&~(dig2bit[cell1%9+1]|dig2bit[cell2%9+1]|dig2bit[cell3%9+1]);//去掉这三个位置后的剩余候选数分布
                        while(b>0)
                        {
                            cell=bit2dig[b&(~(b-1))]-1+row*9;//提取最右端1代表的单元格
                            type=remCellLive(cell,v+1);  //移除侯选数
                            testLiveCell(cell);    //检测是否可确定

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //删除右端一
                            while(type>0)//根据type检验是否需要更新
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //检测更新情况
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                            tag=false;
                            type_val[0]=0;
                            type_val[1]=0;
                            type_val[2]=0;

                        }
                    }

                    if(cell1%9==cell2%9 && cell3%9==cell2%9 && numLivColDig[cell1%9][v]>3)//同列
                    {
                        col=cell1%9;
                        b=bitLivColDig[col][v]&~(dig2bit[cell1/9+1]|dig2bit[cell2/9+1]|dig2bit[cell3/9+1]);//去掉这两个位置后的剩余候选数分布
                        while(b>0)
                        {
                            cell=(bit2dig[b&(~(b-1))]-1)*9+col;//提取最右端1代表的单元格
                            type=remCellLive(cell,v+1);  //移除侯选数
                            testLiveCell(cell);    //检测是否可确定

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //删除右端一
                            while(type>0)//根据type检验是否需要更新
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //检测更新情况
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                            tag=false;
                            type_val[0]=0;
                            type_val[1]=0;
                            type_val[2]=0;

                        }
                    }


                }//宫测试3

            }

        }

        return true;

    }//交叉排除法

    bool uniquesquare()
    {
        int cell1,cell2;
        for(int i=0; i<8; i++) //遍历所有宫格
        {
            for(int j=i+1; j<9; j++)
            {
                if(i/3==j/3)//两宫同行
                {
                    for(int c=0; c<3; c++) //对于每宫的3列
                    {
                        for(int c1=0; c1<3; c1++)
                        {
                            for(int c2=c1+1; c2<3; c2++)
                            {
                                cell1 = boxcell[i][c1*3+c];
                                cell2 = boxcell[i][c2*3+c];
                                if(bitLiveCell[cell1]==bitLiveCell[cell2] && numLiveCell[cell1] == 2)
                                {
                                    UR(cell1,cell2,j);
                                    if(failed==true)
                                    {
                                        return false;
                                    }
                                }
                                //i宫
                                cell1 = boxcell[j][c1*3+c];
                                cell2 = boxcell[j][c2*3+c];
                                if(bitLiveCell[cell1]==bitLiveCell[cell2] && numLiveCell[cell1] == 2)
                                {
                                    UR(cell1,cell2,i);
                                    if(failed==true)
                                    {
                                        return false;
                                    }
                                }
                                //j宫

                            }
                        }

                    }
                }

                if(i%3==j%3)//两宫同列
                {
                    for(int c=0; c<3; c++) //对于每宫的3行
                    {
                        for(int c1=0; c1<3; c1++)
                        {
                            for(int c2=c1+1; c2<3; c2++)
                            {
                                cell1 = boxcell[i][c*3+c1];
                                cell2 = boxcell[i][c*3+c2];
                                if(bitLiveCell[cell1]==bitLiveCell[cell2] && numLiveCell[cell1] == 2)
                                {
                                    UR(cell1,cell2,j);
                                    if(failed==true)
                                    {
                                        return false;
                                    }
                                }
                                //i宫
                                cell1 = boxcell[j][c*3+c1];
                                cell2 = boxcell[j][c*3+c2];
                                if(bitLiveCell[cell1]==bitLiveCell[cell2] && numLiveCell[cell1] == 2)
                                {
                                    UR(cell1,cell2,i);
                                    if(failed==true)
                                    {
                                        return false;
                                    }
                                }
                                //j宫

                            }
                        }

                    }

                }



            }
        }



        return true;

    }

    bool UR(int cell1,int cell2,int box)
    {
        int first;
        int second;
        int bigrow, bigcol;
        int temp_cell;
        int another_cell;
        int b,val,type,type_val[3]= {0};
        bool tag=false;
        int waitnumber1,waitnumber2;
        int posofcell;

        b = bitLiveCell[cell1];
        waitnumber1= bit2dig[b & ~ (b -1)];
        b = b & (b -1);
        waitnumber2= bit2dig[b & ~ (b -1)];

        if(cell1/9 == cell2/9)//两格同行
        {
            first = cell1%9;
            second = cell2%9;
            bigrow = box/3;

            posofcell= dig2bit[first+1]|dig2bit[second+1];

            for(int i = 0; i<3; i++ ) //第一列
            {
                temp_cell = (bigrow*3+i)*9 + first;
                if(bitLiveCell[cell1] == bitLiveCell[temp_cell])
                {
                    another_cell = (bigrow*3+i)*9 + second;
                    b = bitLiveCell[cell1];
                    while(b > 0)
                    {
                        val = bit2dig[b & ~ (b -1)];// 提取最右端的 1代表的数字
                        type = remCellLive(another_cell,val);//除掉这个位置的特定侯选数
                        if(failed==true)
                        {
                            return false;
                        }
                        b = b & (b -1);// 删除最右端的 1
                        while(type>0)//根据type检验是否需要更新
                        {
                            tag = true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                            type = type & (type -1);
                        }
                    }
                    testLiveCell(another_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;
                }
            }

            for(int i = 0; i<3; i++ ) //第二列
            {
                temp_cell = (bigrow*3+i)*9 + second;
                if(bitLiveCell[cell1] == bitLiveCell[temp_cell])
                {
                    another_cell = (bigrow*3+i)*9 + first;
                    b = bitLiveCell[cell1];
                    while(b > 0)
                    {
                        val = bit2dig[b & ~ (b -1)];// 提取最右端的 1代表的数字
                        type = remCellLive(another_cell,val);//除掉这个位置的特定侯选数
                        b = b & (b -1);// 删除最右端的 1
                        while(type>0)//根据type检验是否需要更新
                        {
                            tag = true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                            type = type & (type -1);
                        }
                    }
                    testLiveCell(another_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;
                }
            }

            //type2
            for(int i=0; i<3; i++)
            {
                if(bitLivRowDig[bigrow*3+i][waitnumber1-1] == posofcell && (bitLivRowDig[bigrow*3+i][waitnumber2-1] & posofcell)!=0 )//第一候选数共轭，第二存在
                {
                    temp_cell = (bigrow*3+i)*9 + first;
                    another_cell = (bigrow*3+i)*9 + second;
                    //除第一方格
                    type = remCellLive(temp_cell,waitnumber2);//除掉这个位置的特定侯选数
                    testLiveCell(temp_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//根据type检验是否需要更新
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber2];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(temp_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                    //除第二方格
                    type = remCellLive(another_cell,waitnumber2);//除掉这个位置的特定侯选数
                    testLiveCell(another_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//根据type检验是否需要更新
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber2];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }

                if(bitLivRowDig[bigrow*3+i][waitnumber2-1] == posofcell && (bitLivRowDig[bigrow*3+i][waitnumber1-1] & posofcell)!=0 )//第二候选数共轭，第一存在
                {
                    temp_cell = (bigrow*3+i)*9 + first;
                    another_cell = (bigrow*3+i)*9 + second;
                    //除第一方格
                    type = remCellLive(temp_cell,waitnumber1);//除掉这个位置的特定侯选数
                    testLiveCell(temp_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//根据type检验是否需要更新
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber1];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(temp_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                    //除第二方格
                    type = remCellLive(another_cell,waitnumber1);//除掉这个位置的特定侯选数
                    testLiveCell(another_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//根据type检验是否需要更新
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber1];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;
                }
            }
        }

        if(cell1%9 == cell2%9 )//两格同列
        {

            first = cell1/9;
            second = cell2/9;
            bigcol = box%3;
            posofcell= dig2bit[first+1]|dig2bit[second+1];

            for(int i = 0; i<3; i++ ) //第一列
            {
                temp_cell = bigcol*3+i + first*9;
                if(bitLiveCell[cell1] == bitLiveCell[temp_cell])
                {
                    another_cell = bigcol*3+i + second*9;
                    b = bitLiveCell[cell1];
                    while(b > 0)
                    {
                        val = bit2dig[b & ~ (b -1)];// 提取最右端的 1代表的数字
                        type = remCellLive(another_cell,val);//除掉这个位置的特定侯选数
                        b = b & (b -1);// 删除最右端的 1
                        while(type>0)//根据type检验是否需要更新
                        {
                            tag = true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                            type = type & (type -1);
                        }
                    }
                    testLiveCell(another_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;
                }
            }

            for(int i = 0; i<3; i++ ) //第二列
            {
                temp_cell = bigcol*3+i + second*9;
                if(bitLiveCell[cell1] == bitLiveCell[temp_cell])
                {
                    another_cell = bigcol*3+i + first*9;
                    b = bitLiveCell[cell1];
                    while(b > 0)
                    {
                        val = bit2dig[b & ~ (b -1)];// 提取最右端的 1代表的数字
                        type = remCellLive(another_cell,val);//除掉这个位置的特定侯选数
                        b = b & (b -1);// 删除最右端的 1
                        while(type>0)//根据type检验是否需要更新
                        {
                            tag = true;
                            type_val[bit2dig[type & ~ (type -1)]-1] |= dig2bit[val];
                            type = type & (type -1);
                        }
                    }
                    testLiveCell(another_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;
                }
            }

            //type2
            for(int i=0; i<3; i++)
            {
                if(bitLivColDig[bigcol*3+i][waitnumber1-1] == posofcell && (bitLivColDig[bigcol*3+i][waitnumber2-1] & posofcell)!=0 )//第一候选数共轭，第二候选数存在
                {
                    temp_cell = bigcol*3+i + first*9;
                    another_cell = bigcol*3+i + second*9;
                    //除第一方格
                    type = remCellLive(temp_cell,waitnumber2);//除掉这个位置的特定侯选数
                    testLiveCell(temp_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//根据type检验是否需要更新
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber2];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(temp_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                    //除第二方格
                    type = remCellLive(another_cell,waitnumber2);//除掉这个位置的特定侯选数
                    testLiveCell(another_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//根据type检验是否需要更新
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber2];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }

                if(bitLivColDig[bigcol*3+i][waitnumber2-1] == posofcell && (bitLivColDig[bigcol*3+i][waitnumber1-1] & posofcell)!=0 )//第二候选数共轭,第一候选数存在
                {
                    temp_cell =  bigcol*3+i + first*9;
                    another_cell = bigcol*3+i + second*9;
                    //除第一方格
                    type = remCellLive(temp_cell,waitnumber1);//除掉这个位置的特定侯选数
                    testLiveCell(temp_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//根据type检验是否需要更新
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber1];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(temp_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                    //除第二方格
                    type = remCellLive(another_cell,waitnumber1);//除掉这个位置的特定侯选数
                    testLiveCell(another_cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//根据type检验是否需要更新
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber1];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;
                }
            }
        }

        return true;
    }

    bool fish()
    {
        for(int i=0; i<9; i++) //一到九的数字
        {
            if(fixedNumber[i] <= 5)
            {
                for(int row=0; row<8; row++) //行定义域
                {
                    if(numLivRowDig[row][i]==2)
                    {
                        for(int row2=row+1; row2<9; row2++)
                        {
                            if(bitLivRowDig[row][i]==bitLivRowDig[row2][i] && numLivRowDig[row][i]==2)
                            {
                                killfish(row,row2,bitLivRowDig[row][i],i+1,1);
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                        }

                    }

                }

                for(int col=0; col<8; col++) //列定义域
                {
                    if(numLivColDig[col][i]==2)
                    {
                        for(int col2=col+1; col2<9; col2++)
                        {
                            if(bitLivColDig[col][i]==bitLivColDig[col2][i] && numLivColDig[col][i]==2)
                            {
                                killfish(col,col2,bitLivColDig[col][i],i+1,2);
                                if(failed==true)
                                {
                                    return false;
                                }
                            }
                        }

                    }
                }


            }
        }

        return true;
    }

    bool killfish(int first,int second,int deletepos,int value,int t)
    {
        int b = deletepos, cell, type = 0, type_val[3] = {0};
        bool tag = false;
        bool deletefirst = false, deletesecond = false;
        int delete1 = bit2dig[b & ~ (b -1)];
        b = b & (b -1);
        int delete2 = bit2dig[b & ~ (b -1)];

        if(t==1)//行定义，列删除
        {
            if(numLivColDig[delete1-1][value-1]>2)
            {
                deletefirst=true;
            }
            if(numLivColDig[delete2-1][value-1]>2)
            {
                deletesecond=true;
            }

            if(deletefirst)//第一删除域
            {
                b = bitLivColDig[delete1-1][value-1];
                b = b & ~dig2bit[first+1] & ~dig2bit[second+1];
                while(b>0)
                {
                    cell = (bit2dig[b & ~ (b -1)]-1)*9 + delete1-1;
                    b = b & (b -1);
                    type=remCellLive(cell,value);// 删除候选数
                    testLiveCell(cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }
                    while(type>0)//根据type检验是否需要更新
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[value];
                        type = type & (type -1);
                    }

                    if(tag)
                    {
                        check_hidden1_type(cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }

                deletefirst = false;
            }

            if(deletesecond)//第二删除域
            {
                b = bitLivColDig[delete2-1][value-1];
                b = b & ~dig2bit[first+1] & ~dig2bit[second+1];
                while(b>0)
                {
                    cell = (bit2dig[b & ~ (b -1)]-1)*9 + delete2-1;
                    b = b & (b -1);
                    type=remCellLive(cell,value);// 删除候选数
                    testLiveCell(cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }
                    while(type>0)//根据type检验是否需要更新
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[value];
                        type = type & (type -1);
                    }

                    if(tag)
                    {
                        check_hidden1_type(cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }

                deletesecond = false;
            }


        }

        if(t==2) //列定义，行删除
        {
            if(numLivRowDig[delete1-1][value-1]>2)
            {
                deletefirst=true;
            }
            if(numLivRowDig[delete2-1][value-1]>2)
            {
                deletesecond=true;
            }

            if(deletefirst)//第一删除域
            {
                b = bitLivRowDig[delete1-1][value-1];
                b = b & ~dig2bit[first+1] & ~dig2bit[second+1];
                while(b>0)
                {
                    cell = (bit2dig[b & ~ (b -1)]-1) + (delete1-1)*9;
                    b = b & (b -1);
                    type=remCellLive(cell,value);// 删除候选数
                    testLiveCell(cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }
                    while(type>0)//根据type检验是否需要更新
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[value];
                        type = type & (type -1);
                    }

                    if(tag)
                    {
                        check_hidden1_type(cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }

                deletefirst = false;
            }

            if(deletesecond)//第二删除域
            {
                b = bitLivRowDig[delete2-1][value-1];
                b = b & ~dig2bit[first+1] & ~dig2bit[second+1];
                while(b>0)
                {
                    cell = (bit2dig[b & ~ (b -1)]-1) + (delete2-1)*9;
                    b = b & (b -1);
                    type=remCellLive(cell,value);// 删除候选数
                    testLiveCell(cell);// 检验这个单元格是否可以确定
                    if(failed==true)
                    {
                        return false;
                    }
                    while(type>0)//根据type检验是否需要更新
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[value];
                        type = type & (type -1);
                    }

                    if(tag)
                    {
                        check_hidden1_type(cell,type_val);    //检测更新情况
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                }

                deletesecond = false;
            }

        }

        return true;

    }

    bool dfs(int x,int y)//正向填写dfs
    {
//维护三个数组
        if (x==9||numFixed==81)
        {
            return true;
        }
        int cell=x*9+y;
        if (fixedCell[cell])
        {
            if (y+1<9) return dfs(x,y+1);
            else return dfs(x+1,0);
        }

        int b=bitLiveCell[cell];//可用候选数
        Suduko p=*this;
        while (b>0)
        {
            int m=b&(~(b-1));//取最右端的1
            testLiveCell_force_in(cell,bit2dig[m]);
            change = true;
            while(failed==false && change==true)
            {
                //人类策略+
                change=false;
                intersection();
                if(failed==false)
                {
                    //hiddenMultiples2();
                    if(failed==false)
                    {
                        //nakedMultiples2_();
                        if(failed==false)
                        {
                            //fish();
                            //uniquesquare();
                        }

                    }


                }
            }

            if(failed==false)
            {
                if(numFixed==81)
                {
                    return true;
                }
                else
                {
                    if (y+1<9)
                    {
                        if (dfs(x,y+1)) return true;    //传递1
                    }
                    else
                    {
                        if (dfs(x+1,0)) return true;
                    }
                }
            }
            *this=p;
            b=b&(b-1);
        }

        return false;
    }

    bool opt_dfs()//选择性填写dfs(候选数最少的cell)
    {
        int minnum=10,cell;
//维护三个数组
        if (numFixed==81)
        {
            return true;
        }

        for(int i=0; i<81; i++)
        {
            if(fixedCell[i])continue;
            if(minnum>numLiveCell[i])
            {
                minnum=numLiveCell[i];
                cell=i;
            }
        }

        deep = deep + 1;

        int b=bitLiveCell[cell];//可用候选数
        Suduko p=*this;
        while (b>0)
        {
            int m=b&(~(b-1));//取最右端的1
            testLiveCell_force_in(cell,bit2dig[m]);
            change = true;
            if(failed==false && change==true) //人类策略+ 可将if更改为while
            {
                change=false;
                intersection();
                if(failed==false)
                {
                    hiddenMultiples2();
                    if(failed==false)
                    {
                        //nakedMultiples2_();
                        if(failed==false)
                        {
                            //fish();
                            //uniquesquare();
                        }
                    }
                }
            }

            if(failed==false)
            {
                if(numFixed==81)
                {
                    return true;
                }
                else
                {
                    if (opt_dfs()) return true;  //传递1

                }
            }


            *this=p;
            b=b&(b-1);
        }

        deep=deep-1;
        return false;
    }

    bool opt2_dfs()//选择性填写dfs（候选数最少cell，全局已确定的数字最多）
    {
        int minnum=10,cell,choice,m,max_choice,max_m;
//维护三个数组
        if (numFixed==81)
        {
            return true;
        }

        for(int i=0; i<81; i++)
        {
            if(fixedCell[i])continue;
            if(minnum>numLiveCell[i])
            {
                minnum=numLiveCell[i];
                cell=i;
            }
        }

        deep = deep + 1;

        int b=bitLiveCell[cell];//可用候选数
        Suduko p=*this;
        while (b>0)
        {
            choice=b;
            max_choice=-1;
            while(choice>0)
            {
                m = choice&(~(choice-1));
                choice=choice&(choice-1);
                if(max_choice < fixedNumber[bit2dig[m]-1])
                {
                    max_choice = fixedNumber[bit2dig[m]-1];
                    max_m=m;
                }
            }

            testLiveCell_force_in(cell,bit2dig[max_m]);

            change = true;
            if(failed==false && change==true) //人类策略+ 可将if更改为while
            {
                change=false;
                intersection();
                if(failed==false)
                {
                    hiddenMultiples2();
                    if(failed==false)
                    {
                        //nakedMultiples2_();
                        if(failed==false)
                        {
                            //fish();
                            //uniquesquare();
                        }
                    }
                }
            }
            if(failed==false)
            {
                if(numFixed==81)
                {
                    return true;
                }
                else
                {
                    if (opt2_dfs()) return true;  //传递1

                }
            }


            *this=p;
            b=b-max_m;
        }
        deep=deep-1;
        return false;
    }

    bool opt3_dfs()//按照数字顺序填写
    {
        int maxnum=-1,number,row=0;
//维护三个数组

        if (numFixed==81)
        {
            return true;
        }

        for(int i=0; i<9; i++)
        {
            if(fixedNumber[i]==9)
            {
                continue;
            }
            if(maxnum < fixedNumber[i])
            {
                maxnum = fixedNumber[i];
                number = i;
            }
        }


        while(fixRowDig[row][number]==true)
        {
            row++;
        }

        int b=bitLivRowDig[row][number];
        deep=deep+1;
        Suduko p=*this;

        while (b>0)
        {
            int m=b&(~(b-1));//取最右端的1
            testLiveCell_force_in(row*9+bit2dig[m]-1,number+1);
            change = true;
            if(failed==false && change==true) //人类策略+ 可将if更改为while
            {
                change=false;
                intersection();
                if(failed==false)
                {
                    hiddenMultiples2();
                    if(failed==false)
                    {
                        //nakedMultiples2_();
                        if(failed==false)
                        {
                            //fish();
                            //uniquesquare();
                        }

                    }
                }
            }
            if(failed==false)
            {
                if(numFixed==81)
                {
                    return true;
                }
                else
                {
                    if (opt3_dfs()) return true;  //传递1
                }
            }

            *this=p;
            b=b&(b-1);

        }
        deep=deep-1;
        return false;
    }

    bool bfs()
    {
        queue <Suduko> q;
        q.push(*this);

        int minnum=10,cell;
        while(q.size()!=0)
        {
            if(q.size()>state)
            {
                state = q.size();
            }
            *this=q.front();
            if(numFixed==81)
            {
                return true;
            }

            minnum=10;
            //寻找最小候选数方格
            for(int i=0; i<81; i++)
            {
                if(fixedCell[i])continue;
                if(minnum > numLiveCell[i])
                {
                    minnum=numLiveCell[i];
                    cell=i;
                }
            }

            int b=bitLiveCell[cell];//可用候选数
            Suduko p=*this;

            while (b>0)
            {
                int m=b&(~(b-1));//取最右端的1
                b=b&(b-1);
                testLiveCell_force_in(cell,bit2dig[m]);

                change = true;
                while(failed==false && change==true) //人类策略+ 可将if更改为while
                {
                    change=false;
                    intersection();
                    if(failed==false)
                    {
                        hiddenMultiples2();
                        if(failed==false)
                        {
                            //nakedMultiples2_();
                            if(failed==false)
                            {
                                //fish();
                                //uniquesquare();
                            }

                        }
                    }
                }

                if(failed==false)
                {
                    if(numFixed==81)
                    {
                        return true;
                    }
                    else
                    {
                        q.push(*this);
                    }
                }

//返回上一状态
                *this=p;

            }

            q.pop();

        }

        return false;

    }

    bool opt_bfs() //选取剩余方格最少的首先计算
    {
        vector <Suduko> q;
        q.push_back(*this);

        int minnum=10,cell;
        while(q.size()!=0)
        {
            sort(q.begin(),q.end(),cmp);
            *this=q[0];
            //cout<<"数组长度："<<q.size()<<"确定数字个数："<<numFixed<<endl;
            if(numFixed==81)
            {
                return true;
            }

            minnum=10;
            //寻找最小候选数方格
            for(int i=0; i<81; i++)
            {
                if(fixedCell[i])continue;
                if(minnum>numLiveCell[i])
                {
                    minnum=numLiveCell[i];
                    cell=i;
                }
            }

            int b=bitLiveCell[cell];//可用候选数
            Suduko p=*this;

            while (b>0)
            {
                int m=b&(~(b-1));//取最右端的1
                b=b&(b-1);
                testLiveCell_force_in(cell,bit2dig[m]);

                change = true;
                while(failed==false && change==true) //人类策略+ 可将if更改为while
                {
                    change=false;
                    intersection();
                    if(failed==false)
                    {
                        hiddenMultiples2();
                        if(failed==false)
                        {
                            //nakedMultiples2_();
                            if(failed==false)
                            {
                                //fish();
                                //uniquesquare();
                            }
                        }
                    }
                }

                if(failed==false)
                {
                    if(numFixed==81)
                    {
                        return true;
                    }
                    else
                    {
                        q.push_back(*this);
                    }
                }

//返回上一状态
                *this=p;

            }

            q.erase(q.begin(),q.begin()+1);

        }

        return false;

    }

    bool opt2_bfs() //选取剩余方格最少的首先计算 priority queue
    {
        priority_queue<Suduko,vector<Suduko>,cmp2> q;
        q.push(*this);

        int minnum=10,cell;
        while(q.size()!=0)
        {
            *this=q.top();
            q.pop();
            //cout<<"数组长度："<<q.size()<<"确定数字个数："<<numFixed<<endl;
            if(numFixed==81)
            {
                return true;
            }

            minnum=10;
            //寻找最小候选数方格
            for(int i=0; i<81; i++)
            {
                if(fixedCell[i])continue;
                if(minnum>numLiveCell[i])
                {
                    minnum=numLiveCell[i];
                    cell=i;
                }
            }

            int b=bitLiveCell[cell];//可用候选数
            Suduko p=*this;

            while (b>0)
            {
                int m=b&(~(b-1));//取最右端的1
                b=b&(b-1);
                testLiveCell_force_in(cell,bit2dig[m]);

                change = true;
                while(failed==false && change==true) //人类策略+ 可将if更改为while
                {
                    change=false;
                    intersection();
                    if(failed==false)
                    {
                        hiddenMultiples2();
                        if(failed==false)
                        {
                            //nakedMultiples2_();
                            if(failed==false)
                            {
                                //fish();
                                //uniquesquare();
                            }

                        }
                    }
                }

                if(failed==false)
                {
                    if(numFixed==81)
                    {
                        return true;
                    }
                    else
                    {
                        q.push(*this);
                    }
                }

//返回上一状态
                *this=p;

            }


        }

        return false;

    }

    static bool cmp(Suduko a,Suduko b)
    {
        return a.numFixed>b.numFixed;//最大值优先>
    }

    struct cmp2
    {
    public:
        bool operator ()(Suduko a,Suduko b) const
        {
            return a.numFixed < b.numFixed;//最大值优先<
        }
    };

};

int main()
{
    //初始化全局数组
    int i,j;
    for(i=0; i<9; i++)
    {
        for(j=0; j<9; j++)
        {
            boxid[i*9+j] = (i/3)*3+(j/3);
            boxcell[i][j]=((i/3)*3+j/3)*9+i%3*3+j%3;
            cellbox[i*9+j]=i%3*3+j%3;
        }

        dig2bit[i]=1<<(i-1);
        bit2dig[1<<i]=i+1;
    }
    dig2bit[1]=1;
    dig2bit[9]=1<<8;


    for(j=0; j<(1<<9); j++)
    {
        int counts=0;//计算1的个数
        int i=j;
        while(i!=0)
        {
            i=((i-1)&i);//任何二进制数减1在与原数进行与运算，最终结果即为二进制数中1的个数
            counts++;
        }
        popbit[j]=counts;
    }

    //测试开始
    bool f;
    double sumstate=0, sumt=0,maxt=0,mint=500.00000000;

    //数独文件位置
    ifstream infile("D:/test.txt");//数据集列表"Hodoku137.txt" [Hodoku137,beijing91,Su17_49151,extreme15,Andoku500,17Su_diff_500,95_hard,Su17_49151]
    string temp;
    while(getline(infile,temp))
    {
        Suduko s(temp);
        QueryPerformanceFrequency(&nFreq);
        QueryPerformanceCounter(&nBeginTime);//开始计时

        s.fixUniqueNumber();
        s.hiddenMultiples1();

        while(change)
        {
            change=false;
            s.intersection();
            s.hiddenMultiples2();
            //s.nakedMultiples2_();
            //s.fish();
            //s.uniquesquare();

        }

        deep=0;
        state=0;
        f = s.opt_dfs();//opt_dfs 和 opt2_dfs最快
        //f=s.dfs(0,0);
        QueryPerformanceCounter(&nEndTime);//停止计时
        if(f)
        {
            //答案输出开启 影响时间计算 注释
            //s.print();
        }
        else
        {
            cout<<"无解"<<endl;
        }

        time=(double)(nEndTime.QuadPart-nBeginTime.QuadPart)/(double)nFreq.QuadPart;//计算程序执行时间单位为微秒
        //cout<<" 运行时间 ："<<time*1000<<"ms"<<endl;
        //cout<<" 深度 : "<<deep<<endl;
       // cout<<" 状态 : "<<state<<endl;
        if(time>maxt)
        {
            maxt=time;
        }
        if(time<mint)
        {
            mint=time;
        }
        sumt=sumt+time;
        sumstate = sumstate + state;
    }

    cout<<"运行最长时间："<<maxt*1000<<"ms"<<endl;
    cout<<"运行最短时间："<<mint*1000<<"ms"<<endl;
    cout<<"运行总时间："<<sumt*1000<<"ms"<<endl;
    cout<<"总状态存储："<<sumstate<<endl;

    //刷新
    int x;
    cin>>x;

    return 0;
}

