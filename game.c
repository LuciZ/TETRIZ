#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#include "mtg.h"
#include "inc\fmod.h"

#define Ver "1.7.1"

FMOD_SYSTEM *g_System;		// 배경음악 재생을 위한 함수
FMOD_SOUND *g_Sound[6];		// 배경음악(효과음)의 개수
FMOD_CHANNEL *g_Channel[2]; // 동시재생될 개수

#define MY_W 12+2  // 테트리스의 너비
#define MY_H 23+2  // 테트리스의 높이
#define MY_WALL 0  // 벽 = 0
#define MY_BLANK 1 // 빈 공간 = 1
#define MY_BLOCK 2 // 블럭 = 2

#define MY_PATSIZE 7 // 블럭 패턴의 개수
#define MY_LEFT 75	 // 왼쪽 방향키 = 75
#define MY_RIGHT 77  // 오른쪽 방향키 = 77
#define MY_UP 72	 // 위쪽 방향키 = 72
#define MY_DOWN 80	 // 아래쪽 방향키 = 80
#define MY_SPACE 32  // 스페이스바 = 32

typedef struct
{
	int x;
	int y;
}S_POINT;  // x좌표, y좌표

typedef struct
{
	char name[12];
	int point;
}SCORE;

int g_data[MY_H][MY_W];//전체 맵
S_POINT g_type[MY_PATSIZE][4][4]={
	{{0,0,1,0,2,0,3,0},{1,0,1,1,1,2,1,3},{0,0,1,0,2,0,3,0},{1,0,1,1,1,2,1,3}}, // ㅣ 모양 블록
	{{0,0,1,0,0,1,1,1},{0,0,1,0,0,1,1,1},{0,0,1,0,0,1,1,1},{0,0,1,0,0,1,1,1}}, // ㅁ 모양 블록
	{{0,0,1,0,1,1,1,2},{2,0,0,1,1,1,2,1},{0,0,0,1,0,2,1,2},{0,0,1,0,2,0,0,1}}, // L  모양 블록
	{{0,0,1,0,1,1,2,1},{1,0,0,1,1,1,0,2},{0,0,1,0,1,1,2,1},{1,0,0,1,1,1,0,2}}, // 2  모양 블록
	{{0,0,1,0,2,0,2,1},{1,0,1,1,1,2,0,2},{0,0,0,1,1,1,2,1},{0,0,1,0,0,1,0,2}}, // ㄴ 모양 블록
	{{1,0,0,1,1,1,2,1},{0,0,0,1,1,1,0,2},{0,0,1,0,2,0,1,1},{0,1,1,0,1,1,1,2}}, // ㅗ 모양 블록
	{{0,1,1,1,1,0,2,0},{0,0,0,1,1,1,1,2},{0,1,1,1,1,0,2,0},{0,0,0,1,1,1,1,2}}  // s  모양 블록
};		//떨어지는 블록 패턴


int g_dir, g_dirNext1, g_dirNext2;					// 패턴의 방향
int g_patindex, g_patindexNext1, g_patindexNext2;	// 패턴의 종류
S_POINT g_pos={1,1};								// 블록의 위치
char score[]={48,48,48,48,48,NULL};					// 점수
char level[]={48,49,NULL};							// 레벨
char recordc[]={48,48,48,48,48,NULL};
SCORE record;										// 기록 저장 및 출력
SCORE user;
int color, colorNext1, colorNext2;					// 블럭의 색상
double speed;										// 블럭이 떨어지는 속도
double adminspeed=240;
int combo=0, levelup, gameover=0;					// 콤보, 레벨 업, 게임오버의 메시지출력을 위한 상수값
int levelint=1;										// 레벨 (정수형)
int combocount=0;									// 연속으로 블럭을 부술 때
int musicswitch=1;									// 음악 재생을 위한 스위치


void DataIni();							 	 // 테트리스 전체 틀 (벽) 초기화 // 함수 프로토 타입
void DrawData();						 	 // 테트리스 칸 표시
void DrawBlock();							 // 떨어지고 있는 블럭을 표시
void GameMain();
void DrawHelp();

int CheckBlockMove(int dx, int dy, int dir); // 블럭이 움직일 수 있는 칸인지 확인
int MoveBlockDown();						 // 블럭이 아래로 떨어짐
void CopyBlockToData();						 // 블럭데이터 복사
void GetNewBlock();							 // 새로운 블럭 생성
void MoveToEnd();							 // 스페이스바 눌렀을때

void DownBlock(int y);						 // 테트리스 칸 전체를 아래로 평행이동
void CheckSameBlock();						 // 블럭이 한줄로 될때 확인

void GameStart();							 // 프로그램 처음 실행시 화면
void DrawBlockNext1();						 // 바로 뒤에 나올 블럭을 보여줌
void DrawBlockNext2();						 // 두번 뒤에 나올 블럭을 보여줌
void GetRandomBlock();						 // 블럭을 랜덤으로 생성
void ScoreLevel();							 // 점수와 레벨 올리는 함수
void Init();								 // 배경음악 재생을 위한 초기화 함수
void Release();								 // 음악재생 후 종료

void PrintScoreLevel();						 // 점수와 레벨 출력
void GetRecord();							 // 최고기록 불러옴
void WriteRecord();							 // 최고기록 저장
void DrawTetris();
void Admin();
void DrawAdminTetris();

void GetUserName(void) // 플레이어 이름 입력받기
{
	FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[0],0,&g_Channel[0]); // 타이틀 화면 BGM 재생

	DrawTetris();
	_DrawTextColorOrg(21,19,"└─────────────────┘",0xfc);
	_DrawTextColorOrg(21,15,"┌─────────────────┐",0xfc);
	_DrawTextColorOrg(21,16,"│                                  │",0xfc);
	_DrawTextColorOrg(21,18,"│                                  │",0xfc);
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n  \t\t\t\t\t   ");
	_DrawTextColorOrg(21,17,"│ Input Your Name :                │",0xfc);

	gets(user.name);
	if(!strcmp(user.name,"Admin#"))		
		Admin();	
	else if(!strcmp(user.name,"exit#"))
		exit(0);
	else if(!strcmp(user.name,""))  // 이름을 입력하지 않았을 때 제대로 입력하게 만들기
	{
		while(1)
		{
			system("cls");
			_DrawTextColorOrg(25,21,"Input Your Name Correctly!!",0xfc);
			DrawTetris();
			_DrawTextColorOrg(21,19,"└─────────────────┘",0xfc);
			_DrawTextColorOrg(21,15,"┌─────────────────┐",0xfc);
			_DrawTextColorOrg(21,16,"│                                  │",0xfc);
			_DrawTextColorOrg(21,18,"│                                  │",0xfc);
			printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n  \t\t\t\t\t   ");
			_DrawTextColorOrg(21,17,"│ Input Your Name :                │",0xfc);
			gets(user.name);
			fflush(stdin);
			if(strcmp(user.name,""))
				break;
		}
	}
}

void Admin(void)  // 관리자 모드
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

void DrawTetris(void)   // 타이틀 화면 출력
{	
	_DrawTextColorOrg(5,3,"───────────────────────────────────",0xf1);
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
	_DrawTextColorOrg(3,12,"──────────────────────────────────────",0xf1);
	_DrawTextColorOrg(62,13,"CODING By. Z",0xf0);
}

void DrawAdminTetris(void)   // 관리자모드 타이틀 화면 출력
{	
	_DrawTextColorOrg(5,3,"───────────────────────────────────",0xf1);
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
	_DrawTextColorOrg(3,12,"──────────────────────────────────────",0xf1);
	_DrawTextColorOrg(56,13,"Admin Mode!!",0xfc);

	_DrawTextColorOrg(21,19,"└─────────────────┘",0xfc);
	_DrawTextColorOrg(21,15,"┌─────────────────┐",0xfc);
	_DrawTextColorOrg(21,16,"│                                  │",0xfc);
	_DrawTextColorOrg(21,18,"│                                  │",0xfc);
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n  \t\t\t\t\t   ");
	_DrawTextColorOrg(21,17,"│ Input Command :                  │",0xfc);
}

void GameStart() // 프로그램 처음 실행시 화면
{	
	_InvalidateColor(); // 콘솔창에 색깔 출력 (흰색바탕)
	GetRandomBlock();   // 블럭을 랜덤으로 생성	
	DrawTetris();    // 타이틀 화면 출력

	_DrawTextColorOrg(20,19,"└───────────────────┘",0xfc);
	_DrawTextColorOrg(20,15,"┌───────────────────┐",0xfc);
	_DrawTextColorOrg(25,21,"Welcome!",0xfc);
	_DrawTextColorOrg(20,16,"│                                      │",0xfc);
	_DrawTextColorOrg(34,21,user.name,0xf0);
	_DrawTextColorOrg(20,17,"│ Press any key to start TETRIS game!  │",0xfc);
	_DrawTextColorOrg(20,18,"│                                      │",0xfc);

	GetRecord(); // 최고기록을 불러옴
	
	while (_GetKey()==-1)
	{		
		
	}
	FMOD_Channel_Stop(g_Channel[0]); // 타이틀 화면 BGM 정지

	FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[1],0,&g_Channel[0]); // 메인 화면 BGM 재생
}

void Init() // 배경음악 재생을 위한 초기화 함수
{
	FMOD_System_Create(&g_System);
	FMOD_System_Init(g_System,10,FMOD_INIT_NORMAL,NULL);

	FMOD_System_CreateSound(g_System,"data\\bgm_lobby.mp3",FMOD_LOOP_NORMAL,0,&g_Sound[0]); // 타이틀 화면 BGM
	FMOD_System_CreateSound(g_System,"data\\bgm_main.mp3",FMOD_LOOP_NORMAL,0,&g_Sound[1]);  // 메인 화면 BGM
	FMOD_System_CreateSound(g_System,"data\\g_end.mp3",FMOD_DEFAULT,0,&g_Sound[2]);         // 게임오버 효과음
	FMOD_System_CreateSound(g_System,"data\\g_jelly.mp3",FMOD_DEFAULT,0,&g_Sound[3]);       // 블럭이 바닥에 떨어질 때 효과음
	FMOD_System_CreateSound(g_System,"data\\i_large_energy.mp3",FMOD_DEFAULT,0,&g_Sound[4]);// 한줄 완성했을 때 효과음
	FMOD_System_CreateSound(g_System,"data\\r_levelup.mp3",FMOD_DEFAULT,0,&g_Sound[5]);	  // 레벨업 했을 때 효과음
}

void Release() // 음악재생 후 종료
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

void DrawBlockNext1() // 바로 뒤에 나올 블럭을 보여줌
{
	int x,y,i;	

	for(i=0; i<4; i++)
	{

		x=g_type[g_patindexNext1][g_dirNext1][i].x + 22; // 블럭을 표시할 x좌표
		y=g_type[g_patindexNext1][g_dirNext1][i].y + 8;  // 블럭을 표시할 y좌표
		
		_DrawTextColor(x*2,y,"■",colorNext1);		// 블럭의 모양 및 패턴
	}	
}

void DrawBlockNext2() // 두번 뒤에 나올 블럭을 보여줌
{
	int x,y,i;	

	for(i=0; i<4; i++)
	{
		x=g_type[g_patindexNext2][g_dirNext2][i].x + 32; // 블럭을 표시할 x좌표
		y=g_type[g_patindexNext2][g_dirNext2][i].y + 8;  // 블럭을 표시할 y좌표
		
		_DrawTextColor(x*2,y,"■",colorNext2);		// 블럭의 모양 및 패턴
	}	
}

void GetRandomBlock() // 블럭을 랜덤으로 생성
{
	g_pos.x=5;								 // 떨어지기 시작하는 x좌표  // 1+seed%rand()%(MY_W-5)
	g_dirNext2=rand()%4;					 // 앞으로 나올 패턴의 방향 랜덤 생성
	g_patindexNext2=rand()%46%MY_PATSIZE;	 // 앞으로 나올 패턴 종류 랜덤 생성 	
	colorNext2=g_patindexNext2+241;			 // 앞으로 나올 블럭 색상
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

void DrawHelp() // 오른쪽의 내용 표시
{
	_DrawTextColor(32,1,"TETRIZ 2nd EDITION ver",0xf2);				// 게임 조작법과 상태 메시지 출력
	_DrawTextColor(55,1,Ver,0xf0);
	_DrawTextColor(65,1,"CODING By. Z",0xf0);
	_DrawTextColor(29,2,"─────────────────────────",0xf3);
	_DrawTextColor(32,3,"Q : exit, LEFT,RIGHT : move, UP,DOWN : rotation",0xf3);
	_DrawTextColor(32,5,"P : pause, SPACE : down, R : restart, T : title",0xf3);
	_DrawTextColor(29,6,"─────────────────────────",0xf3);
	_DrawTextColor(32,8,"NEXT 1 :              NEXT 2 : ",0xf3);
	DrawBlockNext1();	
	DrawBlockNext2();
	_DrawTextColor(29,13,"─────────────────────────",0xf3);
	_DrawTextColor(32,15,"SCORE : ",0xf3);	
	_DrawTextColor(41,15,score,0xf3); // 점수 출력
	_DrawTextColor(32,17,"LEVEL : ",0xf3);
	_DrawTextColor(41,17,level,0xf3); // 레벨 출력
	_DrawTextColor(50,15,"RECORD : ",0xf3);
	_DrawTextColor(60,15,recordc,0xf0); // 최고기록 출력
	if(record.point)
		_DrawTextColor(67,15,record.name,0xf0); // 최고기록 출력

	switch (combo)
	{
	case 1:
		_DrawTextColor(32,19,"NICE!",0xfd);		// 블럭 1줄 완성했을 때 출력
		break;
	case 2:
		_DrawTextColor(32,19,"GREAT! ",0xfd);   // 블럭 2줄 완성했을 때 출력
		break;
	case 3:
		_DrawTextColor(32,19,"COOL!! ",0xf9);	// 블럭 3줄 완성했을 때 출력
		break;
	case 4:
		_DrawTextColor(32,19,"EXCELLENT!! ",0xfc);// 블럭 4줄 완성했을 때 출력
		break;
	}

	if (levelup==1)	
		_DrawTextColor(59,19,"Level Up!! ",0xfc); // 레벨업할 때 출력
		
	switch (combocount)
	{
	case 1:		
		break;
	case 2:
		_DrawTextColor(46,19,"2 combo!",0xfc); // 2번 연속으로 완성했을 때 출력
		break;
	case 3:
		_DrawTextColor(46,19,"3 combo!",0xfc); // 3번 연속으로 완성했을 때 출력
		break;
	case 4:
		_DrawTextColor(46,19,"4 combo!!",0xfc); // 4번 연속으로 완성했을 때 출력
		break;
	case 5:
		_DrawTextColor(46,19,"5 combo!!",0xfc); // 5번 연속으로 완성했을 때 출력
		break;
	case 6:
		_DrawTextColor(46,19,"6 combo!!!",0xfc); // 6번 연속으로 완성했을 때 출력
		break;
	case 7:
		_DrawTextColor(46,19,"7 combo!!!",0xfc); // 7번 연속으로 완성했을 때 출력
		break;
	case 8:
		_DrawTextColor(46,19,"8 combo!!!!",0xfc); // 8번 연속으로 완성했을 때 출력
		break;
	case 9:
		_DrawTextColor(46,19,"9 combo!!!!",0xfc); // 9번 연속으로 완성했을 때 출력
		break;
	case 10:
		_DrawTextColor(46,19,"10 combo!!!!!",0xfc); // 10번 연속으로 완성했을 때 출력
		break;
	case 11:
		_DrawTextColor(46,19,"11 combo!!!!!",0xfc); // 11번 연속으로 완성했을 때 출력
		break;
	}

	if (gameover==1)
	{
		if (user.point>=record.point&&user.point>0)
			_DrawTextColor(32,15,"   NEW RECORD!!! ",0xfc); // 최고기록 츨력

		_DrawTextColor(32,19,"──────────────────",0xfc);
		_DrawTextColor(33,20,"Game Over! press 'r' key to restart",0xec);	  // 게임 오버 출력
		_DrawTextColor(32,21,"──────────────────",0xfc);
	}

}

void ScoreLevel() // 점수와 레벨 올리는 함수
{
	if(user.point%1000+100*(combo+1)+100*combocount>=1000) // 레벨 업
	{
		user.point+=((combo+1)*100+100*combocount);    // 점수 증가 (1줄: 100, 2줄: 300, 3줄: 600, 4줄: 1000)
		if(speed>70)								 // 2 콤보    (1줄: 200, 2줄: 500, 3줄: 900, 4줄: 1400)
			speed=7*speed/8;					     // 3 콤보    (1줄: 300, 2줄: 700, 3줄: 1200, 4줄: 1800)  .....
		levelup=1;									 
		levelint++;
	}
	else
		user.point+=((combo+1)*100+100*combocount);  

	combo++; // 콤보 업
}

void PrintScoreLevel()  // int형을 char형으로 변환시켜줌
{
	score[0]=user.point/10000+48;
	score[1]=(user.point%10000)/1000+48;
	score[2]=(user.point%1000)/100+48;

	level[0]=levelint/10+48;
	level[1]=levelint%10+48;
}

void GetRecord()	 // 최고기록 불러옴
{
	FILE* fp=fopen("data\\record.sav","rb");
	if(fp==NULL)
	{
		return;
	}
	fscanf(fp,"%d %s",&record.point,record.name);

	recordc[0]=record.point/10000+48;
	recordc[1]=(record.point%10000)/1000+48;  // char형으로 변환시킴
	recordc[2]=(record.point%1000)/100+48;
	fclose(fp);
}

void WriteRecord()		// 최고기록 저장
{
	if (user.point>record.point) // 현재 점수가 기록보다 높을 때!! 최고기록 달성~!! ㅊㅊ
	{
		FILE* fp=fopen("data\\record.sav","wb");
		record.point=user.point;
		recordc[0]=record.point/10000+48;
		recordc[1]=(record.point%10000)/1000+48;
		recordc[2]=(record.point%1000)/100+48;
		strcpy(record.name,user.name);  // 최고기록에 유저 이름을 복사

		fprintf(fp,"%d %s",record.point,record.name);
		fclose(fp);
	}
}


void DrawData() // 테트리스 칸 표시
{
	int x,y;

	for(y=0; y<MY_H; y++)
	{
		for(x=0; x<MY_W; x++)
		{
			if(MY_WALL==g_data[y][x])
			{
				_DrawTextColor(x*2,y,"◆",0x78); // 칸 외부 벽 표시
			}

			else if(MY_BLANK == g_data[y][x])
			{
				_DrawTextColor(x*2,y," ",0xf7); // 칸 내의 여백 표시
			}

			else
			{
				_DrawTextColor(x*2,y,"■",0xf8); // 떨어진 블럭 표시
			}
		}
	}
}


void DrawBlock() // 떨어지고 있는 블럭을 표시
{
	int x,y,i;	

	for(i=0; i<4; i++)
	{
		x=g_pos.x +g_type[g_patindex][g_dir][i].x;
		y=g_pos.y +g_type[g_patindex][g_dir][i].y;
		_DrawTextColor(x*2,y,"■",color);
	}
}


void DataIni() // 테트리스 전체 틀 (벽) 초기화
{
	int x,y;

	for(y=0; y<MY_H; y++)
	{
		for(x=0;x<MY_W; x++)
		{
			if(0==x || y==0 || MY_W ==(x+1) || MY_H ==(y+1))
			{
				g_data[y][x]=MY_WALL; // x,y는 벽
			}

			else
			{
				g_data[y][x]=MY_BLANK; // x,y는 빈공간
			}

		}
	}
}

void CheckSameBlock() // 블럭이 한줄로 될때 확인
{
	int x,y,count=0;
	combo=0;
	levelup=0;

	for(y=MY_H-2; 0<y ; y--)
	{
		int same=0;
		for(x=1; x<MY_W-1; x++)
		{
			if(MY_BLOCK != g_data[y][x]) // x 축 한줄에 빈공간이 있을때
			{
				same=-1;				
				break;
			}
		}

		if(0==same) // x 축 한줄이 블럭일때
		{

			DownBlock(y); // 블럭들을 아래로 떨어뜨린다
			y++;			
			ScoreLevel();
			PrintScoreLevel();
		}
	}
}

int CheckBlockMove(int dx, int dy, int dir) // 블럭이 움직일 수 있는 칸인지 확인
{
	int x,y;
	int i;
	
	for(i=0; i<4; i++)
	{
		x=dx+g_type[g_patindex][dir][i].x;
		y=dy+g_type[g_patindex][dir][i].y;
		if(MY_BLANK != g_data[y][x])
		{

			return -1; // 움직일 수 없다
		}
	}

	return 0;
}


int MoveBlockDown() // 블럭이 아래로 떨어짐
{

	if(0==CheckBlockMove(g_pos.x, g_pos.y+1, g_dir)) // 블럭이 움직일 수 있는 칸일때
	{
		g_pos.y++; // y좌표 1씩 증가 (아래로 떨어짐)
		return 0;
	}

	return -1;
}

void CopyBlockToData() // 블럭데이터 복사
{

	int x,y;
	int i;

	for(i=0; i<4; i++)
	{

		x=g_pos.x + g_type[g_patindex][g_dir][i].x;
		y=g_pos.y + g_type[g_patindex][g_dir][i].y;

		g_data[y][x]=MY_BLOCK; // 테트리스 칸 안에 블럭데이터 대입

	}
}

void GetNewBlock() // 새로운 블럭 생성
{	
	g_pos.x=5; // 떨어지기 시작하는 위치 (x좌표)
	g_pos.y=1; // 떨어지기 시작하는 위치 (y좌표)

	g_dir=g_dirNext1; // 나올 패턴1을 나오는 패턴에 대입
	g_patindex=g_patindexNext1;
	color=colorNext1;

	g_dirNext1=g_dirNext2; // 나올 패턴2를 나올 패턴1에 대입
	g_patindexNext1=g_patindexNext2;
	colorNext1=colorNext2;
	
	g_dirNext2=rand()%4; // 앞으로 나올 패턴의 방향 랜덤 생성
	g_patindexNext2=rand()%MY_PATSIZE; // 앞으로 나올 패턴 종류 랜덤 생성 	
	colorNext2=g_patindexNext2+241; // 앞으로 나올 블럭 색상 랜덤 생성
	if(colorNext2==241+6)
		colorNext2+=6;

	if (combo>=1) // 블럭이 완성됐을 때
	{
		FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[4],0,&g_Channel[1]); // 블럭이 완성됐을 때 소리 재생
		combocount++;
	}
	else
	{
		FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[3],0,&g_Channel[1]); // 블럭이 떨어졌을 때 소리 재생
		combocount=0;
	}

	if (levelup)
		FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[5],0,&g_Channel[1]); // 레벨업 했을 때 소리 재생
}



void MoveToEnd() // 스페이스바 눌렀을때
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

void DownBlock(int y) // 테트리스 칸 전체를 아래로 평행이동
{

	int x;

	for(; 1<=y; y--) // y 좌표가 1보다 클 때 떨어짐
	{
		for(x=1; x<MY_W -1; x++)
		{
			if(1==y)
			{
				g_data[y][x]=MY_BLANK; // y좌표 1일때 빈공간
			}

			else
			{

				g_data[y][x]=g_data[y-1][x]; // 테트리스 칸이 내려옴				
			}
		}
	}
}

//////////////////////////


void Draw() // 화면에 표시
{
	DrawData(); // 틀을 만든다.
	DrawBlock(); // 블록을 만든다
	DrawHelp(); // 옆에 도움말 표시
}

void RunIni() // 게임 초기화
{
	DataIni(); // 데이터 초기화
	_Invalidate(); // 배경색깔 + 테트리스 창 출력
}

int RunKey() // 키 입력받는 함수
{

	char k;

	k=_GetKey();
	if(-1==k) return 0;

	if('q'==k) // q 입력받으면 -1 (종료) 반환
	{
		return -1;
	}

	else if ('p'==k)  // 게임 일시정지
	{
		_MessageBoxColor(32,21,38,2,"Paused. press any key to resume",0xf4); 
	}

	else if ('r'==k)  // r키 누르면 게임 초기화 (리셋)
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

	else if ('m'==k) // m키 누르면 BGM 재생 or 정지
	{
		if (musicswitch==1)
		{
			FMOD_Channel_Stop(g_Channel[0]); // 타이틀 화면 BGM 정지
			musicswitch=0;
		}

		else
		{
			FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[1],0,&g_Channel[0]); // BGM 재생
			musicswitch=1;
		}
	}
	else if('t'==k)  // t키 누르면 타이틀 화면으로
	{
		system("cls");

		FMOD_Channel_Stop(g_Channel[0]); // 타이틀 화면 BGM 정지		
		GetRandomBlock();			
		RunIni();
		speed=adminspeed; // 스피드 초기화
		user.point=0;
		levelint=1; // 점수 레벨 초기화 (0으로)
		combo=0;
		levelup=0;
		g_pos.x=1;
		g_pos.y=1;

		system("cls");

		GetUserName();
		GameStart();
		GameMain();
	}

	if(MY_LEFT == k) // 방향키 왼쪽 입력받으면 왼쪽으로 이동
	{
		if(0==CheckBlockMove(g_pos.x-1, g_pos.y, g_dir)) // x좌표 -1 이 움직일 수 있는 칸이면 
		{
			g_pos.x--;
		}
	}

	else if(MY_RIGHT==k) // 방향키 오른쪽 입력받으면 오른쪽으로 이동
	{
		if(0==CheckBlockMove(g_pos.x+1, g_pos.y, g_dir))
		{
			g_pos.x++;
		}

	}

	else if(MY_SPACE ==k) // 스페이스바 입력받으면 맨 아래로 이동
	{
		MoveToEnd();
	}

	else if(MY_UP==k) // 방향키 위쪽 입력받으면 시계방향으로 회전
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

	else if(MY_DOWN ==k) // 방향키 아래쪽 입력받으면 시계방향으로 회전
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


void RunTimer() // 타이머
{
	static long oldT=0;
	long newT;
	static int killTimer=0;

	if(-1==killTimer)
		return;

	newT=_GetTickCount();
	if(abs(newT-oldT)<speed) // 떨어지는 속도
	{
		return;
	}
	else
	{
		oldT=newT;
	}
	
	if(-1==MoveBlockDown()) // 바닥에 떨어졌을 때 
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
			FMOD_System_PlaySound(g_System,FMOD_CHANNEL_FREE,g_Sound[2],0,&g_Channel[1]); // 게임 오버 사운드 재생

			while (1)
			{	
				if (_GetKey()=='r') // r키 누르면 게임 재시작
					break;
			}

			gameover=0;
			GetRandomBlock();			
			RunIni();
			killTimer=0;
			speed=adminspeed; // 스피드 초기화
			user.point=0;
			levelint=1; // 점수 레벨 초기화 (0으로)
			combo=0;
			levelup=0;
			PrintScoreLevel();
		}		
	}
	_Invalidate();
}

void GameMain() // 게임 메인 함수
{
	RunIni();	// 게임 초기화
	while(1)
	{
		RunTimer();
		if(-1==RunKey()) // 반환값이 -1 이면 게임 종료
		{
			break;
		}
	}
}

int main()
{	
	system("color f0");
	puts("\n\t * Loading....Please Wait....");
	Init(); // 배경음악 재생을 위한 초기화 함수
	FMOD_System_Update(g_System);
	srand(time(NULL));
	_BeginWindow();
	GetUserName();
	GameStart();	 // 프로그램 처음 실행시 화면
	speed=adminspeed;
	GameMain();
	_EndWindow();
	Release();
	return 0;
}