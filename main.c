//
//main.c
//SMMarble
//
//

#include <time.h>
#include <string.h>

#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"

//
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;

//1. player struct

typedef struct player {
	int credit;
	int energy;
	int position;
	char name[MAX_CHARNAME];
	int accumCredit;
	int flag_graduate;
}player_t;

static player_t *cur_player;

#if 0
void generatePlayers(int n, int initEnergy);
//generate a new player
int isGraduated(void);
//check if any player is graduated
void goForward(int player, int step);
//make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void);
//print all player status at the beginning of each turn
float calcAverageGrade(int player);
//calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit);
//take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName);
//find the grade from the player's grade history
void printGrades(int player);
//print all the grade history of the player
#endif

#if 0
//print all the grade history of the player
void printGrades(int player)
{
	int i;
	void *gradePtr;
	for (i=0;i<smmdb_len(LISTNO_OFFSET_GRADE+player);i++)
	{
		gradePtr=smmdb_getData(LISTNO_OFFSET_GRADE+player,i);
		printf("%s : %i\n", smmObj_getNodeName(gradePtr), );//lecture's name & palyer's grade
	}
}
#endif

void printPlayerStatus(void)//print all player status at the beginning of each turn
{
	int i;
	for (i=0;i<player_nr;i++)//player_nr ???? name,credit,energy,posit 
	{
		printf("%s : credit %i, energy %i, position %i\n",
		cur_player[i].name,
		cur_player[i].accumCredit,
		cur_player[i].energy,
		cur_player[i].position);
	}
}

void generatePlayers(int n, int initEnergy)//generate a new player
{
	int i;
	//n time loop
	for (i=0;i<n;i++)
	{
		//input name
		printf("input player %i's name: ",i);
		scanf("%s", cur_player[i].name);
		fflush(stdin);
		
		//set position
		cur_player[i].position=0;
		
		//set energy
		cur_player[i].energy=initEnergy;
		
		//set etc
		cur_player[i].accumCredit=0;
		cur_player[i].flag_graduate=0;
	}
}

//Operates based on user input
int rolldie(int player)
{
	char c;
	printf("Press any key to roll a die (press g to see grade): ");
	c=getchar();
	fflush(stdin);
	
	#if 0
	//to print out grades when a user enters 'g'
	if (c=='g')
		printGrades(player);
	#endif
		
	return (rand()%MAX_DIE + 1);
}


// actionNode Modification : 
//SMMNODE_TYPE_RESTAURANT << �׳� ������ ���� �ΰ� 
//SMMNODE_TYPE_LABORATORY << �ֻ��� ������ Ż���ϴ� ����  
//SMMNODE_TYPE_HOME <<< flag_graduate Ȯ�� �� ������ ���� 
//SMMNODE_TYPE_GOTOLAB <<< ����� ���� �̵� 
//SMMNODE_TYPE_FOODCHANCE << ���ɽð�,���Ľð��� ����ī��� ������ ���� 
//SMMNODE_TYPE_FESTIVAL << �������� �̼� ��� �� ���� 
// 


//action code when a player stays at a node
void actionNode(int player)
{
	void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
	int type=smmObj_getNodeType(boardPtr);
	char *name=smmObj_getNodeName(boardPtr);
	void *gradePtr;
	
	//For lectures, you must save the player's credit and energy changes
	switch(type)
	{
		case SMMNODE_TYPE_LECTURE:
			cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
			cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr);
			
			//grade Randomly generation
			gradePtr = smmObj_genObject(name, smmObjType_grade, 0, 
						smmObj_getNodeCredit(boardPtr), 0, rand()%9);
			//saving to a list 
			smmdb_addTail(LISTNO_OFFSET_GRADE, gradePtr);
			break;
			
	
		#if 0
		case SMMNODE_TYPE_RESTAURANT:
			break;
		
		case SMMNODE_TYPE_LABORATORY:
			break;
			
		case SMMNODE_TYPE_HOME:
			break;
			
		case SMMNODE_TYPE_GOTOLAB:
			break;
			
		case SMMNODE_TYPE_FOODCHANCE:
			break;
			
		case SMMNODE_TYPE_FESTIVAL:
			break;
		#endif
		
		default:	
			break;	
			
	}
	

}

// Need 3 modifications
// 1. Circulation structure -ok! 
//2. Output of node name passing by when moving -ok!
//3. Supplement energy when passing home -actionNode���� ������ ���� 

void goForward(int player, int step)
{
	void *boardPtr; 
	int i;
	
	for (i=0;i<step;i++)
	{
		//check position & go 1 step
		if (cur_player[player].position == board_nr-1)
		{
			cur_player[player].position = 0;//for Circulation structure
			//'actionNode' �߰��ؼ� ������ ������ �� �ֵ��� �ϱ� 
		}
		else
		{
			cur_player[player].position += 1;
		}
		
		//player's new position & print node Name
		boardPtr=smmdb_getData(LISTNO_NODE, cur_player[player].position);
		//������ �ʿ� ����!!! 
		printf("%s\n", smmObj_getNodeName(boardPtr));
	}
	
	
	//to output information of the node reached by the player
	printf("%s go node %i (name: %s)\n",
	cur_player[player].name,
	cur_player[player].position,
	smmObj_getNodeName(boardPtr));
}

//if isGraduated(void) is 1, there is player who graduated
int isGraduated(void)
{
	int flag=0;
	int i;
	for (i=0;i<player_nr;i++)
	{
		if (cur_player[i].flag_graduate == 1)
			flag=1;
			break;
	}
	return flag;
}

int main(void){
	
	FILE* fp;
	char name[MAX_CHARNAME];
	int type;
	int credit;
	int energy;
	int i;
	int initEnergy;
	int turn=0;
	
	board_nr =0;
	food_nr =0;
	festival_nr =0;
	
	srand(time(NULL));
	
	//1. import parameters
	//1.1 boardConfig
	if ((fp = fopen(BOARDFILEPATH,"r"))==NULL)
	{
		printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
	}
	
	printf("Reading board component......\n");
    while ( fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4 ) //read a node parameter set
    {
        //store the parameter set
        //
        void *boardObj = smmObj_genObject(name, smmObjType_board, type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj); //?
        
		if (type==SMMNODE_TYPE_HOME)
        	initEnergy=energy;
		board_nr++;
    }
    fclose(fp);
    printf("Total number of board nodes : %i\n", board_nr);
    
    //print board num,name,type,
    for (i=0;i<board_nr;i++)
    {
    	void *boardObj=smmdb_getData(LISTNO_NODE, i);
    	
    	printf("node %i : %s, %i(%s), credit %i, energy %i\n",
		i, smmObj_getNodeName(boardObj), 
		smmObj_getNodeType(boardObj), smmObj_getTypeName(smmObj_getNodeType(boardObj)),
		 smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
    		
	}
	
	#if 0
	//1.2. food card config
	if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while () //read a food parameter set
    {
        //store the parameter set
    }
    fclose(fp);
    printf("Total number of food cards : %i\n", food_nr);
    
    
    
    //1.3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while () //read a festival card string
    {
        //store the parameter set
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);
    #endif
    
    //2. Player configuration
    do
    {
    	//input player number to player_nr
    	printf("input player no.: ");
    	scanf("%d", &player_nr);
    	fflush(stdin);
	}
	while (player_nr < 0 || player_nr >  MAX_PLAYER);
	
	//
	cur_player=(player_t*)malloc(player_nr*sizeof(player_t));
	generatePlayers(player_nr, initEnergy);
	
	
	
	//3. SM Marble game starts
	while (isGraduated()==0)//is anybody graduated?
	{
		int die_result;
		
		//4.1. initial printing
		printPlayerStatus();
		
		//4.2. die rolling (if not in experiment)
		die_result = rolldie(turn);
		
		//4.3. go forward
		goForward(turn,die_result);
		
		//4.4. take action at the destination node of the board
		actionNode(turn);
		
		//4.5. next turn
		turn = (turn+1)%player_nr;
		
	}
	
	free(cur_player);
	system("PAUSE");
	return 0;
	
	
}


