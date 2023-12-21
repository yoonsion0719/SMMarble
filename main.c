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
	int flag_experiment; //실험중 여부를 나타내는 변수 
	int flag_graduate;
}player_t;

static player_t *cur_player;


void generatePlayers(int n, int initEnergy);
//generate a new player
int isGraduated(void);
//check if any player is graduated
int rolldie(int player);
//Operates based on user input
void goForward(int player, int step);
//make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void);
//print all player status at the beginning of each turn
void printGrades(int player);
//print all the grade history of the player



//print all the grade history of the player
void printGrades(int player)
{
	int i;
	void *gradePtr;
	printf("<%s's course history>\n", cur_player[player].name);
	for (i=0;i<smmdb_len(LISTNO_OFFSET_GRADE+player);i++)
	{
		gradePtr=smmdb_getData(LISTNO_OFFSET_GRADE+player,i);
		printf("Lecture's name: %s (credit: %i) Grade: %s\n", smmObj_getNodeName(gradePtr), 
		smmObj_getNodeCredit(gradePtr), smmObj_getGradeName(smmObj_getGradeGrade(gradePtr)));
	}
	printf("\n");
}

//print all player status at the beginning of each turn
void printPlayerStatus(void)
{
	int i;
	for (i=0;i<player_nr;i++)
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
		cur_player[i].flag_experiment=0;
		
	}
}

//Operates based on user input
int rolldie(int player)
{
	char c;
	printf("Press any key to roll a die (press g to see grade): ");
	c=getchar();
	fflush(stdin);
	
	//to print out grades when a user enters 'g'
	if (c=='g')
		printGrades(player);
	
	return (rand()%MAX_DIE + 1);
}



//action code when a player stays at a node
void actionNode(int player)
{
	void *boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
	int type=smmObj_getNodeType(boardPtr);
	char *name=smmObj_getNodeName(boardPtr);
	void *gradePtr;
	void *foodPtr;
	void *festivalPtr;
	int i;
	char c;
	int flag_lec;//Whether to take a lecture or not
	
	//For lectures, you must save the player's credit and energy changes
	switch(type)
	{
		case SMMNODE_TYPE_LECTURE:
			//1. Decide whether to take a course
			flag_lec=1;
			
			//1.1.Energy Check
			if (cur_player[player].energy >= smmObj_getNodeEnergy(boardPtr))
			{
				//1.2. Confirmation of course history
				for (i=0;i<smmdb_len(LISTNO_OFFSET_GRADE+player);i++)
				{
					gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE+player, i);
					if (smmObj_getNodeName(gradePtr) == name)
						flag_lec=0;
						break;
				}
				//Confirmation of intention to take the course
				if (flag_lec==1)
				{
					//Decide whether to take a course
					printf("Would you like to take a lecture %s?\nTo drop, press 'd': ", name);
					c=getchar();
					if (c !='d')
					{
						//takeLecture
						printf("\nYou decided to take the course\n");
						//1. Credit fluctuation
						cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
						cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr);
						//2. grade Randomly generation
						gradePtr = smmObj_genObject(name, smmObjType_grade, 0, 
						smmObj_getNodeCredit(boardPtr), 0, rand()%9);
						//3. saving to a list 
						smmdb_addTail(LISTNO_OFFSET_GRADE+player, gradePtr);
				
					}
				}
			}
			break;
		
		
		case SMMNODE_TYPE_RESTAURANT:
			//energy replenishment
			cur_player[player].energy += smmObj_getNodeEnergy(boardPtr);
			break;
		
		case SMMNODE_TYPE_LABORATORY:
			//When the player is in an experimental state, conduct the experiment
			if (cur_player[player].flag_experiment == 1)
			{
				//1. Randomize experiment success thresholds
				int exp_success= rand()%6;
				printf("Standard of experiment success: %i\n", exp_success);
				//2. experiment result 
				int exp_result=rolldie(player);
				printf("%s's Experiment Result: %i\n",cur_player[player].name, exp_result);
				//3. If over reference, release the experiment 
				if (exp_result >= exp_success)
				{
					printf("The experiment was successful!\n");
					cur_player[player].flag_experiment = 0;
				}
				else
				{
					printf("Experiment failed..\n");
				}
			}
			break;
			
		case SMMNODE_TYPE_HOME:
			//flag_graduate modification
			if (cur_player[player].accumCredit >= GRADUATE_CREDIT)
			{
				cur_player[player].flag_graduate = 1;
				break; 
			}
			//energy replenishment
			cur_player[player].energy += smmObj_getNodeEnergy(boardPtr);
			break;
			
		case SMMNODE_TYPE_GOTOLAB:
			//1. Switch player to experimental state 
			printf("Player %s have to go labatory and do Experiment\n", cur_player[player].name);
			cur_player[player].flag_experiment=1;
			//2. Finding laboratory nodes (assuming there is one laboratory)
			for (i=0;i<board_nr;i++)
			{
				boardPtr= smmdb_getData(LISTNO_NODE, i);
				if (smmObj_getNodeType(boardPtr) == SMMNODE_TYPE_LABORATORY) 
				{
					//3. Move to the laboratory
					cur_player[player].position = i;
					break;
				}
			}
			
			break;
			
		case SMMNODE_TYPE_FOODCHANCE:
			//Choose a food card at random and get energy
			foodPtr=smmdb_getData(LISTNO_FOODCARD,rand()%food_nr);
			printf("Wow! Player %s got %s!\n", cur_player[player].name, smmObj_getNodeName(foodPtr));
			//energy replenishment from foodcard
			cur_player[player].energy += smmObj_getNodeEnergy(foodPtr);
			break;
		
		case SMMNODE_TYPE_FESTIVAL:
			
			//Randomly output the mission
			festivalPtr = smmdb_getData(LISTNO_FESTCARD, rand()%festival_nr);
			printf("Mission to perform: %s\n", smmObj_getNodeName(festivalPtr));
			//Get input 
			c=getchar();
			fflush(stdin);
			
			break;
		
		default:	
			break;	
			
	}
	

}

// Need 2 motion
// 1. Circulation structure -ok! 
//2. Output of node name passing by when moving -ok! 

void goForward(int player, int step)
{
	void *boardPtr; 
	int i;
	
	printf("Dice's result is : %i\n\n", step);
	for (i=0;i<step;i++)
	{
		//check position & go 1 step
		if (cur_player[player].position == board_nr-1)
		{
			cur_player[player].position = 0;//for Circulation structure
			actionNode(player);//In home, we need actionNode
		}
		else
		{
			cur_player[player].position += 1;
		}
		
		//player's new position & print node Name
		boardPtr=smmdb_getData(LISTNO_NODE, cur_player[player].position); 
		printf("%s go to %s\n", cur_player[player].name, smmObj_getNodeName(boardPtr));
	}
	
	
	//to output information of the player's destination
	printf("%s's destination is %i (name: %s)\n\n",
	cur_player[player].name,
	cur_player[player].position,
	smmObj_getNodeName(boardPtr));
}


//check is there player who graduated
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
	int nextTurn;//Variables that confirm the next turn
	
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
	
	
	//1.2. food card config
	if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n"); 
    while ( fscanf(fp, "%s %i", name, &energy) == 2 ) //read a food parameter set
    {
        //store the parameter set
        void *foodObj = smmObj_genObject(name, smmObjType_card, 0, 0, energy, 0);
        smmdb_addTail(LISTNO_FOODCARD, foodObj);
        food_nr++;
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
    while( fscanf(fp, "%s", name) == 1 )//read a festival card string
    {
    	//store the parameter set
        void *festivalObj = smmObj_genObject(name, smmObjType_card, 0, 0, 0 , 0);
        smmdb_addTail(LISTNO_FESTCARD, festivalObj);
        festival_nr++;
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n\n", festival_nr);
    
    
    //2. Player configuration
    do
    {
    	//input player number to player_nr
    	printf("input player no.: ");
    	scanf("%d", &player_nr);
    	fflush(stdin);
	}
	while (player_nr < 0 || player_nr >  MAX_PLAYER);
	
	//generate Players
	cur_player=(player_t*)malloc(player_nr*sizeof(player_t));
	generatePlayers(player_nr, initEnergy);
	
	
	
	//3. SM Marble game starts
	while (isGraduated()==0)//is anybody graduated?
	{
		int die_result;
		
		//4.1. Check turn
		turn=nextTurn;
		printf("\nPlayer %s's turn!\n",cur_player[turn].name);
		
		//4.2. initial printing
		printPlayerStatus();
		
		
		//4.3. die rolling (if not in experiment)
		if(cur_player[turn].flag_experiment == 0)
			die_result = rolldie(turn);//실험실에서 실험중일때를 제외하고 
		
		//4.4. go forward
		if(cur_player[turn].flag_experiment == 0)
			goForward(turn,die_result);//실험실에서 실험중일때를 제외하고
		
		//4.5. take action at the destination node of the board
		if (isGraduated() == 0)//
			actionNode(turn);
		
		//4.6. next turn
		nextTurn = (turn+1)%player_nr;
		
	}
	
	//5. Game-ending action
	
	//Congratulations to the winner
	printf("\n<<<Congratulations on your graduation, %s!!>>>\n\n", cur_player[turn].name);
	//Print out the name, credit, and grade of the course taken by the graduated player
	printGrades(turn);
	
	free(cur_player);
	system("PAUSE");
	return 0;
	
	
}


