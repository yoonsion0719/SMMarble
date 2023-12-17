//
//  smm_node.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include "smm_common.h"
#include "smm_object.h"
#include <string.h>

#define MAX_NODETYPE    7
#define MAX_GRADE       9
#define MAX_NODE        100


static char smmNodeName[SMMNODE_TYPE_MAX][MAX_CHARNAME] = {
       "강의",
       "식당",
       "실험실",
       "집",
       "실험실로이동",
       "음식찬스",
       "축제시간"
};

char* smmObj_getTypeName(int type)
{
      return (char*)smmNodeName[type];
}


typedef enum smmObjGrade {
	smmObjGrade_Ap =0,
	smmObjGrade_A0,
	smmObjGrade_Am,
	smmObjGrade_Bp,
	smmObjGrade_B0,
	smmObjGrade_Bm,
	smmObjGrade_Cp,
	smmObjGrade_C0,
	smmObjGrade_Cm
}smmObjGrade_e;


//1.구조체 형식 정의
typedef struct smmObject {
	char name[MAX_CHARNAME];
	smmObjType_e objType;
	int type;
	int credit;
	int energy;
	smmObjGrade_e grade;
}smmObject_t;
 

//static smmObject_t smm_node[MAX_NODE];
//static int smmObj_noNode=0;

//3. 기존 함수의 수정 및 새로 작성 
//object generation
void* smmObj_genObject(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade)
{
	smmObject_t* ptr;
    
    ptr= (smmObject_t*)malloc(sizeof(smmObject_t));
    
    //strcpy(smm_node[smmObj_noNode].name, name);
    strcpy(ptr->name, name);
    ptr->objType=objType;
    ptr->type=type;
    ptr->credit=credit;
    ptr->energy=energy;
    ptr->grade=grade;
    
    return ptr;
}

//3. 기존 함수의 수정 및 새로 작성  
char* smmObj_getNodeName(void* obj)
{
	smmObject_t* ptr = (smmObject_t*)obj;
    return ptr->name;
}

//3. 기존 함수의 수정 및 새로 작성 
int smmObj_getNodeType(void* obj)
{
    //return smm_node[node_nr].type;
    smmObject_t* ptr = (smmObject_t*)obj;
    return ptr->type;
}

int smmObj_getNodeCredit(void* obj)
{
    //return smm_node[node_nr].credit;
    smmObject_t* ptr = (smmObject_t*)obj;
    return ptr->credit;
}

int smmObj_getNodeEnergy(void* obj)
{
    //return smm_node[node_nr].energy;
    smmObject_t* ptr = (smmObject_t*)obj;
    return ptr->energy;
}
#if 0


//member retrieving



//element to string
char* smmObj_getNodeName(smmNode_e type)
{
    return smmNodeName[type];
}

char* smmObj_getGradeName(smmGrade_e grade)
{
    return smmGradeName[grade];
}

#endif
