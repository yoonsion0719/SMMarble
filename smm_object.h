//
//smm_object.h
//SMMarble object
//
//

#ifndef object_h
#define smm_object_h

#define SMMNODE_TYPE_LECTURE            0
#define SMMNODE_TYPE_RESTAURANT         1
#define SMMNODE_TYPE_LABORATORY         2
#define SMMNODE_TYPE_HOME               3
#define SMMNODE_TYPE_GOTOLAB            4
#define SMMNODE_TYPE_FOODCHANCE         5
#define SMMNODE_TYPE_FESTIVAL           6

#define SMMNODE_TYPE_MAX                7

typedef enum smmObjType {
	smmObjType_board =0,
	smmObjType_card,
	smmObjType_grade
}smmObjType_e;

 //define grade enum
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

 
//object generation
void* smmObj_genObject(char* name, smmObjType_e objType, int type, 
int credit, int energy, smmObjGrade_e grade);

//Get Object information
char* smmObj_getNodeName(void* obj);
int smmObj_getNodeType(void* obj);
int smmObj_getNodeCredit(void* obj);
int smmObj_getNodeEnergy(void* obj);
//from gradeObj get information
int smmObj_getGradeGrade(void* obj);

//element to string
char* smmObj_getTypeName(int type);
char* smmObj_getGradeName(smmObjGrade_e grade);


#endif /* smm_object_h */
