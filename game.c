#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#include "mtg.h"
#include "inc\fmod.h"

#define Ver "1.7.1"

FMOD_SYSTEM *g_System;		// ������� ����� ���� �Լ�
FMOD_SOUND *g_Sound[6];		// �������(ȿ����)�� ����
FMOD_CHANNEL *g_Channel[2]; // ��������� ����

#define MY_W 12+2  // ��Ʈ������ �ʺ�
#define MY_H 23+2  // ��Ʈ������ ����
#define MY_WALL 0  // �� = 0
#define MY_BLANK 1 // �� ���� = 1
#define MY_BLOCK 2 // �� = 2

#define MY_PATSIZE 7 // �� ������ ����
#define MY_LEFT 75	 // ���� ����Ű = 75
#define MY_RIGHT 77  // ������ ����Ű = 77
#define MY_UP 72	 // ���� ����Ű = 72
#define MY_DOWN 80	 // �Ʒ��� ����Ű = 80
#define MY_SPACE 32  // �����̽��� = 32

typedef struct
{
	int x;
	int y;
}S_POINT;  // x��ǥ, y��ǥ

typedef struct
{
	char name[12];
	int point;
}SCORE;

int g_data[MY_H][MY_W];//��ü ��
S_POINT g_type[MY_PATSIZE][4][4]={
	{{0,0,1,0,2,0,3,0},{1,0,1,1,1,2,1,3},{0,0,1,0,2,0,3,0},{1,0,1,1,1,2,1,3}}, // �� ��� ���
	{{0,0,1,0,0,1,1,1},{0,0,1,0,0,1,1,1},{0,0,1,0,0,1,1,1},{0,0,1,0,0,1,1,1}}, // �� ��� ���
	{{0,0,1,0,1,1,1,2},{2,0,0,1,1,1,2,1},{0,0,0,1,0,2,1,2},{0,0,1,0,2,0,0,1}}, // L  ��� ���
	{{0,0,1,0,1,1,2,1},{1,0,0,1,1,1,0,2},{0,0,1,0,1,1,2,1},{1,0,0,1,1,1,0,2}}, // 2  ��� ���
	{{0,0,1,0,2,0,2,1},{1,0,1,1,1,2,0,2},{0,0,0,1,1,1,2,1},{0,0,1,0,0,1,0,2}}, // �� ��� ���
	{{1,0,0,1,1,1,2,1},{0,0,0,1,1,1,0,2},{0,0,1,0,2,0,1,1},{0,1,1,0,1,1,1,2}}, // �� ��� ���
	{{0,1,1,1,1,0,2,0},{0,0,0,1,1,1,1,2},{0,1,1,1,1,0,2,0},{0,0,0,1,1,1,1,2}}  // s  ��� ���
};		//�������� ��� ����


int g_dir, g_dirNext1, g_dirNext2;					// ������ ����
int g_patindex, g_patindexNext1, g_patindexNext2;	// ������ ����
S_POINT g_pos={1,1};								// ����� ��ġ
char score[]={48,48,48,48,48,NULL};					// ����
char level[]={48,49,NULL};							// ����
char recordc[]={48,48,48,48,48,NULL};
SCORE record;										// ��� ���� �� ���
SCORE user;
int color, colorNext1, colorNext2;					// ���� ����
double speed;										// ���� �������� �ӵ�
double adminspeed=240;
int combo=0, levelup, gameover=0;					// �޺�, ���� ��, ���ӿ����� �޽�������� ���� �����
int levelint=1;										// ���� (������)
int combocount=0;									// �������� ���� �μ� ��
int musicswitch=1;									// ���� ����� ���� ����ġ


void DataIni();							 	 // ��Ʈ���� ��ü Ʋ (��) �ʱ�ȭ // �Լ� ������ Ÿ��
void DrawData();						 	 // ��Ʈ���� ĭ ǥ��
void DrawBlock();							 // �������� �ִ� ���� ǥ��
void GameMain();
void DrawHelp();

int CheckBlockMove(int dx, int dy, int dir); // ���� ������ �� �ִ� ĭ���� Ȯ��
int MoveBlockDown();						 // ���� �Ʒ��� ������
void CopyBlockToData();						 // �������� ����
void GetNewBlock();							 // ���ο� �� ����
void MoveToEnd();							 // �����̽��� ��������

void DownBlock(int y);						 // ��Ʈ���� ĭ ��ü�� �Ʒ��� �����̵�
void CheckSameBlock();						 // ���� ���ٷ� �ɶ� Ȯ��

void GameStart();							 // ���α׷� ó�� ����� ȭ��
void DrawBlockNext1();						 // �ٷ� �ڿ� ���� ���� ������
void DrawBlockNext2();						 // �ι� �ڿ� ���� ���� ������
void GetRandomBlock();						 // ���� �������� ����
void ScoreLevel();							 // ������ ���� �ø��� �Լ�
void Init();								 // ������� ����� ���� �ʱ�ȭ �Լ�
void Release();								 // ������� �� ����

void PrintScoreLevel();						 // ������ ���� ���
void GetRecord();							 // �ְ��� �ҷ���
void WriteRecord();							 // �ְ��� ����
void DrawTetris();
void Admin();
void DrawAdminTetris();

void GetUserName(void) // �÷��̾� �̸� �Է¹ޱ�
{
	FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[0],0,&g_Channel[0]); // Ÿ��Ʋ ȭ�� BGM ���

	DrawTetris();
	_DrawTextColorOrg(21,19,"��������������������������������������",0xfc);
	_DrawTextColorOrg(21,15,"��������������������������������������",0xfc);
	_DrawTextColorOrg(21,16,"��                                  ��",0xfc);
	_DrawTextColorOrg(21,18,"��                                  ��",0xfc);
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n  \t\t\t\t\t   ");
	_DrawTextColorOrg(21,17,"�� Input Your Name :                ��",0xfc);

	gets(user.name);
	if(!strcmp(user.name,"Admin#"))		
		Admin();	
	else if(!strcmp(user.name,"exit#"))
		exit(0);
	else if(!strcmp(user.name,""))  // �̸��� �Է����� �ʾ��� �� ����� �Է��ϰ� �����
	{
		while(1)
		{
			system("cls");
			_DrawTextColorOrg(25,21,"Input Your Name Correctly!!",0xfc);
			DrawTetris();
			_DrawTextColorOrg(21,19,"��������������������������������������",0xfc);
			_DrawTextColorOrg(21,15,"��������������������������������������",0xfc);
			_DrawTextColorOrg(21,16,"��                                  ��",0xfc);
			_DrawTextColorOrg(21,18,"��                                  ��",0xfc);
			printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n  \t\t\t\t\t   ");
			_DrawTextColorOrg(21,17,"�� Input Your Name :                ��",0xfc);
			gets(user.name);
			fflush(stdin);
			if(strcmp(user.name,""))
				break;
		}
	}
}

void Admin(void)  // ������ ���
{
	char command[10]="0";
	int switch1=0, switch2=0;;

	system("cls");
	FMOD_Channel_Stop(g_Channel[0]);	

	while(1)
	{
		DrawAdminTetris();		

		if(!strcmp(command,"reset#"))			
			_DrawTextColorOrg(25,21,"Reset Record Data!!",0xfc);
		else if(switch1)
			_DrawTextColorOrg(25,21,"Set Record Data!!",0xfc);
		else if(switch2)
			_DrawTextColorOrg(25,21,"Set Speed Data!!",0xfc);

		gets(command);//////////////////////////////////////////////////////////////

		if(!strcmp(command,"reset#"))
		{
			FILE* fp=fopen("data\\record.sav","wb");
			record.point=0;

			fprintf(fp,"%d %s",record.point,record.name);
			fclose(fp);			
		}
		else if(!strcmp(command,"exit#"))
		{
			system("cls");
			GetUserName();
			break;
		}
		else if(!strcmp(command,"speed#"))
		{
			system("cls");
			DrawAdminTetris();
			_DrawTextColorOrg(25,21,"Input Speed Data..",0xfc);
			scanf("%lf",&adminspeed);

			switch2=1;
		}
		else if(!strcmp(command,"set#"))
		{
			FILE* fp=fopen("data\\record.sav","wb");

			system("cls");
			DrawAdminTetris();

			_DrawTextColorOrg(25,21,"Input Record Data..",0xfc);
			gets(command);

			fprintf(fp,command);
			fclose(fp);

			switch1=1;
		}		

		system("cls");
	}	
}

void DrawTetris(void)   // Ÿ��Ʋ ȭ�� ���
{	
	_DrawTextColorOrg(5,3,"����������������������������������������������������������������������",0xf1);
	_DrawTextColorOrg(10,5,"         ",0x99);
	_DrawTextColorOrg(14,6," " ,0x99);
	_DrawTextColorOrg(14,7," ",0x99);
	_DrawTextColorOrg(14,8," ",0x99);
	_DrawTextColorOrg(14,9," ",0x99);               // T
	_DrawTextColorOrg(21,5,"       ",0xdd);
	_DrawTextColorOrg(21,6," " ,0xdd);
	_DrawTextColorOrg(21,7,"       ",0xdd);
	_DrawTextColorOrg(21,8," ",0xdd);
	_DrawTextColorOrg(21,9,"       ",0xdd);         // E
	_DrawTextColorOrg(30,5,"         ",0x22);
	_DrawTextColorOrg(34,6," " ,0x22);
	_DrawTextColorOrg(34,7," ",0x22);
	_DrawTextColorOrg(34,8," ",0x22);
	_DrawTextColorOrg(34,9," ",0x22);				// T
	_DrawTextColorOrg(41,5,"       ",0x33);
	_DrawTextColorOrg(41,6," ",0x33);
	_DrawTextColorOrg(47,6,"  ",0x33);
	_DrawTextColorOrg(41,7,"       ",0x33);
	_DrawTextColorOrg(41,8," ",0x33);
	_DrawTextColorOrg(46,8," ",0x33);
	_DrawTextColorOrg(41,9," ",0x33);
	_DrawTextColorOrg(47,9,"  ",0x33);				// R
	_DrawTextColorOrg(51,5,"     ",0xcc);
	_DrawTextColorOrg(53,6," ",0xcc);
	_DrawTextColorOrg(53,7," ",0xcc);
	_DrawTextColorOrg(53,8," ",0xcc);
	_DrawTextColorOrg(51,9,"     ",0xcc);			// I
	_DrawTextColorOrg(58,5,"            ",0x00);
	_DrawTextColorOrg(66,6,"   ",0x00);
	_DrawTextColorOrg(64,7,"   ",0x00);
	_DrawTextColorOrg(62,8,"   ",0x00);
	_DrawTextColorOrg(60,9,"   ",0x00);
	_DrawTextColorOrg(58,10,"                ",0x00); // Z
	_DrawTextColorOrg(48,11,Ver,0xf0);
	_DrawTextColorOrg(20,11,"2 n d  E D I T I O N  ver",0xf1);
	_DrawTextColorOrg(3,12,"����������������������������������������������������������������������������",0xf1);
	_DrawTextColorOrg(62,13,"CODING By. Z",0xf0);
}

void DrawAdminTetris(void)   // �����ڸ�� Ÿ��Ʋ ȭ�� ���
{	
	_DrawTextColorOrg(5,3,"����������������������������������������������������������������������",0xf1);
	_DrawTextColorOrg(10,5,"         ",0x88);
	_DrawTextColorOrg(14,6," " ,0x88);
	_DrawTextColorOrg(14,7," ",0x88);
	_DrawTextColorOrg(14,8," ",0x88);
	_DrawTextColorOrg(14,9," ",0x88);               // T
	_DrawTextColorOrg(21,5,"       ",0x88);
	_DrawTextColorOrg(21,6," " ,0x88);
	_DrawTextColorOrg(21,7,"       ",0x88);
	_DrawTextColorOrg(21,8," ",0x88);
	_DrawTextColorOrg(21,9,"       ",0x88);         // E
	_DrawTextColorOrg(30,5,"         ",0x88);
	_DrawTextColorOrg(34,6," " ,0x88);
	_DrawTextColorOrg(34,7," ",0x88);
	_DrawTextColorOrg(34,8," ",0x88);
	_DrawTextColorOrg(34,9," ",0x88);				// T
	_DrawTextColorOrg(41,5,"       ",0x88);
	_DrawTextColorOrg(41,6," ",0x88);
	_DrawTextColorOrg(47,6,"  ",0x88);
	_DrawTextColorOrg(41,7,"       ",0x88);
	_DrawTextColorOrg(41,8," ",0x88);
	_DrawTextColorOrg(46,8," ",0x88);
	_DrawTextColorOrg(41,9," ",0x88);
	_DrawTextColorOrg(47,9,"  ",0x88);				// R
	_DrawTextColorOrg(51,5,"     ",0x88);
	_DrawTextColorOrg(53,6," ",0x88);
	_DrawTextColorOrg(53,7," ",0x88);
	_DrawTextColorOrg(53,8," ",0x88);
	_DrawTextColorOrg(51,9,"     ",0x88);			// I
	_DrawTextColorOrg(58,5,"            ",0x00);
	_DrawTextColorOrg(66,6,"   ",0x00);
	_DrawTextColorOrg(64,7,"   ",0x00);
	_DrawTextColorOrg(62,8,"   ",0x00);
	_DrawTextColorOrg(60,9,"   ",0x00);
	_DrawTextColorOrg(58,10,"                ",0x00); // Z
	_DrawTextColorOrg(48,11,Ver,0xf0);
	_DrawTextColorOrg(20,11,"2 n d  E D I T I O N  ver",0xf1);
	_DrawTextColorOrg(3,12,"����������������������������������������������������������������������������",0xf1);
	_DrawTextColorOrg(56,13,"Admin Mode!!",0xfc);

	_DrawTextColorOrg(21,19,"��������������������������������������",0xfc);
	_DrawTextColorOrg(21,15,"��������������������������������������",0xfc);
	_DrawTextColorOrg(21,16,"��                                  ��",0xfc);
	_DrawTextColorOrg(21,18,"��                                  ��",0xfc);
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n  \t\t\t\t\t   ");
	_DrawTextColorOrg(21,17,"�� Input Command :                  ��",0xfc);
}

void GameStart() // ���α׷� ó�� ����� ȭ��
{	
	_InvalidateColor(); // �ܼ�â�� ���� ��� (�������)
	GetRandomBlock();   // ���� �������� ����	
	DrawTetris();    // Ÿ��Ʋ ȭ�� ���

	_DrawTextColorOrg(20,19,"������������������������������������������",0xfc);
	_DrawTextColorOrg(20,15,"������������������������������������������",0xfc);
	_DrawTextColorOrg(25,21,"Welcome!",0xfc);
	_DrawTextColorOrg(20,16,"��                                      ��",0xfc);
	_DrawTextColorOrg(34,21,user.name,0xf0);
	_DrawTextColorOrg(20,17,"�� Press any key to start TETRIS game!  ��",0xfc);
	_DrawTextColorOrg(20,18,"��                                      ��",0xfc);

	GetRecord(); // �ְ����� �ҷ���
	
	while (_GetKey()==-1)
	{		
		
	}
	FMOD_Channel_Stop(g_Channel[0]); // Ÿ��Ʋ ȭ�� BGM ����

	FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[1],0,&g_Channel[0]); // ���� ȭ�� BGM ���
}

void Init() // ������� ����� ���� �ʱ�ȭ �Լ�
{
	FMOD_System_Create(&g_System);
	FMOD_System_Init(g_System,10,FMOD_INIT_NORMAL,NULL);

	FMOD_System_CreateSound(g_System,"data\\bgm_lobby.mp3",FMOD_LOOP_NORMAL,0,&g_Sound[0]); // Ÿ��Ʋ ȭ�� BGM
	FMOD_System_CreateSound(g_System,"data\\bgm_main.mp3",FMOD_LOOP_NORMAL,0,&g_Sound[1]);  // ���� ȭ�� BGM
	FMOD_System_CreateSound(g_System,"data\\g_end.mp3",FMOD_DEFAULT,0,&g_Sound[2]);         // ���ӿ��� ȿ����
	FMOD_System_CreateSound(g_System,"data\\g_jelly.mp3",FMOD_DEFAULT,0,&g_Sound[3]);       // ���� �ٴڿ� ������ �� ȿ����
	FMOD_System_CreateSound(g_System,"data\\i_large_energy.mp3",FMOD_DEFAULT,0,&g_Sound[4]);// ���� �ϼ����� �� ȿ����
	FMOD_System_CreateSound(g_System,"data\\r_levelup.mp3",FMOD_DEFAULT,0,&g_Sound[5]);	  // ������ ���� �� ȿ����
}

void Release() // ������� �� ����
{
	FMOD_Sound_Release(g_Sound[0]);
	FMOD_Sound_Release(g_Sound[1]);
	FMOD_Sound_Release(g_Sound[2]);
	FMOD_Sound_Release(g_Sound[3]);
	FMOD_Sound_Release(g_Sound[4]);
	FMOD_Sound_Release(g_Sound[5]);
	FMOD_System_Close(g_System);
	FMOD_System_Release(g_System);
}

void DrawBlockNext1() // �ٷ� �ڿ� ���� ���� ������
{
	int x,y,i;	

	for(i=0; i<4; i++)
	{

		x=g_type[g_patindexNext1][g_dirNext1][i].x + 22; // ���� ǥ���� x��ǥ
		y=g_type[g_patindexNext1][g_dirNext1][i].y + 8;  // ���� ǥ���� y��ǥ
		
		_DrawTextColor(x*2,y,"��",colorNext1);		// ���� ��� �� ����
	}	
}

void DrawBlockNext2() // �ι� �ڿ� ���� ���� ������
{
	int x,y,i;	

	for(i=0; i<4; i++)
	{
		x=g_type[g_patindexNext2][g_dirNext2][i].x + 32; // ���� ǥ���� x��ǥ
		y=g_type[g_patindexNext2][g_dirNext2][i].y + 8;  // ���� ǥ���� y��ǥ
		
		_DrawTextColor(x*2,y,"��",colorNext2);		// ���� ��� �� ����
	}	
}

void GetRandomBlock() // ���� �������� ����
{
	g_pos.x=5;								 // �������� �����ϴ� x��ǥ  // 1+seed%rand()%(MY_W-5)
	g_dirNext2=rand()%4;					 // ������ ���� ������ ���� ���� ����
	g_patindexNext2=rand()%46%MY_PATSIZE;	 // ������ ���� ���� ���� ���� ���� 	
	colorNext2=g_patindexNext2+241;			 // ������ ���� �� ����
	if(colorNext2==241+6)
		colorNext2+=6;
	
	g_dirNext1=rand()%4; 
	g_patindexNext1=rand()%MY_PATSIZE;
	colorNext1=g_patindexNext1+241;
	if(colorNext1==241+6)
		colorNext1+=6;

	g_dir=rand()%4; 
	g_patindex=rand()%MY_PATSIZE;
	color=g_patindex+241;
	if(color==241+6)
		color+=6;
}

void DrawHelp() // �������� ���� ǥ��
{
	_DrawTextColor(32,1,"TETRIZ 2nd EDITION ver",0xf2);				// ���� ���۹��� ���� �޽��� ���
	_DrawTextColor(55,1,Ver,0xf0);
	_DrawTextColor(65,1,"CODING By. Z",0xf0);
	_DrawTextColor(29,2,"��������������������������������������������������",0xf3);
	_DrawTextColor(32,3,"Q : exit, LEFT,RIGHT : move, UP,DOWN : rotation",0xf3);
	_DrawTextColor(32,5,"P : pause, SPACE : down, R : restart, T : title",0xf3);
	_DrawTextColor(29,6,"��������������������������������������������������",0xf3);
	_DrawTextColor(32,8,"NEXT 1 :              NEXT 2 : ",0xf3);
	DrawBlockNext1();	
	DrawBlockNext2();
	_DrawTextColor(29,13,"��������������������������������������������������",0xf3);
	_DrawTextColor(32,15,"SCORE : ",0xf3);	
	_DrawTextColor(41,15,score,0xf3); // ���� ���
	_DrawTextColor(32,17,"LEVEL : ",0xf3);
	_DrawTextColor(41,17,level,0xf3); // ���� ���
	_DrawTextColor(50,15,"RECORD : ",0xf3);
	_DrawTextColor(60,15,recordc,0xf0); // �ְ��� ���
	if(record.point)
		_DrawTextColor(67,15,record.name,0xf0); // �ְ��� ���

	switch (combo)
	{
	case 1:
		_DrawTextColor(32,19,"NICE!",0xfd);		// �� 1�� �ϼ����� �� ���
		break;
	case 2:
		_DrawTextColor(32,19,"GREAT! ",0xfd);   // �� 2�� �ϼ����� �� ���
		break;
	case 3:
		_DrawTextColor(32,19,"COOL!! ",0xf9);	// �� 3�� �ϼ����� �� ���
		break;
	case 4:
		_DrawTextColor(32,19,"EXCELLENT!! ",0xfc);// �� 4�� �ϼ����� �� ���
		break;
	}

	if (levelup==1)	
		_DrawTextColor(59,19,"Level Up!! ",0xfc); // �������� �� ���
		
	switch (combocount)
	{
	case 1:		
		break;
	case 2:
		_DrawTextColor(46,19,"2 combo!",0xfc); // 2�� �������� �ϼ����� �� ���
		break;
	case 3:
		_DrawTextColor(46,19,"3 combo!",0xfc); // 3�� �������� �ϼ����� �� ���
		break;
	case 4:
		_DrawTextColor(46,19,"4 combo!!",0xfc); // 4�� �������� �ϼ����� �� ���
		break;
	case 5:
		_DrawTextColor(46,19,"5 combo!!",0xfc); // 5�� �������� �ϼ����� �� ���
		break;
	case 6:
		_DrawTextColor(46,19,"6 combo!!!",0xfc); // 6�� �������� �ϼ����� �� ���
		break;
	case 7:
		_DrawTextColor(46,19,"7 combo!!!",0xfc); // 7�� �������� �ϼ����� �� ���
		break;
	case 8:
		_DrawTextColor(46,19,"8 combo!!!!",0xfc); // 8�� �������� �ϼ����� �� ���
		break;
	case 9:
		_DrawTextColor(46,19,"9 combo!!!!",0xfc); // 9�� �������� �ϼ����� �� ���
		break;
	case 10:
		_DrawTextColor(46,19,"10 combo!!!!!",0xfc); // 10�� �������� �ϼ����� �� ���
		break;
	case 11:
		_DrawTextColor(46,19,"11 combo!!!!!",0xfc); // 11�� �������� �ϼ����� �� ���
		break;
	}

	if (gameover==1)
	{
		if (user.point>=record.point&&user.point>0)
			_DrawTextColor(32,15,"   NEW RECORD!!! ",0xfc); // �ְ��� ����

		_DrawTextColor(32,19,"������������������������������������",0xfc);
		_DrawTextColor(33,20,"Game Over! press 'r' key to restart",0xec);	  // ���� ���� ���
		_DrawTextColor(32,21,"������������������������������������",0xfc);
	}

}

void ScoreLevel() // ������ ���� �ø��� �Լ�
{
	if(user.point%1000+100*(combo+1)+100*combocount>=1000) // ���� ��
	{
		user.point+=((combo+1)*100+100*combocount);    // ���� ���� (1��: 100, 2��: 300, 3��: 600, 4��: 1000)
		if(speed>70)								 // 2 �޺�    (1��: 200, 2��: 500, 3��: 900, 4��: 1400)
			speed=7*speed/8;					     // 3 �޺�    (1��: 300, 2��: 700, 3��: 1200, 4��: 1800)  .....
		levelup=1;									 
		levelint++;
	}
	else
		user.point+=((combo+1)*100+100*combocount);  

	combo++; // �޺� ��
}

void PrintScoreLevel()  // int���� char������ ��ȯ������
{
	score[0]=user.point/10000+48;
	score[1]=(user.point%10000)/1000+48;
	score[2]=(user.point%1000)/100+48;

	level[0]=levelint/10+48;
	level[1]=levelint%10+48;
}

void GetRecord()	 // �ְ��� �ҷ���
{
	FILE* fp=fopen("data\\record.sav","rb");
	if(fp==NULL)
	{
		return;
	}
	fscanf(fp,"%d %s",&record.point,record.name);

	recordc[0]=record.point/10000+48;
	recordc[1]=(record.point%10000)/1000+48;  // char������ ��ȯ��Ŵ
	recordc[2]=(record.point%1000)/100+48;
	fclose(fp);
}

void WriteRecord()		// �ְ��� ����
{
	if (user.point>record.point) // ���� ������ ��Ϻ��� ���� ��!! �ְ��� �޼�~!! ����
	{
		FILE* fp=fopen("data\\record.sav","wb");
		record.point=user.point;
		recordc[0]=record.point/10000+48;
		recordc[1]=(record.point%10000)/1000+48;
		recordc[2]=(record.point%1000)/100+48;
		strcpy(record.name,user.name);  // �ְ��Ͽ� ���� �̸��� ����

		fprintf(fp,"%d %s",record.point,record.name);
		fclose(fp);
	}
}


void DrawData() // ��Ʈ���� ĭ ǥ��
{
	int x,y;

	for(y=0; y<MY_H; y++)
	{
		for(x=0; x<MY_W; x++)
		{
			if(MY_WALL==g_data[y][x])
			{
				_DrawTextColor(x*2,y,"��",0x78); // ĭ �ܺ� �� ǥ��
			}

			else if(MY_BLANK == g_data[y][x])
			{
				_DrawTextColor(x*2,y," ",0xf7); // ĭ ���� ���� ǥ��
			}

			else
			{
				_DrawTextColor(x*2,y,"��",0xf8); // ������ �� ǥ��
			}
		}
	}
}


void DrawBlock() // �������� �ִ� ���� ǥ��
{
	int x,y,i;	

	for(i=0; i<4; i++)
	{
		x=g_pos.x +g_type[g_patindex][g_dir][i].x;
		y=g_pos.y +g_type[g_patindex][g_dir][i].y;
		_DrawTextColor(x*2,y,"��",color);
	}
}


void DataIni() // ��Ʈ���� ��ü Ʋ (��) �ʱ�ȭ
{
	int x,y;

	for(y=0; y<MY_H; y++)
	{
		for(x=0;x<MY_W; x++)
		{
			if(0==x || y==0 || MY_W ==(x+1) || MY_H ==(y+1))
			{
				g_data[y][x]=MY_WALL; // x,y�� ��
			}

			else
			{
				g_data[y][x]=MY_BLANK; // x,y�� �����
			}

		}
	}
}

void CheckSameBlock() // ���� ���ٷ� �ɶ� Ȯ��
{
	int x,y,count=0;
	combo=0;
	levelup=0;

	for(y=MY_H-2; 0<y ; y--)
	{
		int same=0;
		for(x=1; x<MY_W-1; x++)
		{
			if(MY_BLOCK != g_data[y][x]) // x �� ���ٿ� ������� ������
			{
				same=-1;				
				break;
			}
		}

		if(0==same) // x �� ������ ���϶�
		{

			DownBlock(y); // ������ �Ʒ��� ����߸���
			y++;			
			ScoreLevel();
			PrintScoreLevel();
		}
	}
}

int CheckBlockMove(int dx, int dy, int dir) // ���� ������ �� �ִ� ĭ���� Ȯ��
{
	int x,y;
	int i;
	
	for(i=0; i<4; i++)
	{
		x=dx+g_type[g_patindex][dir][i].x;
		y=dy+g_type[g_patindex][dir][i].y;
		if(MY_BLANK != g_data[y][x])
		{

			return -1; // ������ �� ����
		}
	}

	return 0;
}


int MoveBlockDown() // ���� �Ʒ��� ������
{

	if(0==CheckBlockMove(g_pos.x, g_pos.y+1, g_dir)) // ���� ������ �� �ִ� ĭ�϶�
	{
		g_pos.y++; // y��ǥ 1�� ���� (�Ʒ��� ������)
		return 0;
	}

	return -1;
}

void CopyBlockToData() // �������� ����
{

	int x,y;
	int i;

	for(i=0; i<4; i++)
	{

		x=g_pos.x + g_type[g_patindex][g_dir][i].x;
		y=g_pos.y + g_type[g_patindex][g_dir][i].y;

		g_data[y][x]=MY_BLOCK; // ��Ʈ���� ĭ �ȿ� �������� ����

	}
}

void GetNewBlock() // ���ο� �� ����
{	
	g_pos.x=5; // �������� �����ϴ� ��ġ (x��ǥ)
	g_pos.y=1; // �������� �����ϴ� ��ġ (y��ǥ)

	g_dir=g_dirNext1; // ���� ����1�� ������ ���Ͽ� ����
	g_patindex=g_patindexNext1;
	color=colorNext1;

	g_dirNext1=g_dirNext2; // ���� ����2�� ���� ����1�� ����
	g_patindexNext1=g_patindexNext2;
	colorNext1=colorNext2;
	
	g_dirNext2=rand()%4; // ������ ���� ������ ���� ���� ����
	g_patindexNext2=rand()%MY_PATSIZE; // ������ ���� ���� ���� ���� ���� 	
	colorNext2=g_patindexNext2+241; // ������ ���� �� ���� ���� ����
	if(colorNext2==241+6)
		colorNext2+=6;

	if (combo>=1) // ���� �ϼ����� ��
	{
		FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[4],0,&g_Channel[1]); // ���� �ϼ����� �� �Ҹ� ���
		combocount++;
	}
	else
	{
		FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[3],0,&g_Channel[1]); // ���� �������� �� �Ҹ� ���
		combocount=0;
	}

	if (levelup)
		FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[5],0,&g_Channel[1]); // ������ ���� �� �Ҹ� ���
}



void MoveToEnd() // �����̽��� ��������
{
	while(1)
	{
		if(-1==MoveBlockDown())
		{
			CopyBlockToData();
			CheckSameBlock();
			GetNewBlock();			
			break;
		}
	}
}

void DownBlock(int y) // ��Ʈ���� ĭ ��ü�� �Ʒ��� �����̵�
{

	int x;

	for(; 1<=y; y--) // y ��ǥ�� 1���� Ŭ �� ������
	{
		for(x=1; x<MY_W -1; x++)
		{
			if(1==y)
			{
				g_data[y][x]=MY_BLANK; // y��ǥ 1�϶� �����
			}

			else
			{

				g_data[y][x]=g_data[y-1][x]; // ��Ʈ���� ĭ�� ������				
			}
		}
	}
}

//////////////////////////


void Draw() // ȭ�鿡 ǥ��
{
	DrawData(); // Ʋ�� �����.
	DrawBlock(); // ����� �����
	DrawHelp(); // ���� ���� ǥ��
}

void RunIni() // ���� �ʱ�ȭ
{
	DataIni(); // ������ �ʱ�ȭ
	_Invalidate(); // ������ + ��Ʈ���� â ���
}

int RunKey() // Ű �Է¹޴� �Լ�
{

	char k;

	k=_GetKey();
	if(-1==k) return 0;

	if('q'==k) // q �Է¹����� -1 (����) ��ȯ
	{
		return -1;
	}

	else if ('p'==k)  // ���� �Ͻ�����
	{
		_MessageBoxColor(32,21,38,2,"Paused. press any key to resume",0xf4); 
	}

	else if ('r'==k)  // rŰ ������ ���� �ʱ�ȭ (����)
	{
		GetRandomBlock();
		RunIni();
		user.point=0;
		levelint=1;
		speed=adminspeed;
		g_pos.y=1;
		combo=0;
		levelup=0;
		PrintScoreLevel();
	}

	else if ('m'==k) // mŰ ������ BGM ��� or ����
	{
		if (musicswitch==1)
		{
			FMOD_Channel_Stop(g_Channel[0]); // Ÿ��Ʋ ȭ�� BGM ����
			musicswitch=0;
		}

		else
		{
			FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[1],0,&g_Channel[0]); // BGM ���
			musicswitch=1;
		}
	}
	else if('t'==k)  // tŰ ������ Ÿ��Ʋ ȭ������
	{
		system("cls");

		FMOD_Channel_Stop(g_Channel[0]); // Ÿ��Ʋ ȭ�� BGM ����		
		GetRandomBlock();			
		RunIni();
		speed=adminspeed; // ���ǵ� �ʱ�ȭ
		user.point=0;
		levelint=1; // ���� ���� �ʱ�ȭ (0����)
		combo=0;
		levelup=0;
		g_pos.x=1;
		g_pos.y=1;

		system("cls");

		GetUserName();
		GameStart();
		GameMain();
	}

	if(MY_LEFT == k) // ����Ű ���� �Է¹����� �������� �̵�
	{
		if(0==CheckBlockMove(g_pos.x-1, g_pos.y, g_dir)) // x��ǥ -1 �� ������ �� �ִ� ĭ�̸� 
		{
			g_pos.x--;
		}
	}

	else if(MY_RIGHT==k) // ����Ű ������ �Է¹����� ���������� �̵�
	{
		if(0==CheckBlockMove(g_pos.x+1, g_pos.y, g_dir))
		{
			g_pos.x++;
		}

	}

	else if(MY_SPACE ==k) // �����̽��� �Է¹����� �� �Ʒ��� �̵�
	{
		MoveToEnd();
	}

	else if(MY_UP==k) // ����Ű ���� �Է¹����� �ð�������� ȸ��
	{
		int dir=g_dir;
		dir++;
		if(3<dir)dir=0;
		if(0==CheckBlockMove(g_pos.x, g_pos.y, dir))
		{
			g_dir=dir;
			_Invalidate();
		}
	}

	else if(MY_DOWN ==k) // ����Ű �Ʒ��� �Է¹����� �ð�������� ȸ��
	{
		int dir=g_dir;

		dir--;
		if(dir<0) dir=3;
		if(0==CheckBlockMove(g_pos.x, g_pos.y, dir))
		{
			g_dir=dir;
			_Invalidate();
		}
	}
	_Invalidate();

	return 0;

}


void RunTimer() // Ÿ�̸�
{
	static long oldT=0;
	long newT;
	static int killTimer=0;

	if(-1==killTimer)
		return;

	newT=_GetTickCount();
	if(abs(newT-oldT)<speed) // �������� �ӵ�
	{
		return;
	}
	else
	{
		oldT=newT;
	}
	
	if(-1==MoveBlockDown()) // �ٴڿ� �������� �� 
	{		
		CopyBlockToData();
		CheckSameBlock();
		GetNewBlock();
				
		if(-1==MoveBlockDown())
		{
			
			gameover=1;
			WriteRecord();
			_Invalidate();
			killTimer=-1;
			FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[2],0,&g_Channel[1]); // ���� ���� ���� ���

			while (1)
			{	
				if (_GetKey()=='r') // rŰ ������ ���� �����
					break;
			}

			gameover=0;
			GetRandomBlock();			
			RunIni();
			killTimer=0;
			speed=adminspeed; // ���ǵ� �ʱ�ȭ
			user.point=0;
			levelint=1; // ���� ���� �ʱ�ȭ (0����)
			combo=0;
			levelup=0;
			PrintScoreLevel();
		}		
	}
	_Invalidate();
}

void GameMain() // ���� ���� �Լ�
{
	RunIni();	// ���� �ʱ�ȭ
	while(1)
	{
		RunTimer();
		if(-1==RunKey()) // ��ȯ���� -1 �̸� ���� ����
		{
			break;
		}
	}
}

int main()
{	
	system("color f0");
	puts("\n\t * Loading....Please Wait....");
	Init(); // ������� ����� ���� �ʱ�ȭ �Լ�
	FMOD_System_Update(g_System);
	srand(time(NULL));
	_BeginWindow();
	GetUserName();
	GameStart();	 // ���α׷� ó�� ����� ȭ��
	speed=adminspeed;
	GameMain();
	_EndWindow();
	Release();
	return 0;
}