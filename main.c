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
	int flag_experiment; //실험중 여부를 나타내는 변수 
	int flag_graduate;
}player_t;

static player_t *cur_player;

#if 0
void generatePlayers(int n, int initEnergy);
//generate a new player
int isGraduated(void);
//check if any player is graduated
int rolldie(int player)
//Operates based on user input
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



//print all the grade history of the player
void printGrades(int player)
{
	int i;
	void *gradePtr;
	for (i=0;i<smmdb_len(LISTNO_OFFSET_GRADE+player);i++)
	{
		gradePtr=smmdb_getData(LISTNO_OFFSET_GRADE+player,i);
		printf("Lecture's name and credit: %s (%i) Grade: %s\n", 
		smmObj_getNodeName(gradePtr), smmObj_getNodeCredit(gradePtr), smmObj_getGradeGrade(gradePtr));//lecture's name & palyer's grade
	}
}

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


// actionNode Modification : 
//SMMNODE_TYPE_RESTAURANT << 그냥 에너지 보충 인가 
//SMMNODE_TYPE_LABORATORY << 주사위 굴려서 탈출하는 동작  
//SMMNODE_TYPE_HOME <<< flag_graduate 확인 및 에너지 보충 
//SMMNODE_TYPE_GOTOLAB <<< 실험실 노드로 이동 
//SMMNODE_TYPE_FOODCHANCE << 점심시간,간식시간에 음식카드로 에너지 보충 
//SMMNODE_TYPE_FESTIVAL << 랜덤으로 미션 출력 후 수행 
// 

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
				//2. rolldie 
				int exp_result=rolldie(player);
				//3. If over reference, release the experiment 
				if (exp_result >= exp_success)
					cur_player[player].flag_experiment = 0;
			}
			break;
			
		case SMMNODE_TYPE_HOME:
			//flag_graduate modification 
			if (cur_player[player].accumCredit >= GRADUATE_CREDIT)
			{
				cur_player[player].flag_graduate = 1;
				isGraduated();//player's Graduation!
			}
			//energy replenishment
			cur_player[player].energy += smmObj_getNodeEnergy(boardPtr);
			break;
			
		case SMMNODE_TYPE_GOTOLAB:
			//1. Switch player to experimental state 
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
			printf("Wow!\nI got %s!\n", smmObj_getNodeName(foodPtr));
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
	
	for (i=0;i<step;i++)
	{
		//check position & go 1 step
		if (cur_player[player].position == board_nr-1)
		{
			cur_player[player].position = 0;//for Circulation structure
			actionNode(player);//집에서 필요한 'actionNode' 추가 
		}
		else
		{
			cur_player[player].position += 1;
		}
		
		//player's new position & print node Name
		boardPtr=smmdb_getData(LISTNO_NODE, cur_player[player].position);
		//수정할 필요 있음!!! 
		printf("%s go to %s\n", cur_player[player].name, smmObj_getNodeName(boardPtr));
	}
	
	
	//to output information of the node reached by the player
	printf("%s's destination is %i (name: %s)\n",
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
	int nextTurn;//다음 턴을 확인하는 변수임 
	
	int foodtype=0; //음식 유형을 저장하기 위해서 만든 변수임요
	
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
        void *foodObj = smmObj_genObject(name, smmObjType_card, foodtype, 0, energy, 0);
        smmdb_addTail(LISTNO_FOODCARD, foodObj);
        foodtype++;
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
		
		//4.2. initial printing
		printPlayerStatus();
		
		
		//4.3. die rolling (if not in experiment)
		if(cur_player[turn].flag_experiment == 0)
			die_result = rolldie(turn);//실험실에서 실험중일때를 제외하고 
		
		//4.4. go forward
		if(cur_player[turn].flag_experiment == 0)
			goForward(turn,die_result);//실험실에서 실험중일때를 제외하고
		
		//4.5. take action at the destination node of the board
		actionNode(turn);
		
		//4.6. next turn
		nextTurn = (turn+1)%player_nr;
		
	}
	
	//5. game-ending action
	//Congratulations to the winner
	printf("Congratulations on your graduation! %s\n", cur_player[turn].name);
	//Print out the name, credit, and grade of the course taken by the graduated player
	printGrades(turn);
	
	free(cur_player);
	system("PAUSE");
	return 0;
	
	
}


