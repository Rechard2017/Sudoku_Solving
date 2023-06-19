#include <iostream>
#include<windows.h>
#include <cstring>
#include <fstream>
#include <queue>
#include <vector>
#include<algorithm>
#include <functional>

using namespace std;

int deep;//��ȼ��
unsigned int state;
bool change=true;
int boxid[81];//��Ԫ�����ڹ�������*
int cellbox[81];//������Ԫ��ȫ�ֱ�ŵ����е�Ԫ��ֲ���ŵ�����*
int boxcell[9][9];//���е�Ԫ��ֲ���ŵ�������Ԫ��ȫ�ֱ�ŵ�����*
int dig2bit[10];//���� k ����Ӧλ���� 1 << ��k -1��������*
int bit2dig[1<<9];//��Ӧλ���� 1 << ��k -1�������� k ������*
int popbit[1<<9];//����������"1"�ĸ���*
double time=0;
double counts=0;
LARGE_INTEGER nFreq;
LARGE_INTEGER nBeginTime;
LARGE_INTEGER nEndTime;


class Suduko
{
public:
    bool fixedCell[81];// ��Ԫ���е������Ƿ�ȷ��**
    int fixedNumber[9];// ȫ����ÿ��������ȷ���ĸ���**
    int rowFixed[9];//���пյ�Ԫ������*
    int colFixed[9];//���пյ�Ԫ������*
    int boxFixed[9];//�ù��յ�Ԫ������*
    int bitFixRow[9];//һ���е���֪��*/
    int bitFixCol[9];//һ���е���֪��*/
    int bitFixBox[9];//һ���е���֪��*/
    int bitLiveCell[81];// ��Ԫ��ĺ�ѡ��,һλ��Ӧһ������**
    int bitLivRowDig[9][9];//һ����ĳ����ѡ���ķֲ�,һλ(��λ������)��Ӧһ��**
    int bitLivColDig[9][9];//һ����ĳ����ѡ���ķֲ�,һλ��Ӧһ��**
    int bitLivBoxDig[9][9];//һ����ĳ����ѡ���ķֲ�,һλ��Ӧ��������**
    int numLiveCell[81];// ��Ԫ�к�ѡ������**
    int numLivRowDig[9][9];// һ����ĳ����ѡ���ĸ���**
    int numLivColDig[9][9];// һ����ĳ����ѡ���ĸ���**
    int numLivBoxDig[9][9];// һ����ĳ����ѡ���ĸ���**
    bool fixRowDig[9][9];// һ����ĳ����ѡ���Ƿ�ȷ��**
    bool fixColDig[9][9];// һ����ĳ����ѡ���Ƿ�ȷ��**
    bool fixBoxDig[9][9];// һ����ĳ����ѡ���Ƿ�ȷ��**


    int values[81];// ��Ԫ�������������*
    int ini_num;
    int numFixed;// �Ѿ�ȷ���ĵ�Ԫ������*
    bool solved;// �����Ƿ�ɹ����*
    bool failed; // �����Ƿ����ʧ��*
    Suduko(string temp)
    {
        int w,b,cont,val;
        numFixed=0;
        ini_num=0;
        solved=false;
        failed=false;

        //0��ֵ
        for(int i=0; i<9; i++) //���ֳ�ʼ��
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


        for(int i=0; i<81; i++) //���ֳ�ʼ��
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

                fixRowDig[i/9][values[i]-1]=true;// һ����ĳ����ѡ���Ƿ�ȷ��**
                fixColDig[i%9][values[i]-1]=true;// һ����ĳ����ѡ���Ƿ�ȷ��**
                fixBoxDig[boxid[i]][values[i]-1]=true;

            }
            else
            {
                values[i]=0;
                fixedCell[i]=false;

            }
        }
        //cout<<"���ֳ�ʼ���ɹ���"<<endl;

        for(int i=0; i<81; i++) //��ѡ����ʼ��
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
                val = bit2dig[(b & ~ (b -1))];// ��ȡ���Ҷ˵� 1���������
                bitLivRowDig[i/9][val-1]+=dig2bit[i%9+1];//�иú�ѡ���ֲ�
                numLivRowDig[i/9][val-1]++;//�иú�ѡ������
                bitLivColDig[i%9][val-1]+=dig2bit[i/9+1];//�иú�ѡ���ֲ�
                numLivColDig[i%9][val-1]++;//�иú�ѡ������
                bitLivBoxDig[boxid[i]][val-1]+=dig2bit[cellbox[i]+1];//���ú�ѡ���ֲ�

                numLivBoxDig[boxid[i]][val-1]++;//���ú�ѡ������
                b = b & (b -1);// ɾ�����Ҷ˵� 1
            }
            numLiveCell[i] = cont;

        }
        //cout<<"��ѡ����ʼ���ɹ���"<<endl;
    }

    int remCellLive(int cell,int v)//ɾ��һ�������еĺ�ѡ��
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
            bitLiveCell[cell]=bitLiveCell[cell]-dig2bit[v];//�Ƴ��ø��ض���ѡ��
            bitLivRowDig[cell/9][v-1]=bitLivRowDig[cell/9][v-1]-dig2bit[cell%9+1];//�Ƴ������ض���ѡ���ֲ�
            bitLivColDig[cell%9][v-1]=bitLivColDig[cell%9][v-1]-dig2bit[cell/9+1];//�Ƴ������ض���ѡ���ֲ�
            bitLivBoxDig[boxid[cell]][v-1]=bitLivBoxDig[boxid[cell]][v-1]-dig2bit[cellbox[cell]+1];//�Ƴ��ù��ض���ѡ���ֲ�
            numLiveCell[cell]--; //���ٸ����ض���ѡ������
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

    bool testLiveCell(int cell)// ���������Ԫ���Ƿ����ȷ��-������ʽ1
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

            fixRowDig[cell/9][values[cell]-1]=true;// һ����ĳ����ѡ���Ƿ�ȷ��**
            fixColDig[cell%9][values[cell]-1]=true;// һ����ĳ����ѡ���Ƿ�ȷ��**
            fixBoxDig[boxid[cell]][values[cell]-1]=true;

            remCellLive(cell,values[cell]);


            if(numLivRowDig[cell/9][values[cell]-1]!=0)//��������ѡ��
            {

                while(bitLivRowDig[cell/9][values[cell]-1]>0)
                {
                    val = bit2dig[(bitLivRowDig[cell/9][values[cell]-1] & ~ (bitLivRowDig[cell/9][values[cell]-1] -1))];// ��ȡ���Ҷ˵� 1���������
                    type=remCellLive((cell/9*9+val-1),values[cell]);//�����������λ�õ��ض���ѡ��
                    testLiveCell((cell/9*9+val-1));//����Ƿ�ɼ�������
                    if(failed==true)
                    {
                        return false;
                    }

                    type=type&(~dig2bit[1]);//���ڲ�������
                    if(boxid[cell]==boxid[(cell/9*9+val-1)])
                    {
                        type=type&(~dig2bit[3]);//��ͬһ���ڲ����¹�
                    }
                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[values[cell]];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type((cell/9*9+val-1),type_val);    //���������
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
            if(numLivColDig[cell%9][values[cell]-1]!=0)//��������ѡ��
            {
                while(bitLivColDig[cell%9][values[cell]-1]>0)
                {
                    val = bit2dig[(bitLivColDig[cell%9][values[cell]-1] & ~ (bitLivColDig[cell%9][values[cell]-1] -1))];// ��ȡ���Ҷ˵� 1���������
                    type=remCellLive(((val-1)*9+cell%9),values[cell]);//�����������λ�õ��ض���ѡ��
                    testLiveCell(((val-1)*9+cell%9));//����Ƿ�ɼ�������
                    if(failed==true)
                    {
                        return false;
                    }

                    type=type&(~dig2bit[2]);//���ڲ�������
                    if(boxid[cell]==boxid[((val-1)*9+cell%9)])
                    {
                        type=type&(~dig2bit[3]);//��ͬһ���ڲ����¹�
                    }
                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[values[cell]];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(((val-1)*9+cell%9),type_val);    //���������
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
            if(numLivBoxDig[boxid[cell]][values[cell]-1]!=0)//��������ѡ��
            {

                while(bitLivBoxDig[boxid[cell]][values[cell]-1]>0)
                {
                    val = bit2dig[(bitLivBoxDig[boxid[cell]][values[cell]-1] & ~ (bitLivBoxDig[boxid[cell]][values[cell]-1] -1))];// ��ȡ���Ҷ˵� 1���������
                    type=remCellLive((boxcell[boxid[cell]][val-1]),values[cell]);//�����ù����λ�õ��ض���ѡ��
                    testLiveCell((boxcell[boxid[cell]][val-1]));
                    if(failed==true)
                    {
                        return false;
                    }

                    type=type&(~dig2bit[3]);//���ڲ����¹�
                    if(cell/9==(boxcell[boxid[cell]][val-1])/9)
                    {
                        type=type&(~dig2bit[1]);//��ͬһ���ڲ�������
                    }
                    if(cell%9==(boxcell[boxid[cell]][val-1])%9)
                    {
                        type=type&(~dig2bit[2]);//��ͬһ���ڲ�������
                    }
                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[values[cell]];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type((boxcell[boxid[cell]][val-1]),type_val);    //���������
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

    bool testLiveCell_force_in(int cell,int v)//ǿ�Ƽ���
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
        b=(bitLiveCell[cell] & ~dig2bit[v]);//�ں�ѡ������ʱ�Ƴ�����
        while(b>0)
        {
            val = bit2dig[b & ~ (b -1)];// ��ȡ���Ҷ˵� 1���������
            type=remCellLive(cell,val);//�������λ�õ��ض���ѡ��
            b = b & (b -1);// ɾ�����Ҷ˵� 1

            while(type>0)//����type�����Ƿ���Ҫ����
            {
                tag=true;
                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                type = type & (type -1);
            }

        }
        bitLiveCell[cell]=dig2bit[v];//���ָ��˺�ѡ��
        testLiveCell(cell);// ���������Ԫ���Ƿ����ȷ��
        if(failed==true)
        {
            return false;
        }

        if(tag)
        {
            check_hidden1_type(cell,type_val);    //���������
            if(failed==true)
            {
                return false;
            }
        }
        return true;
    }

    bool check_hidden1_type(int cell,int* type_val)//����Ƿ�����º�ѡ����������ʽ1
    {
        int num,pos;
        bool flag=false;
        while(type_val[0]>0)//�и���
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
        while(type_val[1]>0)//�и���
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
        while(type_val[2]>0)//������
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

    bool fixUniqueNumber()//Ψһ������
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

    bool hiddenMultiples1()//����ռλ��1
    {
        int cell;
        for(int i = 0; i < 9; i++)
        {
            for(int j = 0; j < 9; j++)
            {
                if(numLivRowDig[i][j]==1)
                {

                    cell=i*9+bit2dig[bitLivRowDig[i][j]]-1;//ȷ��������1λ��
                    testLiveCell_force_in(cell,j+1);//�������
                    if(failed==true)
                    {
                        return false;
                    }

                }
                if(numLivColDig[i][j]==1)
                {

                    cell=i+(bit2dig[bitLivColDig[i][j]]-1)*9;//ȷ��������1λ��
                    testLiveCell_force_in(cell,j+1);//�������
                    if(failed==true)
                    {
                        return false;
                    }

                }
                if(numLivBoxDig[i][j]==1)
                {

                    cell=boxcell[i][bit2dig[bitLivBoxDig[i][j]]-1];//ȷ��������1λ��
                    testLiveCell_force_in(cell,j+1);//�������
                    if(failed==true)
                    {
                        return false;
                    }
                }

            }


        }

        return true;
    }

    bool hiddenMultiples2()//����ռλ��2
    {
        int ncons=2,b,col1,col2,k,val,type,type_val[3]= {0}; //����hiddenֵ
        bool tag = false;
        int cell[ncons]= {0},delbit[ncons]= {0};

        for(int n=0; n<9; n++) //�������й�����
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
                                col1 = bit2dig[(b & ~(b-1))]-1;//��ȡ���Ҷ˵�1
                                b = b & (b-1);//ɾ�����Ҷ˵�1
                                col2 = bit2dig[(b & ~(b-1))]-1;//��ȡ���Ҷ˵�1
                                cell[0]=n*9+col1;//ȷ��������Ԫ������
                                cell[1]=n*9+col2;
                                delbit[0]=dig2bit[v1+1];
                                delbit[1]=dig2bit[v2+1];
                                for(k=0; k<ncons; k++)
                                {
                                    if(numLiveCell[cell[k]]>2)//���˸��ѡ���������ڶ�
                                    {

                                        b=(bitLiveCell[cell[k]] & ~(dig2bit[v1+1]|dig2bit[v2+1]));//�ں�ѡ������ʱ�Ƴ���2��
                                        while(b>0)
                                        {
                                            val = bit2dig[b & ~ (b -1)];// ��ȡ���Ҷ˵� 1���������
                                            type=remCellLive(cell[k],val);//�������λ�õ��ض���ѡ��
                                            b = b & (b -1);// ɾ�����Ҷ˵� 1
                                            while(type>0)//����type�����Ƿ���Ҫ����
                                            {
                                                tag=true;
                                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                                                type = type & (type -1);
                                            }
                                        }
                                        bitLiveCell[cell[k]]=(dig2bit[v1+1]|dig2bit[v2+1]);//���ָ���2��ѡ��
                                        if(tag)
                                        {
                                            check_hidden1_type(cell[k],type_val);    //���������
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
                                    nakedMultiples(cell,delbit,2);    //��������������Ƿ������ʽ1
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
                                col1 = bit2dig[(b & ~(b-1))]-1;//��ȡ���Ҷ˵�1
                                b = b & (b-1);//ɾ�����Ҷ˵�1
                                col2 = bit2dig[(b & ~(b-1))]-1;//��ȡ���Ҷ˵�1
                                cell[0]=n+col1*9;//ȷ��������Ԫ������
                                cell[1]=n+col2*9;
                                delbit[0]=dig2bit[v1+1];
                                delbit[1]=dig2bit[v2+1];
                                // cout<<"hidden 2 find in "<<n+1<<" col :"<<col1+1<<" row and "<<col2+1<<" row ::"<<" value are "<<bit2dig[delbit[0]]<<" and "<<bit2dig[delbit[1]]<<endl;
                                for(k=0; k<ncons; k++)
                                {
                                    if(numLiveCell[cell[k]]>2)//���˸��ѡ���������ڶ�
                                    {

                                        b=(bitLiveCell[cell[k]] & ~(dig2bit[v1+1]|dig2bit[v2+1]));//�ں�ѡ������ʱ�Ƴ���2��
                                        while(b>0)
                                        {
                                            val = bit2dig[b & ~ (b -1)];// ��ȡ���Ҷ˵� 1���������
                                            type=remCellLive(cell[k],val);//�������λ�õ��ض���ѡ��
                                            //cout<<"�Ƴ���ѡ����"<<cell[k]/9+1<<" row "<<cell[k]%9+1<<" col, value is "<<val<<endl;
                                            b = b & (b -1);// ɾ�����Ҷ˵� 1
                                            while(type>0)//����type�����Ƿ���Ҫ����
                                            {
                                                tag=true;
                                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                                                type = type & (type -1);
                                            }
                                        }
                                        bitLiveCell[cell[k]]=(dig2bit[v1+1]|dig2bit[v2+1]);//���ָ���2��ѡ��
                                        if(tag)
                                        {
                                            check_hidden1_type(cell[k],type_val);    //���������
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
                                    nakedMultiples(cell,delbit,2);    //��������������Ƿ������ʽ1
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
                                col1 = bit2dig[(b & ~(b-1))]-1;//��ȡ���Ҷ˵�1
                                b = b & (b-1);//ɾ�����Ҷ˵�1
                                col2 = bit2dig[(b & ~(b-1))]-1;//��ȡ���Ҷ˵�1
                                cell[0]=boxcell[n][col1];//ȷ��������Ԫ������
                                cell[1]=boxcell[n][col2];
                                delbit[0]=dig2bit[v1+1];
                                delbit[1]=dig2bit[v2+1];
                                //cout<<"hidden 2 find in "<<n+1<<" box :"<<col1+1<<" pos and "<<col2+1<<" pos ::"<<" value are "<<bit2dig[delbit[0]]<<" and "<<bit2dig[delbit[1]]<<endl;
                                for(k=0; k<ncons; k++)
                                {
                                    if(numLiveCell[cell[k]]>2)//���˸��ѡ���������ڶ�
                                    {

                                        b=(bitLiveCell[cell[k]] & ~(dig2bit[v1+1]|dig2bit[v2+1]));//�ں�ѡ������ʱ�Ƴ���2��
                                        while(b>0)
                                        {
                                            val = bit2dig[b & ~ (b -1)];// ��ȡ���Ҷ˵� 1���������
                                            type=remCellLive(cell[k],val);//�������λ�õ��ض���ѡ��
                                            //cout<<"�Ƴ���ѡ����"<<cell[k]/9+1<<" row "<<cell[k]%9+1<<" col, value is "<<val<<endl;
                                            b = b & (b -1);// ɾ�����Ҷ˵� 1
                                            while(type>0)//����type�����Ƿ���Ҫ����
                                            {
                                                tag=true;
                                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                                                type = type & (type -1);
                                            }
                                        }
                                        bitLiveCell[cell[k]]=(dig2bit[v1+1]|dig2bit[v2+1]);//���ָ���2��ѡ��
                                        if(tag)
                                        {
                                            check_hidden1_type(cell[k],type_val);    //���������
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
                                    nakedMultiples(cell,delbit,2);    //��������������Ƿ������ʽ1
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

    bool nakedMultiples(int *ijk,int *delbit,int ncons)//��ʾ2,3ͨ�÷���
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
            colbit |= dig2bit[ijk[i]%9+1];// �ϲ���Ԫ���Ӧ����
            rowbit |= dig2bit[ijk[i]/9+1];// �ϲ���Ԫ���Ӧ����
            boxbit |= dig2bit[cellbox[ijk[i]]+1];// �ϲ���Ԫ���Ӧ�Ĺ�
        }

        if(row!=0)//����ͬһ��
        {

            for(c =0; c <9; c ++ ) // ѭ����,���к�ѡ��ɾ��
            {
                if(dig2bit[c+1] & colbit) continue;// ���б�ռ��
                cell = (bit2dig[row]-1)*9 + c; // ���㵥Ԫ����
                if(fixedCell[cell])continue;// �����Ԫ���Ѿ�ȷ��
                for(k =0; k < ncons; k ++ ) // ѭ��ɾ����ѡ��
                {
                    if(bitLiveCell[cell]&delbit[k]) //�����Ԫ���������ѡ��
                    {
                        type=remCellLive(cell,bit2dig[delbit[k]]);// ɾ����ѡ��
                        testLiveCell(cell);// ���������Ԫ���Ƿ����ȷ��
                        if(failed==true)
                        {
                            return false;
                        }
                        while(type>0)//����type�����Ƿ���Ҫ����
                        {
                            tag=true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=delbit[k];
                            type = type & (type -1);
                        }
                    }
                }
                if(tag)
                {
                    check_hidden1_type(cell,type_val);    //���������
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
        if(col!=0)//����ͬһ��
        {
            for(c =0; c <9; c ++ ) // ѭ����,���к�ѡ��ɾ��
            {
                if(dig2bit[c+1] & rowbit) continue;// ���б�ռ��
                cell = bit2dig[col]-1 + c*9; // ���㵥Ԫ����
                if(fixedCell[cell])continue;// �����Ԫ���Ѿ�ȷ��
                for(k =0; k < ncons; k ++ ) // ѭ��ɾ����ѡ��
                {
                    if(bitLiveCell[cell]&delbit[k]) //�����Ԫ���������ѡ��
                    {
                        type=remCellLive(cell,bit2dig[delbit[k]]);// ɾ����ѡ��
                        testLiveCell(cell);// ���������Ԫ���Ƿ����ȷ��
                        if(failed==true)
                        {
                            return false;
                        }
                        while(type>0)//����type�����Ƿ���Ҫ����
                        {
                            tag=true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=delbit[k];
                            type = type & (type -1);
                        }
                    }
                }
                if(tag)
                {
                    check_hidden1_type(cell,type_val);    //���������
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
        if(box!=0)//����ͬһ��
        {
            for(c =0; c <9; c ++ ) // ѭ��λ��,���к�ѡ��ɾ��
            {
                if(dig2bit[c+1] & boxbit) continue;// ��λ�ñ�ռ��
                cell = boxcell[bit2dig[box]-1][c]; // ���㵥Ԫ����
                if(fixedCell[cell])continue;// �����Ԫ���Ѿ�ȷ��
                for(k =0; k < ncons; k ++ ) // ѭ��ɾ����ѡ��
                {
                    if(bitLiveCell[cell]&delbit[k]) //�����Ԫ���������ѡ��
                    {
                        type=remCellLive(cell,bit2dig[delbit[k]]);// ɾ����ѡ��
                        testLiveCell(cell);// ���������Ԫ���Ƿ����ȷ��
                        if(failed==true)
                        {
                            return false;
                        }
                        while(type>0)//����type�����Ƿ���Ҫ����
                        {
                            tag=true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=delbit[k];
                            type = type & (type -1);
                        }
                    }
                }
                if(tag)
                {
                    check_hidden1_type(cell,type_val);    //���������
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
                    continue;   //�����κ���ͬ������
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
                        continue;   //�����κ���ͬ������
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
            for(int v=0; v<9; v++) //�������й�����
            {
                if(numLivRowDig[i][v]==2)//���д���ĳ��ֻ������λ��
                {
                    b=bitLivRowDig[i][v];//�ҵ��ֲ�
                    cell1=bit2dig[b&(~(b-1))]-1+i*9;
                    b=b&(b-1);
                    cell2=bit2dig[b&(~(b-1))]-1+i*9;
                    if(boxid[cell1]==boxid[cell2] && numLivBoxDig[boxid[cell1]][v]>2)
                    {
                        box=boxid[cell1];
                        b=bitLivBoxDig[box][v]&~(dig2bit[cellbox[cell1]+1]|dig2bit[cellbox[cell2]+1]);//ȥ��������λ�ú��ʣ���ѡ���ֲ�
                        while(b>0)
                        {
                            cell=boxcell[box][bit2dig[b&(~(b-1))]-1];//��ȡ���Ҷ�1����ĵ�Ԫ��
                            type=remCellLive(cell,v+1);  //�Ƴ���ѡ��
                            testLiveCell(cell);    //����Ƿ��ȷ��
                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //ɾ���Ҷ�һ
                            while(type>0)//����type�����Ƿ���Ҫ����
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //���������
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
                }//�в���2
                if(numLivRowDig[i][v]==3)//���д���ĳ��ֻ������λ��
                {
                    b=bitLivRowDig[i][v];//�ҵ��ֲ�
                    cell1=bit2dig[b&(~(b-1))]-1+i*9;
                    b=b&(b-1);
                    cell2=bit2dig[b&(~(b-1))]-1+i*9;
                    b=b&(b-1);
                    cell3=bit2dig[b&(~(b-1))]-1+i*9;

                    if(boxid[cell1]==boxid[cell2] && boxid[cell3]==boxid[cell2] && numLivBoxDig[boxid[cell1]][v]>3)
                    {
                        box=boxid[cell1];
                        b=bitLivBoxDig[box][v]&~(dig2bit[cellbox[cell1]+1]|dig2bit[cellbox[cell2]+1]|dig2bit[cellbox[cell3]+1]);//ȥ��������λ�ú��ʣ���ѡ���ֲ�
                        while(b>0)
                        {
                            cell=boxcell[box][bit2dig[b&(~(b-1))]-1];//��ȡ���Ҷ�1����ĵ�Ԫ��
                            type=remCellLive(cell,v+1);  //�Ƴ���ѡ��
                            testLiveCell(cell);    //����Ƿ��ȷ��

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //ɾ���Ҷ�һ
                            while(type>0)//����type�����Ƿ���Ҫ����
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //���������
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
                }//�в���3
                if(numLivColDig[i][v]==2)//���д���ĳ��ֻ������λ��
                {
                    b=bitLivColDig[i][v];//�ҵ��ֲ�
                    cell1=(bit2dig[b&(~(b-1))]-1)*9+i;
                    b=b&(b-1);
                    cell2=(bit2dig[b&(~(b-1))]-1)*9+i;
                    if(boxid[cell1]==boxid[cell2] && numLivBoxDig[boxid[cell1]][v]>2)
                    {
                        box=boxid[cell1];
                        b=bitLivBoxDig[box][v]&~(dig2bit[cellbox[cell1]+1]|dig2bit[cellbox[cell2]+1]);//ȥ��������λ�ú��ʣ���ѡ���ֲ�
                        while(b>0)
                        {
                            cell=boxcell[box][bit2dig[b&(~(b-1))]-1];//��ȡ���Ҷ�1����ĵ�Ԫ��
                            type=remCellLive(cell,v+1);  //�Ƴ���ѡ��
                            testLiveCell(cell);    //����Ƿ��ȷ��

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //ɾ���Ҷ�һ
                            while(type>0)//����type�����Ƿ���Ҫ����
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //���������
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
                }//�в���2
                if(numLivColDig[i][v]==3)//���д���ĳ��ֻ������λ��
                {
                    b=bitLivColDig[i][v];//�ҵ��ֲ�
                    cell1=(bit2dig[b&(~(b-1))]-1)*9+i;
                    b=b&(b-1);
                    cell2=(bit2dig[b&(~(b-1))]-1)*9+i;
                    b=b&(b-1);
                    cell3=(bit2dig[b&(~(b-1))]-1)*9+i;
                    if(boxid[cell1]==boxid[cell2] && boxid[cell3]==boxid[cell2] && numLivBoxDig[boxid[cell1]][v]>3)
                    {
                        box=boxid[cell1];
                        b=bitLivBoxDig[box][v]&~(dig2bit[cellbox[cell1]+1]|dig2bit[cellbox[cell2]+1]|dig2bit[cellbox[cell3]+1]);//ȥ��������λ�ú��ʣ���ѡ���ֲ�
                        while(b>0)
                        {
                            cell=boxcell[box][bit2dig[b&(~(b-1))]-1];//��ȡ���Ҷ�1����ĵ�Ԫ��
                            type=remCellLive(cell,v+1);  //�Ƴ���ѡ��
                            testLiveCell(cell);    //����Ƿ��ȷ��

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //ɾ���Ҷ�һ
                            while(type>0)//����type�����Ƿ���Ҫ����
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //���������
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
                }//�в���3

                if(numLivBoxDig[i][v]==2)//�˹�����ĳ��ֻ������λ��
                {
                    b=bitLivBoxDig[i][v];//�ҵ��ֲ�
                    cell1 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    b=b&(b-1);
                    cell2 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    if(cell1/9==cell2/9 && numLivRowDig[cell1/9][v]>2)//ͬ��
                    {
                        row=cell1/9;
                        b=bitLivRowDig[row][v]&~(dig2bit[cell1%9+1]|dig2bit[cell2%9+1]);//ȥ��������λ�ú��ʣ���ѡ���ֲ�
                        while(b>0)
                        {
                            cell=bit2dig[b&(~(b-1))]-1+row*9;//��ȡ���Ҷ�1����ĵ�Ԫ��
                            type=remCellLive(cell,v+1);  //�Ƴ���ѡ��
                            testLiveCell(cell);    //����Ƿ��ȷ��

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //ɾ���Ҷ�һ
                            while(type>0)//����type�����Ƿ���Ҫ����
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //���������
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

                    if(cell1%9==cell2%9 && numLivColDig[cell1%9][v]>2)//ͬ��
                    {
                        col=cell1%9;
                        b=bitLivColDig[col][v]&~(dig2bit[cell1/9+1]|dig2bit[cell2/9+1]);//ȥ��������λ�ú��ʣ���ѡ���ֲ�
                        while(b>0)
                        {
                            cell=(bit2dig[b&(~(b-1))]-1)*9+col;//��ȡ���Ҷ�1����ĵ�Ԫ��
                            type=remCellLive(cell,v+1);  //�Ƴ���ѡ��
                            testLiveCell(cell);    //����Ƿ��ȷ��

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //ɾ���Ҷ�һ
                            while(type>0)//����type�����Ƿ���Ҫ����
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //���������
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


                }//������2

                if(numLivBoxDig[i][v]==3)//�˹�����ĳ��ֻ������λ��
                {
                    b=bitLivBoxDig[i][v];//�ҵ��ֲ�
                    cell1 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    b=b&(b-1);
                    cell2 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    b=b&(b-1);
                    cell3 = boxcell[i][(bit2dig[b&(~(b-1))]-1)];
                    if(cell1/9==cell2/9 && cell3/9==cell2/9 && numLivRowDig[cell1/9][v]>3)//ͬ��
                    {
                        row=cell1/9;
                        b=bitLivRowDig[row][v]&~(dig2bit[cell1%9+1]|dig2bit[cell2%9+1]|dig2bit[cell3%9+1]);//ȥ��������λ�ú��ʣ���ѡ���ֲ�
                        while(b>0)
                        {
                            cell=bit2dig[b&(~(b-1))]-1+row*9;//��ȡ���Ҷ�1����ĵ�Ԫ��
                            type=remCellLive(cell,v+1);  //�Ƴ���ѡ��
                            testLiveCell(cell);    //����Ƿ��ȷ��

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //ɾ���Ҷ�һ
                            while(type>0)//����type�����Ƿ���Ҫ����
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //���������
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

                    if(cell1%9==cell2%9 && cell3%9==cell2%9 && numLivColDig[cell1%9][v]>3)//ͬ��
                    {
                        col=cell1%9;
                        b=bitLivColDig[col][v]&~(dig2bit[cell1/9+1]|dig2bit[cell2/9+1]|dig2bit[cell3/9+1]);//ȥ��������λ�ú��ʣ���ѡ���ֲ�
                        while(b>0)
                        {
                            cell=(bit2dig[b&(~(b-1))]-1)*9+col;//��ȡ���Ҷ�1����ĵ�Ԫ��
                            type=remCellLive(cell,v+1);  //�Ƴ���ѡ��
                            testLiveCell(cell);    //����Ƿ��ȷ��

                            if(failed==true)
                            {
                                return false;
                            }

                            b=b&(b-1);   //ɾ���Ҷ�һ
                            while(type>0)//����type�����Ƿ���Ҫ����
                            {
                                tag=true;
                                type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[v+1];
                                type = type & (type -1);
                            }
                            if(tag)
                            {
                                check_hidden1_type(cell,type_val);    //���������
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


                }//������3

            }

        }

        return true;

    }//�����ų���

    bool uniquesquare()
    {
        int cell1,cell2;
        for(int i=0; i<8; i++) //�������й���
        {
            for(int j=i+1; j<9; j++)
            {
                if(i/3==j/3)//����ͬ��
                {
                    for(int c=0; c<3; c++) //����ÿ����3��
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
                                //i��
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
                                //j��

                            }
                        }

                    }
                }

                if(i%3==j%3)//����ͬ��
                {
                    for(int c=0; c<3; c++) //����ÿ����3��
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
                                //i��
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
                                //j��

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

        if(cell1/9 == cell2/9)//����ͬ��
        {
            first = cell1%9;
            second = cell2%9;
            bigrow = box/3;

            posofcell= dig2bit[first+1]|dig2bit[second+1];

            for(int i = 0; i<3; i++ ) //��һ��
            {
                temp_cell = (bigrow*3+i)*9 + first;
                if(bitLiveCell[cell1] == bitLiveCell[temp_cell])
                {
                    another_cell = (bigrow*3+i)*9 + second;
                    b = bitLiveCell[cell1];
                    while(b > 0)
                    {
                        val = bit2dig[b & ~ (b -1)];// ��ȡ���Ҷ˵� 1���������
                        type = remCellLive(another_cell,val);//�������λ�õ��ض���ѡ��
                        if(failed==true)
                        {
                            return false;
                        }
                        b = b & (b -1);// ɾ�����Ҷ˵� 1
                        while(type>0)//����type�����Ƿ���Ҫ����
                        {
                            tag = true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                            type = type & (type -1);
                        }
                    }
                    testLiveCell(another_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //���������
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

            for(int i = 0; i<3; i++ ) //�ڶ���
            {
                temp_cell = (bigrow*3+i)*9 + second;
                if(bitLiveCell[cell1] == bitLiveCell[temp_cell])
                {
                    another_cell = (bigrow*3+i)*9 + first;
                    b = bitLiveCell[cell1];
                    while(b > 0)
                    {
                        val = bit2dig[b & ~ (b -1)];// ��ȡ���Ҷ˵� 1���������
                        type = remCellLive(another_cell,val);//�������λ�õ��ض���ѡ��
                        b = b & (b -1);// ɾ�����Ҷ˵� 1
                        while(type>0)//����type�����Ƿ���Ҫ����
                        {
                            tag = true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                            type = type & (type -1);
                        }
                    }
                    testLiveCell(another_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //���������
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
                if(bitLivRowDig[bigrow*3+i][waitnumber1-1] == posofcell && (bitLivRowDig[bigrow*3+i][waitnumber2-1] & posofcell)!=0 )//��һ��ѡ������ڶ�����
                {
                    temp_cell = (bigrow*3+i)*9 + first;
                    another_cell = (bigrow*3+i)*9 + second;
                    //����һ����
                    type = remCellLive(temp_cell,waitnumber2);//�������λ�õ��ض���ѡ��
                    testLiveCell(temp_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber2];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(temp_cell,type_val);    //���������
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                    //���ڶ�����
                    type = remCellLive(another_cell,waitnumber2);//�������λ�õ��ض���ѡ��
                    testLiveCell(another_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber2];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //���������
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

                if(bitLivRowDig[bigrow*3+i][waitnumber2-1] == posofcell && (bitLivRowDig[bigrow*3+i][waitnumber1-1] & posofcell)!=0 )//�ڶ���ѡ�������һ����
                {
                    temp_cell = (bigrow*3+i)*9 + first;
                    another_cell = (bigrow*3+i)*9 + second;
                    //����һ����
                    type = remCellLive(temp_cell,waitnumber1);//�������λ�õ��ض���ѡ��
                    testLiveCell(temp_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber1];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(temp_cell,type_val);    //���������
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                    //���ڶ�����
                    type = remCellLive(another_cell,waitnumber1);//�������λ�õ��ض���ѡ��
                    testLiveCell(another_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber1];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //���������
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

        if(cell1%9 == cell2%9 )//����ͬ��
        {

            first = cell1/9;
            second = cell2/9;
            bigcol = box%3;
            posofcell= dig2bit[first+1]|dig2bit[second+1];

            for(int i = 0; i<3; i++ ) //��һ��
            {
                temp_cell = bigcol*3+i + first*9;
                if(bitLiveCell[cell1] == bitLiveCell[temp_cell])
                {
                    another_cell = bigcol*3+i + second*9;
                    b = bitLiveCell[cell1];
                    while(b > 0)
                    {
                        val = bit2dig[b & ~ (b -1)];// ��ȡ���Ҷ˵� 1���������
                        type = remCellLive(another_cell,val);//�������λ�õ��ض���ѡ��
                        b = b & (b -1);// ɾ�����Ҷ˵� 1
                        while(type>0)//����type�����Ƿ���Ҫ����
                        {
                            tag = true;
                            type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[val];
                            type = type & (type -1);
                        }
                    }
                    testLiveCell(another_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //���������
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

            for(int i = 0; i<3; i++ ) //�ڶ���
            {
                temp_cell = bigcol*3+i + second*9;
                if(bitLiveCell[cell1] == bitLiveCell[temp_cell])
                {
                    another_cell = bigcol*3+i + first*9;
                    b = bitLiveCell[cell1];
                    while(b > 0)
                    {
                        val = bit2dig[b & ~ (b -1)];// ��ȡ���Ҷ˵� 1���������
                        type = remCellLive(another_cell,val);//�������λ�õ��ض���ѡ��
                        b = b & (b -1);// ɾ�����Ҷ˵� 1
                        while(type>0)//����type�����Ƿ���Ҫ����
                        {
                            tag = true;
                            type_val[bit2dig[type & ~ (type -1)]-1] |= dig2bit[val];
                            type = type & (type -1);
                        }
                    }
                    testLiveCell(another_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //���������
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
                if(bitLivColDig[bigcol*3+i][waitnumber1-1] == posofcell && (bitLivColDig[bigcol*3+i][waitnumber2-1] & posofcell)!=0 )//��һ��ѡ������ڶ���ѡ������
                {
                    temp_cell = bigcol*3+i + first*9;
                    another_cell = bigcol*3+i + second*9;
                    //����һ����
                    type = remCellLive(temp_cell,waitnumber2);//�������λ�õ��ض���ѡ��
                    testLiveCell(temp_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber2];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(temp_cell,type_val);    //���������
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                    //���ڶ�����
                    type = remCellLive(another_cell,waitnumber2);//�������λ�õ��ض���ѡ��
                    testLiveCell(another_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber2];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //���������
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

                if(bitLivColDig[bigcol*3+i][waitnumber2-1] == posofcell && (bitLivColDig[bigcol*3+i][waitnumber1-1] & posofcell)!=0 )//�ڶ���ѡ������,��һ��ѡ������
                {
                    temp_cell =  bigcol*3+i + first*9;
                    another_cell = bigcol*3+i + second*9;
                    //����һ����
                    type = remCellLive(temp_cell,waitnumber1);//�������λ�õ��ض���ѡ��
                    testLiveCell(temp_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber1];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(temp_cell,type_val);    //���������
                        if(failed==true)
                        {
                            return false;
                        }
                    }
                    tag=false;
                    type_val[0]=0;
                    type_val[1]=0;
                    type_val[2]=0;

                    //���ڶ�����
                    type = remCellLive(another_cell,waitnumber1);//�������λ�õ��ض���ѡ��
                    testLiveCell(another_cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }

                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag = true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[waitnumber1];
                        type = type & (type -1);
                    }
                    if(tag)
                    {
                        check_hidden1_type(another_cell,type_val);    //���������
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
        for(int i=0; i<9; i++) //һ���ŵ�����
        {
            if(fixedNumber[i] <= 5)
            {
                for(int row=0; row<8; row++) //�ж�����
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

                for(int col=0; col<8; col++) //�ж�����
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

        if(t==1)//�ж��壬��ɾ��
        {
            if(numLivColDig[delete1-1][value-1]>2)
            {
                deletefirst=true;
            }
            if(numLivColDig[delete2-1][value-1]>2)
            {
                deletesecond=true;
            }

            if(deletefirst)//��һɾ����
            {
                b = bitLivColDig[delete1-1][value-1];
                b = b & ~dig2bit[first+1] & ~dig2bit[second+1];
                while(b>0)
                {
                    cell = (bit2dig[b & ~ (b -1)]-1)*9 + delete1-1;
                    b = b & (b -1);
                    type=remCellLive(cell,value);// ɾ����ѡ��
                    testLiveCell(cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }
                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[value];
                        type = type & (type -1);
                    }

                    if(tag)
                    {
                        check_hidden1_type(cell,type_val);    //���������
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

            if(deletesecond)//�ڶ�ɾ����
            {
                b = bitLivColDig[delete2-1][value-1];
                b = b & ~dig2bit[first+1] & ~dig2bit[second+1];
                while(b>0)
                {
                    cell = (bit2dig[b & ~ (b -1)]-1)*9 + delete2-1;
                    b = b & (b -1);
                    type=remCellLive(cell,value);// ɾ����ѡ��
                    testLiveCell(cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }
                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[value];
                        type = type & (type -1);
                    }

                    if(tag)
                    {
                        check_hidden1_type(cell,type_val);    //���������
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

        if(t==2) //�ж��壬��ɾ��
        {
            if(numLivRowDig[delete1-1][value-1]>2)
            {
                deletefirst=true;
            }
            if(numLivRowDig[delete2-1][value-1]>2)
            {
                deletesecond=true;
            }

            if(deletefirst)//��һɾ����
            {
                b = bitLivRowDig[delete1-1][value-1];
                b = b & ~dig2bit[first+1] & ~dig2bit[second+1];
                while(b>0)
                {
                    cell = (bit2dig[b & ~ (b -1)]-1) + (delete1-1)*9;
                    b = b & (b -1);
                    type=remCellLive(cell,value);// ɾ����ѡ��
                    testLiveCell(cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }
                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[value];
                        type = type & (type -1);
                    }

                    if(tag)
                    {
                        check_hidden1_type(cell,type_val);    //���������
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

            if(deletesecond)//�ڶ�ɾ����
            {
                b = bitLivRowDig[delete2-1][value-1];
                b = b & ~dig2bit[first+1] & ~dig2bit[second+1];
                while(b>0)
                {
                    cell = (bit2dig[b & ~ (b -1)]-1) + (delete2-1)*9;
                    b = b & (b -1);
                    type=remCellLive(cell,value);// ɾ����ѡ��
                    testLiveCell(cell);// ���������Ԫ���Ƿ����ȷ��
                    if(failed==true)
                    {
                        return false;
                    }
                    while(type>0)//����type�����Ƿ���Ҫ����
                    {
                        tag=true;
                        type_val[bit2dig[type & ~ (type -1)]-1]|=dig2bit[value];
                        type = type & (type -1);
                    }

                    if(tag)
                    {
                        check_hidden1_type(cell,type_val);    //���������
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

    bool dfs(int x,int y)//������дdfs
    {
//ά����������
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

        int b=bitLiveCell[cell];//���ú�ѡ��
        Suduko p=*this;
        while (b>0)
        {
            int m=b&(~(b-1));//ȡ���Ҷ˵�1
            testLiveCell_force_in(cell,bit2dig[m]);
            change = true;
            while(failed==false && change==true)
            {
                //�������+
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
                        if (dfs(x,y+1)) return true;    //����1
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

    bool opt_dfs()//ѡ������дdfs(��ѡ�����ٵ�cell)
    {
        int minnum=10,cell;
//ά����������
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

        int b=bitLiveCell[cell];//���ú�ѡ��
        Suduko p=*this;
        while (b>0)
        {
            int m=b&(~(b-1));//ȡ���Ҷ˵�1
            testLiveCell_force_in(cell,bit2dig[m]);
            change = true;
            if(failed==false && change==true) //�������+ �ɽ�if����Ϊwhile
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
                    if (opt_dfs()) return true;  //����1

                }
            }


            *this=p;
            b=b&(b-1);
        }

        deep=deep-1;
        return false;
    }

    bool opt2_dfs()//ѡ������дdfs����ѡ������cell��ȫ����ȷ����������ࣩ
    {
        int minnum=10,cell,choice,m,max_choice,max_m;
//ά����������
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

        int b=bitLiveCell[cell];//���ú�ѡ��
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
            if(failed==false && change==true) //�������+ �ɽ�if����Ϊwhile
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
                    if (opt2_dfs()) return true;  //����1

                }
            }


            *this=p;
            b=b-max_m;
        }
        deep=deep-1;
        return false;
    }

    bool opt3_dfs()//��������˳����д
    {
        int maxnum=-1,number,row=0;
//ά����������

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
            int m=b&(~(b-1));//ȡ���Ҷ˵�1
            testLiveCell_force_in(row*9+bit2dig[m]-1,number+1);
            change = true;
            if(failed==false && change==true) //�������+ �ɽ�if����Ϊwhile
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
                    if (opt3_dfs()) return true;  //����1
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
            //Ѱ����С��ѡ������
            for(int i=0; i<81; i++)
            {
                if(fixedCell[i])continue;
                if(minnum > numLiveCell[i])
                {
                    minnum=numLiveCell[i];
                    cell=i;
                }
            }

            int b=bitLiveCell[cell];//���ú�ѡ��
            Suduko p=*this;

            while (b>0)
            {
                int m=b&(~(b-1));//ȡ���Ҷ˵�1
                b=b&(b-1);
                testLiveCell_force_in(cell,bit2dig[m]);

                change = true;
                while(failed==false && change==true) //�������+ �ɽ�if����Ϊwhile
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

//������һ״̬
                *this=p;

            }

            q.pop();

        }

        return false;

    }

    bool opt_bfs() //ѡȡʣ�෽�����ٵ����ȼ���
    {
        vector <Suduko> q;
        q.push_back(*this);

        int minnum=10,cell;
        while(q.size()!=0)
        {
            sort(q.begin(),q.end(),cmp);
            *this=q[0];
            //cout<<"���鳤�ȣ�"<<q.size()<<"ȷ�����ָ�����"<<numFixed<<endl;
            if(numFixed==81)
            {
                return true;
            }

            minnum=10;
            //Ѱ����С��ѡ������
            for(int i=0; i<81; i++)
            {
                if(fixedCell[i])continue;
                if(minnum>numLiveCell[i])
                {
                    minnum=numLiveCell[i];
                    cell=i;
                }
            }

            int b=bitLiveCell[cell];//���ú�ѡ��
            Suduko p=*this;

            while (b>0)
            {
                int m=b&(~(b-1));//ȡ���Ҷ˵�1
                b=b&(b-1);
                testLiveCell_force_in(cell,bit2dig[m]);

                change = true;
                while(failed==false && change==true) //�������+ �ɽ�if����Ϊwhile
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

//������һ״̬
                *this=p;

            }

            q.erase(q.begin(),q.begin()+1);

        }

        return false;

    }

    bool opt2_bfs() //ѡȡʣ�෽�����ٵ����ȼ��� priority queue
    {
        priority_queue<Suduko,vector<Suduko>,cmp2> q;
        q.push(*this);

        int minnum=10,cell;
        while(q.size()!=0)
        {
            *this=q.top();
            q.pop();
            //cout<<"���鳤�ȣ�"<<q.size()<<"ȷ�����ָ�����"<<numFixed<<endl;
            if(numFixed==81)
            {
                return true;
            }

            minnum=10;
            //Ѱ����С��ѡ������
            for(int i=0; i<81; i++)
            {
                if(fixedCell[i])continue;
                if(minnum>numLiveCell[i])
                {
                    minnum=numLiveCell[i];
                    cell=i;
                }
            }

            int b=bitLiveCell[cell];//���ú�ѡ��
            Suduko p=*this;

            while (b>0)
            {
                int m=b&(~(b-1));//ȡ���Ҷ˵�1
                b=b&(b-1);
                testLiveCell_force_in(cell,bit2dig[m]);

                change = true;
                while(failed==false && change==true) //�������+ �ɽ�if����Ϊwhile
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

//������һ״̬
                *this=p;

            }


        }

        return false;

    }

    static bool cmp(Suduko a,Suduko b)
    {
        return a.numFixed>b.numFixed;//���ֵ����>
    }

    struct cmp2
    {
    public:
        bool operator ()(Suduko a,Suduko b) const
        {
            return a.numFixed < b.numFixed;//���ֵ����<
        }
    };

};

int main()
{
    //��ʼ��ȫ������
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
        int counts=0;//����1�ĸ���
        int i=j;
        while(i!=0)
        {
            i=((i-1)&i);//�κζ���������1����ԭ�����������㣬���ս����Ϊ����������1�ĸ���
            counts++;
        }
        popbit[j]=counts;
    }

    //���Կ�ʼ
    bool f;
    double sumstate=0, sumt=0,maxt=0,mint=500.00000000;

    //�����ļ�λ��
    ifstream infile("D:/test.txt");//���ݼ��б�"Hodoku137.txt" [Hodoku137,beijing91,Su17_49151,extreme15,Andoku500,17Su_diff_500,95_hard,Su17_49151]
    string temp;
    while(getline(infile,temp))
    {
        Suduko s(temp);
        QueryPerformanceFrequency(&nFreq);
        QueryPerformanceCounter(&nBeginTime);//��ʼ��ʱ

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
        f = s.opt_dfs();//opt_dfs �� opt2_dfs���
        //f=s.dfs(0,0);
        QueryPerformanceCounter(&nEndTime);//ֹͣ��ʱ
        if(f)
        {
            //��������� Ӱ��ʱ����� ע��
            //s.print();
        }
        else
        {
            cout<<"�޽�"<<endl;
        }

        time=(double)(nEndTime.QuadPart-nBeginTime.QuadPart)/(double)nFreq.QuadPart;//�������ִ��ʱ�䵥λΪ΢��
        //cout<<" ����ʱ�� ��"<<time*1000<<"ms"<<endl;
        //cout<<" ��� : "<<deep<<endl;
       // cout<<" ״̬ : "<<state<<endl;
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

    cout<<"�����ʱ�䣺"<<maxt*1000<<"ms"<<endl;
    cout<<"�������ʱ�䣺"<<mint*1000<<"ms"<<endl;
    cout<<"������ʱ�䣺"<<sumt*1000<<"ms"<<endl;
    cout<<"��״̬�洢��"<<sumstate<<endl;

    //ˢ��
    int x;
    cin>>x;

    return 0;
}

