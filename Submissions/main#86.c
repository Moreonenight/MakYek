#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// board information
#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2

// bool
typedef int BOOL;
#define TRUE 1
#define FALSE 0

// option
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

#define START "START"
#define PLACE "PLACE"
#define TURN  "TURN"
#define END   "END"

struct Command
{
  int x;
  int y;
  OPTION option;
};

struct TestCommand
{
	int x;
	int y;
	OPTION option;
	int mark;
};

struct Boardlist
{
	int blacklist[16][2];
	int whitelist[16][2];
	int black;
	int white;
};

struct Boardlist boardlist;
struct Command command = {0, 0, 0};
struct TestCommand testcommand[16]={{0,0,0,0}};

int abs(int i)
{
	if(i>=0)
		return i;
	else
		return -i;
}

int max(int a,int b)
{
	if(a>=b)
		return a;
	else
		return b;
}

void Listboard(char board[BOARD_SIZE][BOARD_SIZE])
{
	boardlist.black=0;
	boardlist.white=0;
	for(int i=0;i<BOARD_SIZE;i++)
	{
		for(int j=0;j<BOARD_SIZE;j++)
		{
			if (board[i][j]==BLACK)
			{
				boardlist.blacklist[boardlist.black][0]=i;
				boardlist.blacklist[boardlist.black][1]=j;
				(boardlist.black)++;
			}
			if (board[i][j]==WHITE)
			{
				boardlist.whitelist[boardlist.white][0]=i;
				boardlist.whitelist[boardlist.white][1]=j;
				(boardlist.white)++;
			}
		}
	}			
}

int distance(int x,int y,int flag)
{
	int array[16];
	int min=12;
	if (flag==BLACK)
	{
		for(int i=0;i<boardlist.white;i++)
		{
			array[i]=max(abs(boardlist.whitelist[i][0]-x),abs(boardlist.whitelist[i][1]-y));
		}
		for(int i=0;i<boardlist.white;i++)
		{
			if (array[i]<min)
			{
				min=array[i];
			}
		}
	}
	if (flag==WHITE)
	{
		for(int i=0;i<boardlist.black;i++)
		{
			array[i]=max(abs(boardlist.blacklist[i][0]-x),abs(boardlist.blacklist[i][1]-y));
		}
		for(int i=0;i<boardlist.black;i++)
		{
			if (array[i]<min)
			{
				min=array[i];
			}
		}		
	}
		return min;
}
char buffer[MAX_BYTE] = {0};
char board[BOARD_SIZE][BOARD_SIZE] = {{0}};
int me_flag;
int other_flag;
int privateround; 

const int DIR[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

void debug(const char *str) {
  printf("DEBUG %s\n", str);
  fflush(stdout);
}

void printBoard() {
  char visual_board[BOARD_SIZE][BOARD_SIZE] = {{0}};
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (board[i][j] == EMPTY) {
        visual_board[i][j] = '.';
      } else if (board[i][j] == BLACK) {
        visual_board[i][j] = 'O';
      } else if (board[i][j] == WHITE) {
        visual_board[i][j] = 'X';
      }
    }
    printf("%s\n", visual_board[i]);
  }
}

BOOL isInBound(int x, int y) {
  return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

BOOL SearchFlag(int x, int y,int flag) {
	for (int k = 0; k < 8; k++) {
    const int* rika = DIR[k];
    if (board[x+rika[0]][y+rika[1]]==flag)
    	    return TRUE;
}
return FALSE;
}

int DGivemark(int x,int y,int flag)
{
	int tempmark=0;
	for (int k = 0; k < 8; k++) {
    const int* rika = DIR[k];	

	if(board[x-rika[0]][y-rika[1]]==other_flag&& board[x+rika[0]][y+rika[1]]==other_flag)
	{
		tempmark+=100;
		board[x-rika[0]][y-rika[1]]=3;
		board[x+rika[0]][y+rika[1]]=3;
		
	}
	if(board[x+2*rika[0]][y+2*rika[1]]==me_flag&& board[x+rika[0]][y+rika[1]]==other_flag)
	{
		tempmark+=50;
        board[x+rika[0]][y+rika[1]]=3;
		for (int k1 = 0; k1 < 8; k1++) {
        const int* rika2 = DIR[k1];	
		if (board[x+rika[0]+2*rika2[0]][y+rika[1]+2*rika2[1]]==flag&&board[x+rika[0]+rika2[0]][y+rika[1]+rika2[1]]==EMPTY)
		{
			if (SearchFlag(x+rika[0]+rika2[0],y+rika[1]+rika2[1],other_flag))
			tempmark-=100;
		}
		if(board[x+rika[0]+rika2[0]][y+rika[1]+rika2[1]]==other_flag&&board[x+rika[0]-rika2[0]][y+rika[1]-rika2[1]]==EMPTY)
		{
			if (SearchFlag(x+rika[0]-rika2[0],y+rika[1]-rika2[1],other_flag))
				tempmark-=50;
		}
		}

	}
	}
	for (int k = 0; k < 8; k++) {
    const int* rika = DIR[k];	
    if ((board[x+2*rika[0]][y+2*rika[1]]==flag||board[x+2*rika[0]][y+2*rika[1]]==3)&&board[x+rika[0]][y+rika[1]]==EMPTY)
	{
		if (SearchFlag(x+rika[0],y+rika[1],other_flag))
    	    tempmark-=100;
		else
			tempmark-=5;
	}
	if(board[x+rika[0]][y+rika[1]]==other_flag&&board[x-rika[0]][y-rika[1]]==EMPTY)
	{
		if (board[x-rika[0]][y-rika[1]]==EMPTY&&SearchFlag(x-rika[0],y-rika[1],other_flag))
			tempmark-=50;
		if (board[x+2*rika[0]][y+2*rika[1]]==EMPTY&&SearchFlag(x+2*rika[0],y+2*rika[1],flag))
			tempmark+=10;
	}
	if(board[x+rika[0]][y+rika[1]]==flag&&board[x+2*rika[0]][y+2*rika[1]]==other_flag)
	{
		if (SearchFlag(x,y,other_flag))
			tempmark+=50;
	}
	if(board[x+rika[0]][y+rika[1]]==flag&&board[x-rika[0]][y-rika[1]]==flag)
	{
		if (SearchFlag(x,y,other_flag))
			tempmark+=100;
		else
			tempmark+=5;
	}	
	} 
	 for (int x = 0; x < BOARD_SIZE; x++) {
      for (int y = 0; y < BOARD_SIZE; y++) {
        if (board[x][y] == 3) {
          board[x][y]=other_flag;
        }
	  }
	 }
  tempmark-=distance(x,y,flag);
  return tempmark;
}

int AGivemark(int x,int y,int flag)
{
	int tempmark=0;
	for (int k = 0; k < 8; k++) {
    const int* rika = DIR[k];	

	if(board[x-rika[0]][y-rika[1]]==other_flag&& board[x+rika[0]][y+rika[1]]==other_flag)
	{
		tempmark+=100;
		board[x-rika[0]][y-rika[1]]=3;
		board[x+rika[0]][y+rika[1]]=3;
		
	}
	if(board[x+2*rika[0]][y+2*rika[1]]==me_flag&& board[x+rika[0]][y+rika[1]]==other_flag)
	{
		tempmark+=50;
        board[x+rika[0]][y+rika[1]]=3;
		for (int k1 = 0; k1 < 8; k1++) {
        const int* rika2 = DIR[k1];	
		if (board[x+rika[0]+2*rika2[0]][y+rika[1]+2*rika2[1]]==flag&&board[x+rika[0]+rika2[0]][y+rika[1]+rika2[1]]==EMPTY)
		{
			if (SearchFlag(x+rika[0]+rika2[0],y+rika[1]+rika2[1],other_flag))
			tempmark-=100;
		}
		if(board[x+rika[0]+rika2[0]][y+rika[1]+rika2[1]]==other_flag&&board[x+rika[0]-rika2[0]][y+rika[1]-rika2[1]]==EMPTY)
		{
			if (SearchFlag(x+rika[0]-rika2[0],y+rika[1]-rika2[1],other_flag))
				tempmark-=50;
		}
		}

	}
	}
	for (int k = 0; k < 8; k++) {
    const int* rika = DIR[k];	
    if ((board[x+2*rika[0]][y+2*rika[1]]==flag||board[x+2*rika[0]][y+2*rika[1]]==3)&&board[x+rika[0]][y+rika[1]]==EMPTY)
	{
		if (SearchFlag(x+rika[0],y+rika[1],other_flag))
    	    tempmark-=100;
		else
			tempmark-=5;
	}
	if(board[x+rika[0]][y+rika[1]]==other_flag&&board[x-rika[0]][y-rika[1]]==EMPTY)
	{
		if (board[x-rika[0]][y-rika[1]]==EMPTY&&SearchFlag(x-rika[0],y-rika[1],other_flag))
			tempmark-=50;
		if (board[x+2*rika[0]][y+2*rika[1]]==EMPTY&&SearchFlag(x+2*rika[0],y+2*rika[1],flag))
			tempmark+=10;
	}
	if(board[x+rika[0]][y+rika[1]]==flag&&board[x+2*rika[0]][y+2*rika[1]]==other_flag)
	{
		if (SearchFlag(x,y,other_flag))
			tempmark+=50;
	}
	if(board[x+rika[0]][y+rika[1]]==flag&&board[x-rika[0]][y-rika[1]]==flag)
	{
		if (SearchFlag(x,y,other_flag))
			tempmark+=100;
		else
			tempmark+=5;
	}	
	} 
	 for (int x = 0; x < BOARD_SIZE; x++) {
      for (int y = 0; y < BOARD_SIZE; y++) {
        if (board[x][y] == 3) {
          board[x][y]=other_flag;
        }
	  }
	 }
  tempmark-=distance(x,y,flag);
  return tempmark;
}

int Givemark(int x,int y,int flag)
{	
	int tempmark=0;
	int max=0;
	for (int k = 0; k < 8; k++) {
    const int* rika = DIR[k];
   if (board[x+2*rika[0]][y+2*rika[1]]==flag&&board[x+rika[0]][y+rika[1]]==EMPTY)
	{
		if (SearchFlag(x+rika[0],y+rika[1],other_flag))
    	    tempmark-=100;
		else
			tempmark-=5;
	}
	if (board[x+rika[0]][y+rika[1]]==flag&&board[x-rika[0]][y-rika[1]]==flag)
	{
		if(SearchFlag(x,y,other_flag))
			tempmark+=100;
		else
			tempmark+=5;
	}
	if (board[x+rika[0]][y+rika[1]]==flag&&board[x+2*rika[0]][y+2*rika[1]]==other_flag)
	{
		if(SearchFlag(x,y,other_flag))
			tempmark+=50;
	}	
	if(board[x+rika[0]][y+rika[1]]==other_flag&&board[x-rika[0]][y-rika[1]]==EMPTY)
	{
		if (SearchFlag(x-rika[0],y-rika[1],other_flag))
			tempmark-=50;
	}
	tempmark-=distance(x,y,flag);
  }
	int direction[16]={0};
	int temp =0;	  
    for (int k = 0; k < 8; k++) {
    const int* delta = DIR[k];
        int new_x = x + delta[0];
        int new_y = y + delta[1];	
        if (isInBound(new_x, new_y) && board[new_x][new_y] == EMPTY) 
		{
		  board[x][y]=EMPTY;
		  direction[temp]=AGivemark(new_x, new_y,flag);  
		  temp++;
 //         testcommand[temp].option=k;		  
		  board[x][y]=flag;
		}
    	int max=direction[0];
		for (int k = 0; k < temp; k++)
		   {
		if (direction[k]>max)
		{
			max=direction[k];
//			command.option = testcommand[k].option;
		}		
		   }
	}
  tempmark-=max;
  return tempmark;
}

/**
 * YOUR CODE BEGIN
 * 你的代码开始
 */

/**
 * You can define your own struct and variable here
 * 你可以在这里定义你自己的结构体和变量
 */


/**
 * 你可以在这里初始化你的AI
 */
void initAI(void) {

}

struct Command findValidPos(char board[BOARD_SIZE][BOARD_SIZE], int flag) {
  if (privateround<=2)
  {
  	if(privateround==1&&flag==BLACK)
  	{
          command.x = 9;
          command.y = 9;
          command.option = UP_LEFT; 
		  return command;		
	}
	if(privateround==1&&flag==WHITE&&board[8][7]!=BLACK&&board[8][8]!=BLACK&&board[8][6]!=BLACK)
  	{
          command.x = 2;
          command.y = 2;
          command.option = DOWN_RIGHT; 
		  return command;		
	}
	if(privateround==1&&flag==WHITE&&(board[8][7]==BLACK||board[8][8]==BLACK||board[8][6]==BLACK))
  	{
          command.x = 6;
          command.y = 6;
          command.option = DOWN_RIGHT; 
          privateround++;
		  return command;		
	}
  	if(privateround==2&&flag==BLACK&&board[7][7]!=WHITE&&board[6][6]==WHITE)
  	{
          command.x = 8;
          command.y = 8;
          command.option = UP_LEFT; 
		  return command;		
	}
  	if(privateround==2&&flag==WHITE&&board[4][4]!=BLACK&&board[4][3]!=BLACK&&board[4][5]!=BLACK&&board[5][5]==BLACK)
  	{
          command.x = 3;
          command.y = 3;
          command.option = DOWN_RIGHT; 
		  return command;		
	}	
  }	
    int number=0;
	Listboard(board);
    for (int x = 0; x < BOARD_SIZE; x++) {
      for (int y = 0; y < BOARD_SIZE; y++) {
        if (board[x][y] != flag) {
          continue;
        }
		testcommand[number].x=x;
		testcommand[number].y=y;
		testcommand[number].mark=Givemark(x,y,flag);
		number++;
        }
      }

		int min=testcommand[0].mark;
          command.x = testcommand[0].x;
          command.y = testcommand[0].y;
		for (int k = 0; k < number; k++)
		{
		if (testcommand[k].mark<min)
		{
		  min=testcommand[k].mark;
          command.x = testcommand[k].x;
          command.y = testcommand[k].y;
		}
	}
	int direction[16]={0};
	int temp =0;
    for (int k = 0; k < 8; k++) {
    const int* delta = DIR[k];
        int new_x = command.x + delta[0];
        int new_y = command.y + delta[1];
        if (isInBound(new_x, new_y) && board[new_x][new_y] == EMPTY) 
		{
		  board[command.x][command.y]=EMPTY;
		  direction[temp]=DGivemark(new_x, new_y,flag);
		  testcommand[temp].option=k;
		  temp++;
		  board[command.x][command.y]=flag;
		}
	}

    	int max=direction[0];
		command.option = testcommand[0].option;
		for (int k = 0; k < temp; k++)
		   {
		if (direction[k]>max)
		{
			command.option = testcommand[k].option;
			max=direction[k];
		}
	}

     return command;
}

/**
 * 轮到你落子。
 * 棋盘上0表示空白，1表示黑棋，2表示白旗
 * me表示你所代表的棋子(1或2)
 * 你需要返回一个结构体Command，在x属性和y属性填上你想要移动的棋子的位置，option填上你想要移动的方向。
 */
struct Command aiTurn(char board[BOARD_SIZE][BOARD_SIZE], int me) {
  /*
   * TODO：在这里写下你的AI。
   * 这里有一个示例AI，它只会寻找第一个可下的位置进行落子。
   */
  struct Command preferedPos = findValidPos(board, me);

  return preferedPos;
}

/**
 * 你的代码结束
 */


BOOL place(int x, int y, OPTION option, int cur_flag) {
  // 移动之前的位置没有我方棋子
  if (board[x][y] != cur_flag) {
    return FALSE;
  }

  int new_x = x + DIR[option][0];
  int new_y = y + DIR[option][1];
  // 移动之后的位置超出边界, 或者不是空地
  if (!isInBound(new_x, new_y) || board[new_x][new_y] != EMPTY) {
    return FALSE;
  }

  board[x][y] = EMPTY;
  board[new_x][new_y] = cur_flag;

  int cur_other_flag = 3 - cur_flag;

  // 挑
  int intervention_dir[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
  for (int i = 0; i < 4; i++) {
    int x1 = new_x + intervention_dir[i][0];
    int y1 = new_y + intervention_dir[i][1];
    int x2 = new_x - intervention_dir[i][0];
    int y2 = new_y - intervention_dir[i][1];
    if (isInBound(x1, y1) && isInBound(x2, y2) && board[x1][y1] == cur_other_flag && board[x2][y2] == cur_other_flag) {
      board[x1][y1] = cur_flag;
      board[x2][y2] = cur_flag;
    }
  }

  // 夹
  int custodian_dir[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
  for (int i = 0; i < 8; i++) {
    int x1 = new_x + custodian_dir[i][0];
    int y1 = new_y + custodian_dir[i][1];
    int x2 = new_x + custodian_dir[i][0] * 2;
    int y2 = new_y + custodian_dir[i][1] * 2;
    if (isInBound(x1, y1) && isInBound(x2, y2) && board[x2][y2] == cur_flag && board[x1][y1] == cur_other_flag) {
      board[x1][y1] = cur_flag;
    }
  }

  return TRUE;
}


//........
//........
//..XXXX..
//XXXX....
//....OOOO
//..OOOO..
//........
//........
void start() {
  memset(board, 0, sizeof(board));

  for (int i = 0; i < 3; i++) {
    board[2][2 + i] = WHITE;
    board[6][6 + i] = WHITE;
    board[5][3 + i] = BLACK;
    board[9][7 + i] = BLACK;
  }

  for (int i = 0; i < 2; i++) {
    board[8 + i][2] = WHITE;
    board[2 + i][9] = BLACK;
  }

  initAI();
}

void turn() {
  // AI
  struct Command command = aiTurn((char (*)[BOARD_SIZE])board, me_flag);
  place(command.x, command.y, command.option, me_flag);
  printf("%d %d %d\n", command.x, command.y, command.option);
  fflush(stdout);
}

void end(void) {

}

void loop() {
//  freopen("../input", "r", stdin);
  while (TRUE)
  {
    memset(buffer, 0, sizeof(buffer));
    gets(buffer);

    if (strstr(buffer, START))
    {
      char tmp[MAX_BYTE] = {0};
      sscanf(buffer, "%s %d", tmp, &me_flag);
      other_flag = 3 - me_flag;
	  privateround=0;
      start();
      printf("OK\n");
      fflush(stdout);
    }
    else if (strstr(buffer, PLACE))
    {
      char tmp[MAX_BYTE] = {0};
      int x, y;
      OPTION option;
      sscanf(buffer, "%s %d %d %d", tmp, &x, &y, &option);
      place(x, y, option, other_flag);
    }
    else if (strstr(buffer, TURN))
    {
      privateround++;	
      turn();
    }
    else if (strstr(buffer, END))
    {
      char tmp[MAX_BYTE] = {0};
      int x;
      sscanf(buffer, "%s %d", tmp, &x);
      end();
    }
//    printBoard();
  }
}


int main(void) {
  loop();
  return 0;
}