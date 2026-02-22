#ifndef __MENU_H
#define __MENU_H

#define MAX_NAME_LENGTH			32
#define MAX_MENU_LIST_LENGTH	32

struct Menu_Item {
	char Name[MAX_NAME_LENGTH];
	void (*Function)(void);
	struct Menu_List *NextList;
	
	int16_t Type;
	
	int16_t ON_OFF;
	
	float Num;
	float NumMax;
	float NumMin;
	float NumStep;
	int16_t NumSetFlag;
	char *NumDisplayFormat[10];
};

struct Menu_List {
	char Name[MAX_NAME_LENGTH];
	int16_t WindowIndex;
	int16_t WindowLength;
	int16_t ItemIndex;
	int16_t ItemLength;
	struct Menu_Item Items[32];
};

extern struct Menu_List ParamMenu;

void Menu_Down(void);
void Menu_Up(void);
void Menu_Enter(void);
void Menu_Back(void);

void Menu_Init(void);
void Menu_Loop(void);
void Menu_Exit(void);

#endif
