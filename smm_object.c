//
//smm_object.c
//SMMarble
//
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


 
 //define object struct
 typedef struct smmObject {
 	char name[MAX_CHARNAME];
 	smmObjType_e objType;
 	int type;
 	int credit;
 	int energy;
 	smmObjGrade_e grade;
 }smmObject_t;
 
 
//object generation
void* smmObj_genObject(char* name, smmObjType_e objType, int type, 
int credit, int energy, smmObjGrade_e grade)
{
	smmObject_t* ptr;
	ptr= (smmObject_t*)malloc(sizeof(smmObject_t));
	
	strcpy(ptr->name, name);
	ptr->objType=objType;
	ptr->type=type;
	ptr->credit=credit;
	ptr->energy=energy;
	ptr->grade=grade;
	
	return ptr;
}


//노드 유형의 이름 출력 
char* smmObj_getTypeName(int type)
{
	return (char*)smmNodeName[type];
 } 
 


//node name
char* smmObj_getNodeName(void* obj)
{
	smmObject_t* ptr= (smmObject_t*)obj;
	return ptr->name;
}
//node type
int smmObj_getNodeType(void* obj)
{
	smmObject_t* ptr= (smmObject_t*)obj;
	return ptr->type;
}
//node credit
int smmObj_getNodeCredit(void* obj)
{
	smmObject_t* ptr= (smmObject_t*)obj;
	return ptr->credit;
}
//node energy
int smmObj_getNodeEnergy(void* obj)
{
	smmObject_t* ptr= (smmObject_t*)obj;
	return ptr->energy;
}

#if 0
//grade name<- grade_e
char* smmObj_getGradeName(smmGrade_e grade)
{
	return smmGradeName[grade];
}
#endif
