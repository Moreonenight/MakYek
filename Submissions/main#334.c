#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2
#define INVALID 3
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

struct Command{
  int x;
  int y;
  OPTION option;
};

struct Commandlist{
  int x;
  int y;
  OPTION option;
  int mark;
};

struct Boardlist{
	int enemylist[16][2];
	int ourlist[16][2];
	int enemy;
	int me;
};

char buffer[MAX_BYTE] = {0};
//棋盘开的比较大是为了省去边界检查，多余的部分均初始化为3，与前面的#define 对应
char board[BOARD_SIZE+4][BOARD_SIZE+4];
int me_flag;
int other_flag;
int DIR[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };
char virtualboard[DEPTH+1][BOARD_SIZE+4][BOARD_SIZE+4];
struct Boardlist boardlist;
//记录搜索时每层被吃的子数
int checklist[DEPTH+1];
//记录每轮的走法，后面会用这个来减少重复的走法
struct Command enemycommand[MAX_TURN+1];
struct Command ourcommand[MAX_TURN+1];
//离敌方/我方棋子只有一格的空格视为敌方/我方棋子的控制点，后面的挑夹位估值会用到
char enemy_control[DEPTH+1][BOARD_SIZE+4][BOARD_SIZE+4];
char our_control[DEPTH+1][BOARD_SIZE+4][BOARD_SIZE+4];
//随机决定搜索时k的顺序
int dicenumber;
//记录当前是第几轮
int privateround;
//计算棋子间的距离
int min_distance;
//摆阵
int diamond;
//临近结束时决定搜索层数
int finalcontrol;
//劣势棋局专用对策所使用的位棋盘
unsigned long long my_bitboard[6]={0};

void bit_board()
{
    for (int i = 2; i < 6; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			my_bitboard[0]<<=1;
			my_bitboard[3]<<=1;
			if (board[i][j]==me_flag){
				my_bitboard[0]|=1;
			}
			if (board[i][j]==other_flag){
				my_bitboard[3]|=1;
			}
		}
	}
    for (int i = 6; i < 10; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			my_bitboard[1]<<=1;
			my_bitboard[4]<<=1;
			if (board[i][j]==me_flag){
				my_bitboard[1]|=1;
			}
			if (board[i][j]==other_flag){
				my_bitboard[4]|=1;
			}
		}
	}
    for (int i = 10; i < 14; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			my_bitboard[2]<<=1;
			my_bitboard[5]<<=1;
			if (board[i][j]==me_flag){
				my_bitboard[2]|=1;
			}
			if (board[i][j]==other_flag){
				my_bitboard[5]|=1;
			}
		}
	}
}

void printboard()
{
	printf("DEBUG ");
    for (int i = 2; i < 6; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			if (board[i][j]==me_flag){
				printf("1");
			}
			else{
				printf("0");
			}
		}
	}
	printf("\n");
	printf("DEBUG ");
    for (int i = 6; i < 10; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			if (board[i][j]==me_flag){
				printf("1");
			}
			else{
				printf("0");
			}
		}
	}
	printf("\n");
	printf("DEBUG ");	
    for (int i = 10; i < 14; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			if (board[i][j]==me_flag){
				printf("1");
			}
			else{
				printf("0");
			}
		}
	}
	printf("\n");
	printf("DEBUG ");
    for (int i = 2; i < 6; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			if (board[i][j]==other_flag){
				printf("1");
			}
			else{
				printf("0");
			}
		}
	}
	printf("\n");
	printf("DEBUG ");
    for (int i = 6; i < 10; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			if (board[i][j]==other_flag){
				printf("1");
			}
			else{
				printf("0");
			}
		}
	}
	printf("\n");
	printf("DEBUG ");	
    for (int i = 10; i < 14; i++) {
        for (int j = 2; j < BOARD_SIZE+2; j++) {
			if (board[i][j]==other_flag){
				printf("1");
			}
			else{
				printf("0");
			}
		}
	}
	printf("\n");	
}

int dice_player(int k){
	if (dicenumber==1){
		if(k==6)
			return 5;
		if(k==5)
			return 6;
	}
	if (dicenumber==2){
		if(k==0)
			return 1;
		if(k==1)
			return 0;
	}
	if (dicenumber==3){
		if(k==6)
			return 5;
		if(k==5)
			return 6;
		if (k==0)
			return 1;
		if(k==1)
			return 0;			
	}
    return k;
}

int place(int x, int y, OPTION option, int cur_flag, char placeboard[BOARD_SIZE+4][BOARD_SIZE+4],int depth) {
	//place函数中也对our_control, enemy_control, virtualboard几个棋盘作了更新
    int temp=0;	
    int new_x = x + DIR[option][0];
    int new_y = y + DIR[option][1];	
    placeboard[x][y] = EMPTY;
	if (cur_flag==me_flag){
		for (int k=0;k<8;k++){
			our_control[depth][x+DIR[k][0]][y+DIR[k][1]]--;
		}
	}
	else{
		for (int k=0;k<8;k++){
			enemy_control[depth][x+DIR[k][0]][y+DIR[k][1]]--;
		}
	}		
    placeboard[new_x][new_y] = cur_flag;
	if (cur_flag==me_flag){
		for (int k=0;k<8;k++){
			our_control[depth][new_x+DIR[k][0]][new_y+DIR[k][1]]++;
		}
	}
	else{
		for (int k=0;k<8;k++){
			enemy_control[depth][new_x+DIR[k][0]][new_y+DIR[k][1]]++;
		}
	}		
    int cur_other_flag = 3 - cur_flag;
    int intervention_dir[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
    for (int i = 0; i < 4; i++) {
        int x1 = new_x + intervention_dir[i][0];
        int y1 = new_y + intervention_dir[i][1];
        int x2 = new_x - intervention_dir[i][0];
        int y2 = new_y - intervention_dir[i][1];
        if (placeboard[x1][y1] == cur_other_flag && placeboard[x2][y2] == cur_other_flag) {
            placeboard[x1][y1] = cur_flag;
            placeboard[x2][y2] = cur_flag; 			
		    if (cur_flag==me_flag){
		        for (int k=0;k<8;k++){
					our_control[depth][x1+DIR[k][0]][y1+DIR[k][1]]++;
					our_control[depth][x2+DIR[k][0]][y2+DIR[k][1]]++;	
					enemy_control[depth][x1+DIR[k][0]][y1+DIR[k][1]]--;
					enemy_control[depth][x2+DIR[k][0]][y2+DIR[k][1]]--;					
				}
 	        }
			else{
				for (int k=0;k<8;k++){
					our_control[depth][x1+DIR[k][0]][y1+DIR[k][1]]--;
					our_control[depth][x2+DIR[k][0]][y2+DIR[k][1]]--;	
					enemy_control[depth][x1+DIR[k][0]][y1+DIR[k][1]]++;
					enemy_control[depth][x2+DIR[k][0]][y2+DIR[k][1]]++;	
				}
			}			
	        temp+=2;
        }
    }
    int custodian_dir[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
    for (int i = 0; i < 8; i++) {
        int x1 = new_x + custodian_dir[i][0];
        int y1 = new_y + custodian_dir[i][1];
        int x2 = new_x + custodian_dir[i][0] * 2;
        int y2 = new_y + custodian_dir[i][1] * 2;
        if (placeboard[x2][y2] == cur_flag && placeboard[x1][y1] == cur_other_flag) {
            placeboard[x1][y1] = cur_flag;		
			if (cur_flag==me_flag){
				for (int k=0;k<8;k++){
					our_control[depth][x1+DIR[k][0]][y1+DIR[k][1]]++;
					enemy_control[depth][x1+DIR[k][0]][y1+DIR[k][1]]--;					
				}
			}
			else{
				for (int k=0;k<8;k++){
					our_control[depth][x1+DIR[k][0]][y1+DIR[k][1]]--;	
					enemy_control[depth][x1+DIR[k][0]][y1+DIR[k][1]]++;
				}
			}		
			temp+=1;
        }
    }
    return temp;
}

int abs(int i){
	return i>=0?i:-i;
}

int max(int a,int b){
	return a>b?a:b;
}

int virtualmark(int depth){
	int tempmark;
	//不鼓励先送子再吃回来。因为未必能吃回来 
    tempmark=2000*(checklist[1]-checklist[2])+1800*(checklist[3]-checklist[4]);
    tempmark-=2*min_distance;
	tempmark+=diamond;
	int me_yak=0;
	int me_mak=0;
	int other_yak=0;
	int other_mak=0;
	//判定危险棋子
    for (int i = 2; i < BOARD_SIZE+2; i++) {
            for (int j = 2; j < BOARD_SIZE+2; j++) {
				if (virtualboard[depth][i][j]==me_flag){
					int breakflag=0;
					int mak_breakflag=0;
					for (int k1 = 7; k1 >= 0; k1--) {					
						int i1=i+DIR[k1][0];
						int j1=j+DIR[k1][1];
						int i2=i-DIR[k1][0];
						int j2=j-DIR[k1][1];						
						if (virtualboard[depth][i1][j1]==EMPTY&&enemy_control[depth][i1][j1]!=0){
							if (virtualboard[depth][i2][j2]==other_flag){
									    me_mak++;
										mak_breakflag=1;
										breakflag=1;
										break;
						    }
						    else if (virtualboard[depth][i+2*DIR[k1][0]][j+2*DIR[k1][1]]==me_flag){
									    me_yak++;
										breakflag=1;
										break;
						    }							
					    }					
					}
					//如果危险棋子不容易规避，额外扣分
					if (enemy_control[depth][i][j]!=0){
						for (int k1=0;k1<8;k1++){
							if (virtualboard[depth][i+DIR[k1][0]][j+DIR[k1][1]]==me_flag){
								if (virtualboard[depth][i+2*DIR[k1][0]][j+2*DIR[k1][1]]==other_flag){
									if (mak_breakflag==1){
										tempmark-=150;
									}
									else{
										tempmark-=10;
									}								
									break;
								}
							}
						}
					}				
					if (breakflag==0){
						if (enemy_control[depth][i][j]==0){
							int SUN[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
							for (int k1 = 3; k1 >= 0; k1--) { 
								int i1=i+SUN[k1][0];
								int j1=j+SUN[k1][1];
								int i2=i-SUN[k1][0];
								int j2=j-SUN[k1][1];							
								if (virtualboard[depth][i1][j1]==EMPTY&&enemy_control[depth][i1][j1]!=0&&virtualboard[depth][i2][j2]==EMPTY&&enemy_control[depth][i2][j2]!=0){
									tempmark-=20;
									break;
								}								
							}
						}
						if (enemy_control[depth][i][j]>=2){
							tempmark-=5;
						}
					}
				}
				if (virtualboard[depth][i][j]==other_flag){
					int breakflag=0;
					int mak_breakflag=0;
					for (int k1 = 7; k1 >= 0; k1--) {						
						int i1=i+DIR[k1][0];
						int j1=j+DIR[k1][1];
						int i2=i-DIR[k1][0];
						int j2=j-DIR[k1][1];						
						if (virtualboard[depth][i1][j1]==EMPTY&&our_control[depth][i1][j1]!=0){
							if (virtualboard[depth][i2][j2]==me_flag){
									    other_mak++;
                                        mak_breakflag=1;										
										breakflag=1;
										break;
						    }							
						    else if (virtualboard[depth][i+2*DIR[k1][0]][j+2*DIR[k1][1]]==other_flag){
									    other_yak++;
										breakflag=1;
										break;
						    }
					    }						
					}
					if (our_control[depth][i][j]!=0){
						for (int k1=0;k1<8;k1++){
							if (virtualboard[depth][i+DIR[k1][0]][j+DIR[k1][1]]==other_flag){
								if (virtualboard[depth][i+2*DIR[k1][0]][j+2*DIR[k1][1]]==me_flag){
									if (mak_breakflag==1){
										tempmark+=200;
									}
									else{
										tempmark+=10;
									}
									break;
								}
							}
						}
					}
					if (breakflag==0){
						if (our_control[depth][i][j]==0){
							int SUN[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
							for (int k1 = 3; k1 >= 0; k1--) { 
								int i1=i+SUN[k1][0];
								int j1=j+SUN[k1][1];
								int i2=i-SUN[k1][0];
								int j2=j-SUN[k1][1];							
								if (virtualboard[depth][i1][j1]==EMPTY&&our_control[depth][i1][j1]!=0&&virtualboard[depth][i2][j2]==EMPTY&&our_control[depth][i2][j2]!=0){
									tempmark+=20;
									break;
								}								
							}
						}
						if (our_control[depth][i][j]>=2){
							tempmark+=5;
						}						
					}						
				}				
		    }
		}
	int tempnumber=me_yak/2+me_mak-1;
	if (tempnumber>0){
	  tempmark-=(me_yak+me_mak)*500+300;
	}
	if (tempnumber==0){
		tempmark-=300;
	}
	if (other_yak>0){
		tempmark+=400+600*other_yak;
		tempmark+=600*other_mak;
	}
	if (other_yak==0){
		tempmark+=200+600*other_mak;
	}		
	return tempmark;
}

int virtualgame(int flag,int x,int y,int option,int alpha,int beta,int depth)
{
	memcpy(virtualboard[depth],virtualboard[depth-1],sizeof(virtualboard[depth]));
	memcpy(enemy_control[depth],enemy_control[depth-1],sizeof(enemy_control[depth]));
	memcpy(our_control[depth],our_control[depth-1],sizeof(our_control[depth]));
    checklist[depth]=place(x, y, option, 3-flag, *(virtualboard+depth),depth);
    if (depth==DEPTH){
	    return virtualmark(depth);
    }
	if (depth==DEPTH-1){
		//摆阵
		diamond=0;
	    int DIA[8][2] = {{3,-1},{2, -1}, {1, -2}, {-2, -1}, {-1, -2},{1,-3}, {-1, -3},{-3, -1}};
	    for(int i=3;i<BOARD_SIZE+1;i++){
		    for(int j=3;j<BOARD_SIZE+2;j++){
			    if (virtualboard[depth][i][j]==me_flag){
					for (int k=0;k<8;k++){
						if (virtualboard[depth][i+DIA[k][0]][j+DIA[k][1]]==me_flag){
							diamond++;
						}
					}						
			    }
		    }
	    }
		for(int j=3;j<BOARD_SIZE+2;j++){
			if (virtualboard[depth][BOARD_SIZE+1][j]==me_flag){
				for (int k=1;k<8;k++){
					if (virtualboard[depth][BOARD_SIZE+1+DIA[k][0]][j+DIA[k][1]]==me_flag){
						diamond++;
					}
				}						
			}
		}
		for(int j=3;j<BOARD_SIZE+2;j++){
			if (virtualboard[depth][2][j]==me_flag){
				for (int k=0;k<7;k++){
					if (virtualboard[depth][2+DIA[k][0]][j+DIA[k][1]]==me_flag){
						diamond++;
					}
				}						
			}
		}		
	}
	if (depth==DEPTH-2){
		//靠近敌方棋子
	    boardlist.me=0;
	    boardlist.enemy=0;
	    for(int i=2;i<BOARD_SIZE+2;i++){
		    for(int j=2;j<BOARD_SIZE+2;j++){
			    if (virtualboard[depth][i][j]==me_flag){
				    boardlist.ourlist[boardlist.me][0]=i;
				    boardlist.ourlist[boardlist.me][1]=j;
				    (boardlist.me)++;
			    }
			    if (virtualboard[depth][i][j]==other_flag){
				    boardlist.enemylist[boardlist.enemy][0]=i;
				    boardlist.enemylist[boardlist.enemy][1]=j;
				    (boardlist.enemy)++;
			    }
		    }
	    }
	    int array[16]={0};
	    min_distance=0;	
		for (int j=0;j<boardlist.enemy;j++){
			array[j]=0;
		    for(int i=0;i<boardlist.me;i++){
			    array[j]+=max(abs(boardlist.ourlist[i][0]-boardlist.enemylist[j][0]),abs(boardlist.ourlist[i][1]-boardlist.enemylist[j][0]));
		    }
	    }
		min_distance=array[0];
		for (int j=0;j<boardlist.enemy;j++){
			if (array[j]<min_distance){
				min_distance=array[j];
			}
		}
	}	
    int tempmark;
    for (int k = 7; k >= 0; k--) {		
        for (int i = 2; i < BOARD_SIZE+2; i++) {
            for (int j = 2; j < BOARD_SIZE+2; j++) {
                if (virtualboard[depth][i][j] == flag){					
                int new_x = i + DIR[k][0];
                int new_y = j + DIR[k][1];
					if (virtualboard[depth][new_x][new_y] == EMPTY){
						tempmark=-virtualgame(3-flag,i,j,k,-beta,-alpha,depth+1);
						if (tempmark>alpha){
							alpha=tempmark;
						}
						if(tempmark>=beta){
							return tempmark;
						}
					}
				}
	    	}
	    }			
    }
    return alpha;
}

struct Command aiTurn(const char board[BOARD_SIZE+4][BOARD_SIZE+4], int flag) 
{
	//每步生成随机数的种子
	srand(time(0));
	dicenumber=(rand()&3);	
    struct Command command = {0, 0, 0};
    int tempmark; 
    int chessnumber=0;	
    for (int x = 2; x < BOARD_SIZE+2; x++) {
        for (int y = 2; y < BOARD_SIZE+2; y++) {
		    virtualboard[0][x][y]=board[x][y];
			enemy_control[1][x][y]=enemy_control[0][x][y];
			our_control[1][x][y]=our_control[0][x][y];			
		    if (board[x][y]==flag){
			   chessnumber++;
		    }
	    }
	}
    int beta=INFINITE;
    int alpha=-INFINITE;
	int sum=0;
	//下面是浅层排序，用于减少搜索耗时
	struct Commandlist commandlist[128];
    for (int k = 7; k >= 0; k--){
		// 随机决定k的搜索顺序
		k=dice_player(k);
        for (int x = 2; x < BOARD_SIZE+2; x++) {
            for (int y = 2; y < BOARD_SIZE+2; y++) {
                if (board[x][y] == flag) {
                    int new_x = x + DIR[k][0];
                    int new_y = y + DIR[k][1];
                    if (board[new_x][new_y] == EMPTY) {
						memcpy(virtualboard[1],virtualboard[0],sizeof(virtualboard[0]));
						checklist[1]=place(x, y, k, flag, *(virtualboard+1),1);
						checklist[2]=0;
						int tempnow;
						for (int k0=7;k0>=0;k0--){
							for (int  i = 2; i < BOARD_SIZE+2; i++) {
								for (int j = 2; j < BOARD_SIZE+2; j++) {
									if (virtualboard[1][i][j] == 3-flag){					
										int new_x0 = i + DIR[k0][0];
										int new_y0 = j + DIR[k0][1];
										if (virtualboard[1][new_x0][new_y0] == EMPTY){
											memcpy(virtualboard[2],virtualboard[1],sizeof(virtualboard[1]));
											tempnow=place(i, j, k0, 3-flag, *(virtualboard+2),2);
											if (tempnow>checklist[2]){
												checklist[2]=tempnow;
											}
										}
									}
								}				
							}
						}
						commandlist[sum].x=x;
						commandlist[sum].y=y;
						commandlist[sum].option=k;
						commandlist[sum].mark=checklist[1]-checklist[2];
						sum++;
			        }
		        }
			}
		}
		k=dice_player(k);
	}
	struct Commandlist midlist;
	for (int s=0;s<sum;s++){
		for (int s1=s+1;s1<sum;s1++){
            if (commandlist[s].mark<commandlist[s1].mark){
				midlist.x=commandlist[s1].x;
				midlist.y=commandlist[s1].y;				
				midlist.option=commandlist[s1].option;
				midlist.mark=commandlist[s1].mark;
				commandlist[s1].x=commandlist[s].x;
				commandlist[s1].y=commandlist[s].y;	
				commandlist[s1].option=commandlist[s].option;
				commandlist[s1].mark=commandlist[s].mark;
				commandlist[s]=midlist;
			}
		}
	}
	//对每个着法进行搜索
    for (int s=0;s<sum;s++){
  	    tempmark=-virtualgame(3-flag,commandlist[s].x,commandlist[s].y,commandlist[s].option,-beta,-alpha,1);			
		if (privateround>10&&chessnumber<=8){
		    for (int i=2;i<=4;i++){
				// 下面的扣分是为了在均势或劣势时减少循环着法 
                if(enemycommand[privateround].x==enemycommand[privateround-i].x&&enemycommand[privateround].y==enemycommand[privateround-i].y&&enemycommand[privateround].option==enemycommand[privateround-i].option){
				    if (enemycommand[privateround].x==enemycommand[privateround-2*i].x&&enemycommand[privateround].y==enemycommand[privateround-2*i].y&&enemycommand[privateround].option==enemycommand[privateround-2*i].option){
						if(chessnumber<8){
				            if (commandlist[s].x==ourcommand[privateround-i].x&&commandlist[s].y==ourcommand[privateround-i].y&&commandlist[s].option==ourcommand[privateround-i].option){
								tempmark-=10000;
							}
						}
				    }
					else{
				        if (commandlist[s].x==ourcommand[privateround-i].x&&commandlist[s].y==ourcommand[privateround-i].y&&commandlist[s].option==ourcommand[privateround-i].option){
				            tempmark-=1000;
				        }
				            if (commandlist[s].x==ourcommand[privateround-i].x&&commandlist[s].y==ourcommand[privateround-i].y&&commandlist[s].option!=ourcommand[privateround-i].option){
				                tempmark-=300;
				            }  
					}										
				}
			}
		}
		if (tempmark>alpha){
			printf("DEBUG (%d,%d,%d),mark=%d\n",commandlist[s].x-2,commandlist[s].y-2,commandlist[s].option,tempmark);
			alpha=tempmark;
			alpha=tempmark;
			command.x=commandlist[s].x;
            command.y=commandlist[s].y;
            command.option=commandlist[s].option;
		}
    }		
    return command;
}
// 带Final的这些函数是临近结束时搜索和打分用的 
int Finalvirtualmark(int depth)
{
	int chessnumber=0;
    for (int x = 2; x < BOARD_SIZE+2; x++) {
        for (int y = 2; y < BOARD_SIZE+2; y++) {
		    if (virtualboard[depth][x][y]==me_flag){
			   chessnumber++;
		    }
	    }
	}
	return chessnumber;
}

int Finalvirtualgame(int flag,int x,int y,int option,int alpha,int beta,int depth){
    memcpy(virtualboard[depth],virtualboard[depth-1],sizeof(virtualboard[depth]));	
    place(x, y, option, 3-flag, virtualboard[depth],depth);
    if (depth==finalcontrol){
		if ((finalcontrol&1)==0){
	        return Finalvirtualmark(depth);
		}
		else{
			return -Finalvirtualmark(depth);
		}
    }
    int tempmark;
    for (int k = 7; k >= 0; k--) {
        for (int i = 2; i < BOARD_SIZE+2; i++) {
            for (int j = 2; j < BOARD_SIZE+2; j++) {
                if (virtualboard[depth][i][j] == flag){					
					int new_x = i + DIR[k][0];
					int new_y = j + DIR[k][1];
					if (virtualboard[depth][new_x][new_y] == EMPTY){
						tempmark=-Finalvirtualgame(3-flag,i,j,k,-beta,-alpha,depth+1);
						if (tempmark>alpha){
							alpha=tempmark;
						}
						if(tempmark>=beta){
							return tempmark;
						}
					}
				}
	    	}
	    }
    }
    return alpha;
}

struct Command aiFinalTurn(const char board[BOARD_SIZE+4][BOARD_SIZE+4], int flag) {	
	if (privateround==59&&me_flag==BLACK){
		finalcontrol=4;
	}
	if (privateround==59&&me_flag==WHITE){
		finalcontrol=3;
	}
	if (privateround==60&&me_flag==BLACK){
		finalcontrol=2;
	}
	if (privateround==60&&me_flag==WHITE){
		finalcontrol=1;
	}	
    struct Command command = {0, 0, 0};	
    int tempmark; 
    int beta=INFINITE;
    int alpha=-INFINITE;
	memcpy(virtualboard[0],board,sizeof(virtualboard[0]));
    for (int k = 7; k >= 0; k--){		
        for (int x = 2; x < BOARD_SIZE+2; x++) {
            for (int y = 2; y < BOARD_SIZE+2; y++) {
                if (board[x][y] == flag) {
                    int new_x = x + DIR[k][0];
                    int new_y = y + DIR[k][1];
                    if (board[new_x][new_y] == EMPTY) {	  
  	                    tempmark=-Finalvirtualgame(3-flag,x,y,k,-beta,-alpha,1);						
		                if (tempmark>alpha){
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

int main() {
    while (TRUE){
        memset(buffer, 0, sizeof(buffer));
        gets(buffer);
        if (strstr(buffer, "START")){
            char tmp[MAX_BYTE] = {0};
            sscanf(buffer, "%s %d", tmp, &me_flag);
			finalcontrol=0;
			if (me_flag==BLACK){
			   privateround=1;
			}
			else{
				privateround=0;
			}
            other_flag = 3 - me_flag;  					
            memset(board, 3, sizeof(board));
			memset(virtualboard, 3, sizeof(virtualboard));
			for (int i=2;i<BOARD_SIZE+2;i++){
				for(int j=2;j<BOARD_SIZE+2;j++){
					board[i][j]=0;
				}
			}
			for (int i=2;i<BOARD_SIZE+2;i++){
				for(int j=2;j<BOARD_SIZE+2;j++){
					for (int d=0;d<=DEPTH;d++){
					    virtualboard[d][i][j]=0;
					}
				}
			}
            for (int i = 0; i < 3; i++) {
                board[4][4 + i] = WHITE;
                board[8][8 + i] = WHITE;
                board[7][5 + i] = BLACK;
                board[11][9 + i] = BLACK;
            }
            for (int i = 0; i < 2; i++) {
                board[10 + i][4] = WHITE;
                board[4 + i][11] = BLACK;
            }
			for (int i=2;i<BOARD_SIZE+2;i++){
				for(int j=2;j<BOARD_SIZE+2;j++){
					if (board[i][j]==other_flag){
						for (int k=0;k<8;k++){
						    enemy_control[0][i+DIR[k][0]][j+DIR[k][1]]++;
						}
					}
					if (board[i][j]==me_flag){
						for (int k=0;k<8;k++){
						    our_control[0][i+DIR[k][0]][j+DIR[k][1]]++;
						}
					}					
				}
			}			
            printf("OK\n");
            fflush(stdout);
        }
        else if (strstr(buffer, "PLACE")){
			privateround++;
            char tmp[MAX_BYTE] = {0};
            int x, y;
            OPTION option;
            sscanf(buffer, "%s %d %d %d", tmp, &x, &y, &option);								
			enemycommand[privateround].x=x+2;
			enemycommand[privateround].y=y+2;
			enemycommand[privateround].option=option;				
            place(x+2, y+2, option, other_flag,board,0);
        }
        else if (strstr(buffer, "TURN")){
			struct Command command;
			bit_board();
			printboard();
			// 开局的部分情况进行了手动应对 
			if (privateround<=2){
				if (privateround==1&&me_flag==BLACK){
					command.x=11;
					command.y=9;
					command.option=4;
				}
				else if (privateround==1&&me_flag==WHITE){
					if (enemycommand[1].x==11){
					   command.x=8;
					   command.y=9;
					   command.option=1;
					}
					else {
					   command.x=4;
					   command.y=6;
					   command.option=7;
					}
				}				
				else if (privateround==2&&me_flag==WHITE){
					if (board[4][6]==WHITE&&board[5][7]==EMPTY&&board[9][10]!=BLACK&&board[9][8]!=BLACK&&board[7][8]!=BLACK){
					   command.x=4;
					   command.y=6;
					   command.option=7;
					}
					else {
						command = aiTurn((const char (*)[BOARD_SIZE+4])board, me_flag);
					}
				}
				else if (privateround==2&&me_flag==BLACK){
					if (enemycommand[2].x==8){
					   command.x=5;
					   command.y=11;
					   command.option=6;
					}
					else {
						command = aiTurn((const char (*)[BOARD_SIZE+4])board, me_flag);
					}
				}	
				else if (privateround==2&&me_flag==BLACK){
					if (enemycommand[2].x==4&&enemycommand[2].y==5&&enemycommand[2].option==7&&board[7][7]==BLACK&&board[8][7]==EMPTY){
					   command.x=7;
					   command.y=7;
					   command.option=1;
					}					
					else {
						command = aiTurn((const char (*)[BOARD_SIZE+4])board, me_flag);
					}
				}			
				else{
					command = aiTurn((const char (*)[BOARD_SIZE+4])board, me_flag);
				}				
			}
			else if (privateround<=58){
                command = aiTurn((const char (*)[BOARD_SIZE+4])board, me_flag);
			}
			else{
				command = aiFinalTurn((const char (*)[BOARD_SIZE+4])board, me_flag);
			}
			// 针对程序会给出劣势着法的局面，手动控制其给出适当的着法
			ourcommand[privateround]=command;			
            place(command.x, command.y, command.option, me_flag, board,0);			
            printf("%d %d %d\n", command.x-2, command.y-2, command.option);
            fflush(stdout);
        }
        else if (strstr(buffer, "END")){
            char tmp[MAX_BYTE] = {0};
            int x;
            sscanf(buffer, "%s %d", tmp, &x);
        }
    }
    return 0;
}

//尾注
int message1(){/*陈翔飞说：你不要搞我。我告诉你几个神仙，你去搞他们吧。*/}
int message2(){/*然后，只有沉默。*/}
int message3(){/*“上帝能够创造出一块自己搬不起来的石头吗？”，我不知道答案。*/}
int message4(){/*但陈翔飞的确找不出值得被他称为神仙的人。*/}
int message5(){/*无论如何，*/}
int message6(){/*如果飞总已经在保佑我，愿他仍予我保佑。如果飞总尚未保佑我，愿他对我怀有仁慈。*/}