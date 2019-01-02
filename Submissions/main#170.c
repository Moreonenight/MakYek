#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2
#define DEPTH 4 

typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef int OPTION;
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define UP_LEFT 4
#define UP_RIGHT 5
#define DOWN_LEFT 6
#define DOWN_RIGHT 7

#define MAX_BYTE 10000
#define MAX_TURN 60
#define INFINITE 1000000

struct Command
{
  int x;
  int y;
  OPTION option;
};

struct Boardlist
{
	int blacklist[16][2];
	int whitelist[16][2];
	int black;
	int white;
};

char buffer[MAX_BYTE] = {0};
char board[BOARD_SIZE][BOARD_SIZE] = {{0}};
int me_flag;
int other_flag;
const int DIR[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };
char virtualboard[DEPTH+1][BOARD_SIZE][BOARD_SIZE]={{{0}}};
struct Boardlist boardlist;
int checklist[DEPTH+1];
struct Command enemycommand[MAX_TURN];
struct Command ourcommand[MAX_TURN];
int privateround;

BOOL isInBound(int x, int y) 
{
	return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

int place(int x, int y, OPTION option, int cur_flag, char placeboard[BOARD_SIZE][BOARD_SIZE]) 
{
    int temp=0;	
    int new_x = x + DIR[option][0];
    int new_y = y + DIR[option][1];	
    placeboard[x][y] = EMPTY;
    placeboard[new_x][new_y] = cur_flag;
    int cur_other_flag = 3 - cur_flag;
    int intervention_dir[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
    for (int i = 0; i < 4; i++) 
	{
        int x1 = new_x + intervention_dir[i][0];
        int y1 = new_y + intervention_dir[i][1];
        int x2 = new_x - intervention_dir[i][0];
        int y2 = new_y - intervention_dir[i][1];
        if (isInBound(x1, y1) && isInBound(x2, y2) && placeboard[x1][y1] == cur_other_flag && placeboard[x2][y2] == cur_other_flag) 
	    {
            placeboard[x1][y1] = cur_flag;
            placeboard[x2][y2] = cur_flag; 
	        temp+=2;
        }
    }
    int custodian_dir[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
    for (int i = 0; i < 8; i++) 
	{
        int x1 = new_x + custodian_dir[i][0];
        int y1 = new_y + custodian_dir[i][1];
        int x2 = new_x + custodian_dir[i][0] * 2;
        int y2 = new_y + custodian_dir[i][1] * 2;
        if (isInBound(x1, y1) && isInBound(x2, y2) && placeboard[x2][y2] == cur_flag && placeboard[x1][y1] == cur_other_flag) 
	    {
            placeboard[x1][y1] = cur_flag;  
	        temp+=1;
        }
    }
    return temp;
}

int abs(int i)
{
	return i>=0?i:-i;
}

int max(int a,int b)
{
	return a>b?a:b;
}

void Listboard(int depth)
{
	boardlist.black=0;
	boardlist.white=0;
	for(int i=0;i<BOARD_SIZE;i++)
	{
		for(int j=0;j<BOARD_SIZE;j++)
		{
			if (virtualboard[depth][i][j]==BLACK)
			{
				boardlist.blacklist[boardlist.black][0]=i;
				boardlist.blacklist[boardlist.black][1]=j;
				(boardlist.black)++;
			}
			if (virtualboard[depth][i][j]==WHITE)
			{
				boardlist.whitelist[boardlist.white][0]=i;
				boardlist.whitelist[boardlist.white][1]=j;
				(boardlist.white)++;
			}
		}
	}			
}
 
int enemydistance(int flag)
{
	int array[16]={0};
	int min=0;
	if (flag==BLACK)
	{		
		for (int j=0;j<boardlist.white;j++)
		{
			array[j]=0;
		    for(int i=0;i<boardlist.black;i++)
		    {
			    array[j]+=max(abs(boardlist.blacklist[i][0]-boardlist.whitelist[j][0]),abs(boardlist.blacklist[i][1]-boardlist.whitelist[j][0]));
		    }
	    }
		min=array[0];
		for (int j=0;j<boardlist.white;j++)
		{
			if (array[j]<min)
			{
				min=array[j];
			}
		}
	}
	if (flag==WHITE)
	{	
		for (int j=0;j<boardlist.black;j++)
		{
			array[j]=0;
		    for(int i=0;i<boardlist.white;i++)
		    {
			    array[j]+=max(abs(boardlist.whitelist[i][0]-boardlist.blacklist[j][0]),abs(boardlist.whitelist[i][1]-boardlist.blacklist[j][0]));
		    }
	    }
		min=array[0];
		for (int j=0;j<boardlist.black;j++)
		{
			if (array[j]<min)
			{
				min=array[j];
			}
		}
	}
	return min;
}

int virtualmark(int depth,int flag)
{
	int tempmark;
	Listboard(depth);
    tempmark=20000*(checklist[1]-checklist[2])+18000*(checklist[3]-checklist[4]);
    tempmark-=10*enemydistance(flag);
    for (int i = 0; i < BOARD_SIZE; i++) 
	    {
            for (int j = 0; j < BOARD_SIZE; j++) 
		    {
				if (virtualboard[depth][i][j]==flag)
				{
					for (int k1 = 7; k1 >= 0; k1--) 
					{
						int i1=i-DIR[k1][0];
						int j1=j-DIR[k1][1];
						int i2=i+DIR[k1][0];
						int j2=j+DIR[k1][1];
						if (virtualboard[depth][i2][j2]==3-flag&&virtualboard[depth][i1][j1]==EMPTY)
						{
							for (int k2 = 7; k2 >= 0; k2--) 
							{
								if (virtualboard[depth][i1+DIR[k2][0]][j1+DIR[k2][1]]==3-flag)
								{
									tempmark-=2500;
									break;
								}
							}
						}
						if (virtualboard[depth][i2][j2]==EMPTY&&virtualboard[depth][i+2*DIR[k1][0]][j+2*DIR[k1][1]]==flag)	
						{
							for (int k2 = 7; k2 >= 0; k2--) 
							{
								if (virtualboard[depth][i2+DIR[k2][0]][j2+DIR[k2][1]]==3-flag)
								{
									tempmark-=5000;
									break;
								}
							}
						}
					}
				}
				if (virtualboard[depth][i][j]==3-flag)
				{
					for (int k1 = 7; k1 >= 0; k1--) 
					{
						int i1=i-DIR[k1][0];
						int j1=j-DIR[k1][1];
						int i2=i+DIR[k1][0];
						int j2=j+DIR[k1][1];
						if (virtualboard[depth][i2][j2]==flag&&virtualboard[depth][i1][j1]==EMPTY)
						{
							for (int k2 = 7; k2 >= 0; k2--) 
							{
								if (virtualboard[depth][i1+DIR[k2][0]][j1+DIR[k2][1]]==flag)
								{
									tempmark+=2500;
									break;
								}
							}
						}
						if (virtualboard[depth][i2][j2]==EMPTY&&virtualboard[depth][i+2*DIR[k1][0]][j+2*DIR[k1][1]]==3-flag)	
						{
							for (int k2 = 7; k2 >= 0; k2--) 
							{
								if (virtualboard[depth][i2+DIR[k2][0]][j2+DIR[k2][1]]==flag)
								{
									tempmark+=5000;
									break;
								}
							}
						}
					}
				}				
		    }
		}					
	return tempmark;
}

int virtualgame(int flag,int x,int y,int option,int alpha,int beta,int depth)
{
    for (int i = 0; i < BOARD_SIZE; i++) 
    {
        for (int j = 0; j < BOARD_SIZE; j++) 
	    {
		    virtualboard[depth][i][j]=virtualboard[depth-1][i][j];
	    }
    }
    checklist[depth]=place(x, y, option, 3-flag, *(virtualboard+depth));
    if (depth==DEPTH)
    {
	    return virtualmark(depth,me_flag);
    }
    int tempmark;
    for (int k = 7; k >= 0; k--) 
    {
        const int* delta = DIR[k];
        for (int i = 0; i < BOARD_SIZE; i++) 
	    {
            for (int j = 0; j < BOARD_SIZE; j++) 
		    {
                if (virtualboard[depth][i][j] != flag) 
			    {
                    continue;
                }
                int new_x = i + delta[0];
                int new_y = j + delta[1];
                if (isInBound(new_x, new_y) && virtualboard[depth][new_x][new_y] == EMPTY)
		        {
   	                tempmark=-virtualgame(3-flag,i,j,k,-beta,-alpha,depth+1);
                    if (tempmark>alpha)
		            {
			            alpha=tempmark;
		            }
					if(tempmark>=beta)
					{
						return tempmark;
					}
	            }
	    	}
	    }
    }
    return alpha;
}

struct Command aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int flag) 
{
    struct Command command = {0, 0, 0};
    int tempmark; 
    int chessnumber=0;
    for (int x = 0; x < BOARD_SIZE; x++) 
	{
        for (int y = 0; y < BOARD_SIZE; y++) 
	    {
		    virtualboard[0][x][y]=board[x][y];
		    if (board[x][y]==flag)
		    {
			   chessnumber++;
		    }
	    }
	}
    int beta=INFINITE;
    int alpha=-INFINITE;
    for (int k = 7; k >= 0; k--)
    {
        const int* delta = DIR[k];
        for (int x = 0; x < BOARD_SIZE; x++) 
	    {
            for (int y = 0; y < BOARD_SIZE; y++) 
	        {
                if (board[x][y] == flag) 
				{
                    int new_x = x + delta[0];
                    int new_y = y + delta[1];
                    if (isInBound(new_x, new_y) && board[new_x][new_y] == EMPTY) 
		            {	  
  	                    tempmark=-virtualgame(3-flag,x,y,k,-beta,-alpha,1);
		                if (privateround>4&&chessnumber<=8)
		                {
			                for (int i=2;i<=4;i++)
			                {
                                if(enemycommand[privateround].x==enemycommand[privateround-i].x&&enemycommand[privateround].y==enemycommand[privateround-i].y&&enemycommand[privateround].option==enemycommand[privateround-i].option)
				                {
				                    if (x==ourcommand[privateround-i].x&&y==ourcommand[privateround-i].y&&k==ourcommand[privateround-i].option)
			                        {
				                        tempmark-=1000;
				                    }
				                    if (x==ourcommand[privateround-i].x&&y==ourcommand[privateround-i].y&&k!=ourcommand[privateround-i].option)
			                        {
				                        tempmark-=300;
				                    }		  
				                }
			                }
		                }			
		                if (tempmark>alpha)
		                {
			                alpha=tempmark;
			                command.x=x;
                            command.y=y;
                            command.option=k;
		                }
		            }
                }
            }
        }
    }
    return command;
}

int main() 
{
    while (TRUE)
    {
        memset(buffer, 0, sizeof(buffer));
        gets(buffer);
        if (strstr(buffer, "START"))
        {
			privateround=0;
            char tmp[MAX_BYTE] = {0};
            sscanf(buffer, "%s %d", tmp, &me_flag);
            other_flag = 3 - me_flag;  
            memset(board, 0, sizeof(board));
            for (int i = 0; i < 3; i++) 
            {
                board[2][2 + i] = WHITE;
                board[6][6 + i] = WHITE;
                board[5][3 + i] = BLACK;
                board[9][7 + i] = BLACK;
            }
            for (int i = 0; i < 2; i++) 
            {
                board[8 + i][2] = WHITE;
                board[2 + i][9] = BLACK;
            } 
            printf("OK\n");
            fflush(stdout);
        }
        else if (strstr(buffer, "PLACE"))
        {
			privateround++;
            char tmp[MAX_BYTE] = {0};
            int x, y;
            OPTION option;
            sscanf(buffer, "%s %d %d %d", tmp, &x, &y, &option);
			enemycommand[privateround].x=x;
			enemycommand[privateround].y=y;
			enemycommand[privateround].option=option;
            place(x, y, option, other_flag,board);
        }
        else if (strstr(buffer, "TURN"))
        {
            struct Command command = aiTurn((const char (*)[BOARD_SIZE])board, me_flag);
            place(command.x, command.y, command.option, me_flag, board);
            printf("%d %d %d\n", command.x, command.y, command.option);
            fflush(stdout);
			ourcommand[privateround]=command;
        }  
        else if (strstr(buffer, "END"))
        {
            char tmp[MAX_BYTE] = {0};
            int x;
            sscanf(buffer, "%s %d", tmp, &x);
        }
    }
    return 0;
}

/**
* 以下为尾注部分，不对程序执行起到实际作用
*/

showmessage1()//太初有陈翔飞，陈翔飞与神同在。陈翔飞就是神。
{
	return 0;
}

showmessage2()//有一个人，是从陈翔飞那里差来的，名叫陈翔飞一号机。
{
	return 0;
}

showmessage3()//陈翔飞一号机成了肉身，住在我们中间，充充满满地有恩典，有真理。
{
	return 0;
}

showmessage4()//我们也见过他的荣光，正是陈翔飞的荣光。
{
	return 0;
}

showmessage5()//恩典和真理都是由陈翔飞一号机来的。
{
	return 0;
}

showmessage6()//从来没有人看见陈翔飞，只有陈翔飞创造的一号机将他表明出来。
{
	return 0;
}

showmessage7()//Rika所作的见证记在下面：
{
	return 0;
}

showmessage8()//那差来的是VS2017的死忠粉。
{
	return 0;
}

showmessage9()//他们就问他说：“你既不会VS，不会Clion，又不会Dev-C++，为什么编程呢？”
{
	return 0;
}

showmessage10()//Rika回答说：“我是用文字处理器编程。但有一位是你们认识的，就是那在西南一2104的，我给他解鞋带也不配。”
{
	return 0;
}

showmessage11()//这是Rika在同济大学四平路校区，陈翔飞一号机上学的地方作的见证。
{
	return 0;
}