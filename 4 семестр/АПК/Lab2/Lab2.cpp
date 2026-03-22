#include <dos.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define COLOR_COUNT 7 

struct VIDEO
{
	unsigned char symbol;
	unsigned char attribute;
};

unsigned char colors[COLOR_COUNT] =
{ 0x71,0x62,0x43,0x54,0x35,0x26,0x17 };
char color = 0x89;

void changeColor()
{
	color = colors[rand() % COLOR_COUNT];
	return;
}

void print()
{
	char temp;
	int i, val;
	VIDEO far* screen = (VIDEO far*)MK_FP(0xB800, 0);

	// Чтение масок Master
	val = inp(0x21);
	for (i = 0; i < 8; i++)
	{
		temp = val % 2;
		val = val >> 1;
		screen->symbol = temp + '0';
		screen->attribute = color; 
		screen++;
	}
	screen++;

	val = inp(0xA1);  // маска Slave
	for (i = 0; i < 8; i++)
	{
		temp = val % 2;
		val = val >> 1;
		screen->symbol = temp + '0';
		screen->attribute = color;  
		screen++;
	}
	screen += 63;
	outp(0x20, 0x0A);  // чтение регистра запросов Master

	// Чтение регистров запросов Master
	val = inp(0x20);
	for (i = 0; i < 8; i++)
	{
		temp = val % 2;
		val = val >> 1;
		screen->symbol = temp + '0';
		screen->attribute = color; 
		screen++;
	}
	screen++;

	outp(0xA0, 0x0A);  // чтение регистра запросов Slave
	val = inp(0xA0);
	for (i = 0; i < 8; i++)
	{
		temp = val % 2;
		val = val >> 1;
		screen->symbol = temp + '0';
		screen->attribute = color;  
		screen++;
	}
	screen += 63;

	outp(0x20, 0x0B);  // чтение регистра обслуживания Master

	// Чтение регистров обслуживания Maser
	val = inp(0x20);
	for (i = 0; i < 8; i++)
	{
		temp = val % 2;
		val = val >> 1;
		screen->symbol = temp + '0';
		screen->attribute = color; 
		screen++;
	}
	screen++;

	outp(0xA0, 0x0B);  // чтение регистра обслуживания Slave
	val = inp(0xA0);
	for (i = 0; i < 8; i++)
	{
		temp = val % 2;
		val = val >> 1;
		screen->symbol = temp + '0';
		screen->attribute = color; 
		screen++;
	}
}

// IRQ 0-7
void interrupt(*oldint8) (...); 		// IRQ 0
void interrupt(*oldint9) (...); 		// IRQ 1
void interrupt(*oldint10) (...);		// IRQ 2
void interrupt(*oldint11) (...);		// IRQ 3
void interrupt(*oldint12) (...);		// IRQ 4
void interrupt(*oldint13) (...);		// IRQ 5
void interrupt(*oldint14) (...);		// IRQ 6
void interrupt(*oldint15) (...);		// IRQ 7

// IRQ 8-15
void interrupt(*oldint70) (...);		// IRQ 8
void interrupt(*oldint71) (...);		// IRQ 9
void interrupt(*oldint72) (...);		// IRQ 10
void interrupt(*oldint73) (...);		// IRQ 11
void interrupt(*oldint74) (...);		// IRQ 12
void interrupt(*oldint75) (...);		// IRQ 13
void interrupt(*oldint76) (...);		// IRQ 14
void interrupt(*oldint77) (...);		// IRQ 15

// Обраотчики MASTER (ВЕКТОРЫ 08h-0Fh) 
void interrupt  newintMaster0(...) { print(); oldint8(); }   // IRQ0 -> 08h
void interrupt  newintMaster1(...) { changeColor(); print(); oldint9(); } // IRQ1 -> 09h
void interrupt  newintMaster2(...) { changeColor(); print(); oldint10(); } // IRQ2 -> 0Ah
void interrupt  newintMaster3(...) { changeColor(); print(); oldint11(); } // IRQ3 -> 0Bh
void interrupt  newintMaster4(...) { changeColor(); print(); oldint12(); } // IRQ4 -> 0Ch
void interrupt  newintMaster5(...) { changeColor(); print(); oldint13(); } // IRQ5 -> 0Dh
void interrupt  newintMaster6(...) { changeColor(); print(); oldint14(); } // IRQ6 -> 0Eh
void interrupt  newintMaster7(...) { changeColor(); print(); oldint15(); } // IRQ7 -> 0Fh

// Обработчики SLAVE (ВЕКТОРЫ 60h-67h) 
void interrupt  newintSlave0(...) { changeColor(); print(); oldint70(); } // IRQ8  -> 60h
void interrupt  newintSlave1(...) { changeColor(); print(); oldint71(); } // IRQ9  -> 61h
void interrupt  newintSlave2(...) { changeColor(); print(); oldint72(); } // IRQ10 -> 62h
void interrupt  newintSlave3(...) { changeColor(); print(); oldint73(); } // IRQ11 -> 63h
void interrupt  newintSlave4(...) { changeColor(); print(); oldint74(); } // IRQ12 -> 64h
void interrupt  newintSlave5(...) { changeColor(); print(); oldint75(); } // IRQ13 -> 65h
void interrupt  newintSlave6(...) { changeColor(); print(); oldint76(); } // IRQ14 -> 66h
void interrupt  newintSlave7(...) { changeColor(); print(); oldint77(); } // IRQ15 -> 67h

void initialize()
{
	// Старые обработчики
	oldint8 = getvect(0x08);
	oldint9 = getvect(0x09);
	oldint10 = getvect(0x0A);
	oldint11 = getvect(0x0B);
	oldint12 = getvect(0x0C);
	oldint13 = getvect(0x0D);
	oldint14 = getvect(0x0E);
	oldint15 = getvect(0x0F);

	oldint70 = getvect(0x70);
	oldint71 = getvect(0x71);
	oldint72 = getvect(0x72);
	oldint73 = getvect(0x73);
	oldint74 = getvect(0x74);
	oldint75 = getvect(0x75);
	oldint76 = getvect(0x76);
	oldint77 = getvect(0x77);

	// Новые обработчики MASTER для векторов 08h-0Fh
	setvect(0x08, newintMaster0);  // IRQ0
	setvect(0x09, newintMaster1);  // IRQ1
	setvect(0x0A, newintMaster2);  // IRQ2
	setvect(0x0B, newintMaster3);  // IRQ3
	setvect(0x0C, newintMaster4);  // IRQ4
	setvect(0x0D, newintMaster5);  // IRQ5
	setvect(0x0E, newintMaster6);  // IRQ6
	setvect(0x0F, newintMaster7);  // IRQ7

	// Новые обработчики  SLAVE для векторов 60h-67h
	setvect(0x60, newintSlave0);  // IRQ8
	setvect(0x61, newintSlave1);  // IRQ9
	setvect(0x62, newintSlave2);  // IRQ10
	setvect(0x63, newintSlave3);  // IRQ11
	setvect(0x64, newintSlave4);  // IRQ12
	setvect(0x65, newintSlave5);  // IRQ13
	setvect(0x66, newintSlave6);  // IRQ14
	setvect(0x67, newintSlave7);  // IRQ15

	_disable(); // CLI

	// Перепрограммирование контроллера прерываний
	outp(0x20, 0x11);	// ICW1 - инициализация Master
	outp(0x21, 0x08);   // ICW2 - базовый вектор Master = 08h
	outp(0x21, 0x04);	// ICW3 - Slave на IRQ2
	outp(0x21, 0x01);	// ICW4 - 8086 режим

	outp(0xA0, 0x11);	// ICW1 - инициализация Slave
	outp(0xA1, 0x60);   // ICW2 - базовый вектор Slave = 60h
	outp(0xA1, 0x02);	// ICW3 - каскад на IRQ2 Master
	outp(0xA1, 0x01);	// ICW4 - 8086 режим

	_enable(); // STI
}

int main()
{
	unsigned far* fp;
	initialize();
	system("cls");

	printf("                   - mask\n");
	printf("                   - prepare\n");
	printf("                   - service\n");
	printf("Master   Slave\n");


	FP_SEG(fp) = _psp;		// Установить сегмент PSP
	FP_OFF(fp) = 0x2c;		// Смещение блока окружения
	_dos_freemem(*fp);		// Освободить память окружения

	// Завершить и остаться резидентом
	_dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);		

	return 0;
}