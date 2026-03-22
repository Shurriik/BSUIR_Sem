#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

unsigned int notes[] = { 329, 329, 415, 529, 589, 587, 523, 659 };
unsigned int note_delays[] = { 200, 200, 200, 200, 200, 200, 200, 400 };
int note_count = 8;

void PlaySound();
void StateWords();
void CharToBin(unsigned char state, char* str);
void TurnSpeaker(int isActive);
void SetCount(int iDivider);
void PrintCounters();
unsigned int ReadCounter(int channel);
void Menu();

int main() {
	Menu();
	return 0;
}

void Menu() {
	int choice = 0;
	while (1) {
		system("cls");
		printf("1 - Play sound");
		printf("\n2 - Print channels state words");
		printf("\n3 - Print counter values (CE)");
		printf("\n0 - Exit");

		printf("\n\nEnter choice: ");
		scanf("%d", &choice);
		if (choice >= 0 && choice <= 3) {
			switch (choice) {
			case 0:
				return;

			case 1:
				PlaySound();
				break;

			case 2:
				StateWords();
				printf("\n\nPress any key to continue: ");
				getch();
				break;

			case 3:
				PrintCounters();
				printf("\n\nPress any key to continue: ");
				getch();
				break;
			}
		}
	}
}

//функция считывающая слова состояния каналов
void StateWords()
{
	char* bin_state;
	int iChannel;
	unsigned char state;
	bin_state = (char*)calloc(9, sizeof(char));
	if (bin_state == NULL)
	{
		printf("Memory allocation error");
		exit(EXIT_FAILURE);
	}

	for (iChannel = 0; iChannel < 3; iChannel++)
	{
		switch (iChannel)
		{
		case 0:
		{
			outp(0x43, 0xE2); //заносим управляющее слово, 
			state = inp(0x40); //чтение слова состояния канала 0
			CharToBin(state, bin_state);
			printf("Channel 0x40 word: %s\n", bin_state);
			break;
		}
		case 1:
		{
			bin_state[0] = '\0';
			outp(0x43, 0xE4); 
			state = inp(0x41); 
			CharToBin(state, bin_state);
			printf("Channel 0x41 word: %s\n", bin_state);
			break;
		}
		case 2:
		{
			bin_state[0] = '\0';
			outp(0x43, 0xE8); 
			state = inp(0x42); 
			CharToBin(state, bin_state);
			printf("Channel 0x42 word: %s\n", bin_state);
			break;
		}
		}
	}
	free(bin_state);
	return;
}

//функция перевода в двоичный код
void CharToBin(unsigned char state, char* str)
{
	int i;
	char temp;
	for (i = 7; i >= 0; i--)
	{
		temp = state % 2;
		state /= 2;
		str[i] = temp + '0';
	}
	str[8] = '\0';
}

//функция значения счётчика
void SetCount(int iDivider) {
	long base = 1193180; //максимальная частота
	long kd;
	outp(0x43, 0xB6); //10110110 - канал 2, операция 4, режим 3, формат 0
	kd = base / iDivider;
	outp(0x42, kd % 256); // младший байт делителя
	kd /= 256;
	outp(0x42, kd); //старший байт делителя

	printf("Channel 2 divider set to: 0x%04lX (%ld)\n", base / iDivider, base / iDivider);
	return;
}

//функция работы с динамиком
void TurnSpeaker(int isActive) {
	if (isActive) {
		outp(0x61, inp(0x61) | 3); //устанавливаем 2 младших бита 11
		return;
	}
	else {
		outp(0x61, inp(0x61) & 0xFC); //устанавливаем 2 младших бита 00
		return;
	}
}

// Функция для чтения значения счетчика
unsigned int ReadCounter(int channel) {
	unsigned char low, high;

	// Команда "защелкнуть счетчик" для указанного канала
	switch (channel) {
	case 0:
		outp(0x43, 0x00); // 00 000 000 - канал 0, защелкнуть
		low = inp(0x40);
		high = inp(0x40);
		break;
	case 1:
		outp(0x43, 0x40); // 01 000 000 - канал 1, защелкнуть
		low = inp(0x41);
		high = inp(0x41);
		break;
	case 2:
		outp(0x43, 0x80); // 10 000 000 - канал 2, защелкнуть
		low = inp(0x42);
		high = inp(0x42);
		break;
	default:
		return 0;
	}

	return (high << 8) | low; // объединяем старший и младший байты
}

// Функция для расчета и вывода коэффициентов деления
void PrintCounters() {
	unsigned int ce0, ce1, ce2;

	ce0 = ReadCounter(0);
	ce1 = ReadCounter(1);
	ce2 = ReadCounter(2);

	printf("\nCounter values (CE)\n");
	printf("Channel 0 (IRQ0/Timer): 0x%04X (%u)\n", ce0, ce0);
	printf("Channel 1 (Refresh):    0x%04X (%u)\n", ce1, ce1);
	printf("Channel 2 (Speaker):    0x%04X (%u)\n", ce2, ce2);
}

//функция воспроизведения меложии
void PlaySound() {
	printf("\nPlaying variant 4 melody...\n");
	printf("Notes sequence:\n");
	printf("329(200) 329(200) 415(200) 529(200) 589(200) 587(200) 523(200) 659(400)\n");

	for (int i = 0; i < note_count; i++) {
		printf("\nPlaying note %d: %d Hz for %d ms\n", i + 1, notes[i], note_delays[i]);
		SetCount(notes[i]);
		TurnSpeaker(1); //включаем динамик
		delay(note_delays[i]); //устанавливаем длительность мс
		TurnSpeaker(0); //выключаем динамик

		if (i == note_count - 1) {
			printf("\nFinal counter values after playback:\n");
			PrintCounters();
		}

		delay(50); // пауза между нотами
	}
	printf("\nDone!\n");
	delay(500);
}