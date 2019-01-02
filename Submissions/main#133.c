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

struct Boardlist
{
	int blacklist[16][2];
	int whitelist[16][2];
	int black;
	int white;
};

struct Testcommand
{
  int x;
  int y;
  OPTION option;
  int mark;
};

char buffer[MAX_BYTE] = {0};
char board[BOARD_SIZE][BOARD_SIZE] = {{0}};
int me_flag;
int other_flag;
int privateround;
const int DIR[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };
char virtualboard[DEPTH+1][BOARD_SIZE][BOARD_SIZE]={{{0}}};
struct Boardlist boardlist;

BOOL isInBound(int x, int y) {
  return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

void place(int x, int y, OPTION option, int cur_flag, char placeboard[BOARD_SIZE][BOARD_SIZE]) {
  int new_x = x + DIR[option][0];
  int new_y = y + DIR[option][1];	
  placeboard[x][y] = EMPTY;
  placeboard[new_x][new_y] = cur_flag;
  int cur_other_flag = 3 - cur_flag;
  int intervention_dir[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
  for (int i = 0; i < 4; i++) {
    int x1 = new_x + intervention_dir[i][0];
    int y1 = new_y + intervention_dir[i][1];
    int x2 = new_x - intervention_dir[i][0];
    int y2 = new_y - intervention_dir[i][1];
    if (isInBound(x1, y1) && isInBound(x2, y2) && placeboard[x1][y1] == cur_other_flag && placeboard[x2][y2] == cur_other_flag) {
      placeboard[x1][y1] = cur_flag;
      placeboard[x2][y2] = cur_flag; 
    }
  }
  int custodian_dir[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
  for (int i = 0; i < 8; i++) {
    int x1 = new_x + custodian_dir[i][0];
    int y1 = new_y + custodian_dir[i][1];
    int x2 = new_x + custodian_dir[i][0] * 2;
    int y2 = new_y + custodian_dir[i][1] * 2;
    if (isInBound(x1, y1) && isInBound(x2, y2) && placeboard[x2][y2] == cur_flag && placeboard[x1][y1] == cur_other_flag) {
      placeboard[x1][y1] = cur_flag;  
    }
  }
}

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

void Listboard(char virtualboard[DEPTH][BOARD_SIZE][BOARD_SIZE],int depth)
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
 
int distance(char virtualboard[DEPTH][BOARD_SIZE][BOARD_SIZE],int depth,int flag)
{
	int array[16];
	int arraysum=0;
	if (flag==BLACK)
	{		
		for (int j=0;j<boardlist.black;j++)
		{
			int min=12;
		for(int i=0;i<boardlist.white;i++)
		{
			array[i]=max(abs(boardlist.whitelist[i][0]-boardlist.blacklist[j][0]),abs(boardlist.whitelist[i][1]-boardlist.blacklist[j][0]));
		}
		for(int i=0;i<boardlist.white;i++)
		{
			if (array[i]<min)
			{
				min=array[i];
			}
		}
		arraysum+=min;
	}
	}
	if (flag==WHITE)
	{		
		for (int j=0;j<boardlist.white;j++)
		{
			int min=12;
		for(int i=0;i<boardlist.black;i++)
		{
			array[i]=max(abs(boardlist.blacklist[i][0]-boardlist.whitelist[j][0]),abs(boardlist.blacklist[i][1]-boardlist.whitelist[j][0]));
		}
		for(int i=0;i<boardlist.black;i++)
		{
			if (array[i]<min)
			{
				min=array[i];
			}
		}
		arraysum+=min;
	}
	}
		return arraysum;
}

int virtualmark(char virtualboard[DEPTH+1][BOARD_SIZE][BOARD_SIZE],int depth,int flag)
{
	int tempmark=0;
	Listboard(virtualboard,depth);
	if (flag==BLACK)
	tempmark+=1000*boardlist.black;
    else 
	tempmark+=1000*boardlist.white;
    tempmark-=distance(virtualboard,depth,flag);
	return tempmark;
}

int virtualgame(char virtualboard[DEPTH+1][BOARD_SIZE][BOARD_SIZE],int flag,int x,int y,int option,int alpha,int beta,int depth)
{
    for (int i = 0; i < BOARD_SIZE; i++) 
    {
        for (int j = 0; j < BOARD_SIZE; j++) 
	    {
		    virtualboard[depth][i][j]=virtualboard[depth-1][i][j];
	    }
    }
    place(x, y, option, 3-flag, *(virtualboard+depth));
    if (depth==DEPTH)
    {
	    return virtualmark(virtualboard,depth,me_flag);
    }
    int tempmark;
    for (int k = 0; k < 8; k++) 
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
				    depth++;
   	                tempmark=-virtualgame(virtualboard,3-flag,i,j,k,-beta,-alpha,depth);
				    depth--;
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
    for (int x = 0; x < BOARD_SIZE; x++) 
	{
      for (int y = 0; y < BOARD_SIZE; y++) 
	  {
		  virtualboard[0][x][y]=board[x][y];
	  }
	}
  int beta=10000;
  int alpha=0;
  for (int k = 0; k < 8; k++) 
  {
    const int* delta = DIR[k];
    for (int x = 0; x < BOARD_SIZE; x++) 
	{
      for (int y = 0; y < BOARD_SIZE; y++) 
	  {
        if (board[x][y] != flag) 
		{
          continue;
        }
        int new_x = x + delta[0];
        int new_y = y + delta[1];
        if (isInBound(new_x, new_y) && board[new_x][new_y] == EMPTY) 
		{	  
  	      tempmark=-virtualgame(virtualboard,3-flag,x,y,k,-beta,-alpha,1);
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
  return command;
}

void start() 
{
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
}

void turn() {
  struct Command command = aiTurn((const char (*)[BOARD_SIZE])board, me_flag);
  place(command.x, command.y, command.option, me_flag, board);
  printf("%d %d %d\n", command.x, command.y, command.option);
  fflush(stdout);
}

int main() 
{
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
      start(me_flag);
      printf("OK\n");
      fflush(stdout);
    }
    else if (strstr(buffer, PLACE))
    {
      char tmp[MAX_BYTE] = {0};
      int x, y;
      OPTION option;
      sscanf(buffer, "%s %d %d %d", tmp, &x, &y, &option);
      place(x, y, option, other_flag,board);
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
    }
  }
  return 0;
}