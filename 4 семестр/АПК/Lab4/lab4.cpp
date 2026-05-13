#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>


char data[6];
char alarmData[3];
unsigned int delayTime = 0;
unsigned int regAddr[] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09 };
int alarmOn = 0;
int delayMode = 0;
int currentRate = 3;
char* months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                   "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

#define COLOR_RED   0x4C
#define COLOR_BLACK 0x07
#define TIMER_PORT  0x42
#define TIMER_CTRL  0x43
#define SPEAKER_PORT 0x61

void Menu();
void ShowTime();
int BCDtoDEC(int bcd);
int DECtoBCD(int dec);
void setTime();
void MyDelay(unsigned int ms);
void setAlarm();
void resetAlarm();
void SetRate(int rate);
int IsUpdating();
void InitRTC();
void GetTime(int* h, int* m, int* s);
void SetScreenColor(int color);
void AlarmTriggered();
int CheckAlarm();
void flushInput();
void gotoxy(int x, int y);
void iPhoneAlarmMelody();
void SoundOn(int freq);
void SoundOff();

int main() {
    InitRTC();
    Menu();
    return 0;
}

void flushInput() {
    while (kbhit()) {
        getch();
    }
}

void gotoxy(int x, int y) {
    union REGS r;
    r.h.ah = 2;
    r.h.bh = 0;
    r.h.dh = y;
    r.h.dl = x;
    int86(0x10, &r, &r);
}

void SetScreenColor(int color) {
    char far* screen = (char far*)MK_FP(0xB800, 1);
    int i;
    for (i = 0; i < 2000; i++) {
        *screen = color;
        screen += 2;
    }
}

void GetTime(int* h, int* m, int* s) {
    while (IsUpdating());
    outp(0x70, 0x04); *h = BCDtoDEC(inp(0x71));
    outp(0x70, 0x02); *m = BCDtoDEC(inp(0x71));
    outp(0x70, 0x00); *s = BCDtoDEC(inp(0x71));
}

int CheckAlarm() {
    int ch, cm, cs, ah, am, as;
    if (alarmOn != 1) return 0;

    GetTime(&ch, &cm, &cs);
    ah = BCDtoDEC(alarmData[0]);
    am = BCDtoDEC(alarmData[1]);
    as = BCDtoDEC(alarmData[2]);

    if (ch == ah && cm == am && cs >= as) {
        return 1;
    }
    return 0;
}

// включение звука на заданной частоте
void SoundOn(int freq) {
    int divisor = 1193180 / freq;

    //таймер
    outp(TIMER_CTRL, 0xB6);
    outp(TIMER_PORT, divisor & 0xFF);
    outp(TIMER_PORT, (divisor >> 8) & 0xFF);

    // динамик
    unsigned char spkr = inp(SPEAKER_PORT);
    outp(SPEAKER_PORT, spkr | 0x03);
}

//выключение звука
void SoundOff() {
    unsigned char spkr = inp(SPEAKER_PORT);
    outp(SPEAKER_PORT, spkr & 0xFC);
}

// мелодия будильника 
void iPhoneAlarmMelody() {

    SoundOn(329);
    delay(200);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(329);
    delay(200);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(329);
    delay(200);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(415);
    delay(200);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(523);
    delay(400);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(659);
    delay(200);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(587);
    delay(200);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(523);
    delay(400);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(659);
    delay(200);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
    delay(100);

    SoundOn(659);
    delay(400);
    if (kbhit()) { SoundOff(); return; }
    SoundOff();
}

void AlarmTriggered() {
    int i;
    int stop = 0;

    system("cls");
    printf("\n\n\n  ----------------------------------------------\n");
    printf("  |          !!! ALARM TRIGGERED !!!          |\n");
    printf("  ----------------------------------------------\n\n");
    printf("           Press ANY KEY to stop\n");

    // Воспроизводим мелодию 
    for (i = 0; i < 3 && !stop; i++) {
        SetScreenColor(COLOR_RED);
        iPhoneAlarmMelody();

        if (kbhit()) {
            stop = 1;
            SoundOff();
            break;
        }

        SetScreenColor(COLOR_BLACK);
        delay(100);

        if (kbhit()) {
            stop = 1;
            break;
        }
    }

    while (!stop && !kbhit()) {
        SetScreenColor(COLOR_RED);
        delay(200);
        if (kbhit()) break;

        SetScreenColor(COLOR_BLACK);
        delay(200);
    }

    SoundOff();
    SetScreenColor(COLOR_BLACK);
    flushInput();
    getch();
    alarmOn = 0;
}

void InitRTC() {
    disable();                          // Запрет на прерывание 
    outp(0xA1, inp(0xA1) & 0xFE);       //0xA1-контроллер прерывания(маска), 0xFE-сброс младшего п
    outp(0x70, 0x0B);
    outp(0x71, inp(0x71) | 0x02);
    enable();
}

void ShowTime() {
    int h, m, s, d, mo, y;
    int i;

    while (IsUpdating());
    for (i = 0; i < 6; i++) {
        outp(0x70, regAddr[i]);
        data[i] = inp(0x71);
    }

    s = BCDtoDEC(data[0]);
    m = BCDtoDEC(data[1]);
    h = BCDtoDEC(data[2]);
    d = BCDtoDEC(data[3]);
    mo = BCDtoDEC(data[4]);
    y = BCDtoDEC(data[5]);

    printf("-----------------------------\n");
    printf("|       RTC CLOCK            |\n");
    printf("-----------------------------\n");
    printf("| Time: %02d:%02d:%02d       \n", h, m, s);
    printf("| Date: %02d %s 20%02d       \n", d, months[mo - 1], y);
    printf("| Rate: %d Hz                \n",
        currentRate == 0 ? 8192 :
        currentRate == 1 ? 4096 :
        currentRate == 2 ? 2048 :
        currentRate == 3 ? 1024 : 256);
    printf("-----------------------------\n");
}

int IsUpdating() {
    outp(0x70, 0x0A);
    return (inp(0x71) & 0x80);
}

int BCDtoDEC(int bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

int DECtoBCD(int dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

void setTime() {
    int nd[6];

    system("cls");
    printf("-----------------------------\n");
    printf("|       SET TIME            |\n");
    printf("-----------------------------\n\n");

    do {
        printf("Year (21-100): ");
        flushInput();
        scanf("%d", &nd[5]);
    } while (nd[5] < 21 || nd[5] > 100);

    do {
        printf("Month (1-12): ");
        flushInput();
        scanf("%d", &nd[4]);
    } while (nd[4] < 1 || nd[4] > 12);

    do {
        printf("Day (1-31): ");
        flushInput();
        scanf("%d", &nd[3]);
    } while (nd[3] < 1 || nd[3] > 31);

    do {
        printf("Hours (0-23): ");
        flushInput();
        scanf("%d", &nd[2]);
    } while (nd[2] < 0 || nd[2] > 23);

    do {
        printf("Minutes (0-59): ");
        flushInput();
        scanf("%d", &nd[1]);
    } while (nd[1] < 0 || nd[1] > 59);

    do {
        printf("Seconds (0-59): ");
        flushInput();
        scanf("%d", &nd[0]);
    } while (nd[0] < 0 || nd[0] > 59);

    disable();
    while (IsUpdating());

    outp(0x70, 0x0B);
    outp(0x71, inp(0x71) | 0x80);

    data[5] = DECtoBCD(nd[5]);
    data[4] = DECtoBCD(nd[4]);
    data[3] = DECtoBCD(nd[3]);
    data[2] = DECtoBCD(nd[2]);
    data[1] = DECtoBCD(nd[1]);
    data[0] = DECtoBCD(nd[0]);

    for (int i = 0; i < 6; i++) {
        outp(0x70, regAddr[i]);
        outp(0x71, data[i]);
    }

    outp(0x70, 0x0B);
    outp(0x71, inp(0x71) & 0x7F);
    enable();

    printf("\n-----------------------------\n");
    printf("| Time set successfully!      |\n");
    printf("-----------------------------\n");
    delay(1000);
}

void setAlarm() {
    int h = 0, m = 0, s = 0;
    int cur_h, cur_m, cur_s;
    int pos = 0;
    char key;
    int lastSecond = -1;
    int running = 1;

    system("cls");
    printf("-----------------------------\n");
    printf("|       SET ALARM            |\n");
    printf("-----------------------------\n\n");

    int instr_line = 9;

    while (running) {
        GetTime(&cur_h, &cur_m, &cur_s);

        if (cur_s != lastSecond) {
            gotoxy(1, 5);
            printf("Current time: %02d:%02d:%02d     ", cur_h, cur_m, cur_s);
            lastSecond = cur_s;
        }

        gotoxy(1, 7);
        printf("Set alarm time: %02d:%02d:%02d     ", h, m, s);

        switch (pos) {
        case 0:
            gotoxy(17, 8);
            printf("__");
            break;
        case 1:
            gotoxy(20, 8);
            printf("__");
            break;
        case 2:
            gotoxy(23, 8);
            printf("__");
            break;
        }

        gotoxy(1, instr_line);
        printf("Use LEFT/RIGHT arrows to select field     ");
        gotoxy(1, instr_line + 1);
        printf("Use UP/DOWN arrows to change value        ");
        gotoxy(1, instr_line + 2);
        printf("Press ENTER to confirm, ESC to cancel     ");

        if (kbhit()) {
            key = getch();

            if (key == 27) {
                running = 0;
                gotoxy(1, instr_line + 4);
                printf("Alarm setting cancelled.                     ");
                delay(1000);
                return;
            }
            else if (key == 13) {
                running = 0;
                break;
            }
            else if (key == 75) {
                pos--;
                if (pos < 0) pos = 2;
                gotoxy(17, 8);
                printf("  ");
                gotoxy(20, 8);
                printf("  ");
                gotoxy(23, 8);
                printf("  ");
            }
            else if (key == 77) {
                pos++;
                if (pos > 2) pos = 0;
                gotoxy(17, 8);
                printf("  ");
                gotoxy(20, 8);
                printf("  ");
                gotoxy(23, 8);
                printf("  ");
            }
            else if (key == 72) {
                switch (pos) {
                case 0: h++; if (h > 23) h = 0; break;
                case 1: m++; if (m > 59) m = 0; break;
                case 2: s++; if (s > 59) s = 0; break;
                }
                gotoxy(1, 7);
                printf("Set alarm time: %02d:%02d:%02d     ", h, m, s);
            }
            else if (key == 80) {
                switch (pos) {
                case 0: h--; if (h < 0) h = 23; break;
                case 1: m--; if (m < 0) m = 59; break;
                case 2: s--; if (s < 0) s = 59; break;
                }
                gotoxy(1, 7);
                printf("Set alarm time: %02d:%02d:%02d     ", h, m, s);
            }
        }

        delay(50);
    }

    if (h == 0 && m == 0 && s == 0) {
        gotoxy(1, instr_line + 4);
        printf("Warning: Alarm set to 00:00:00. Continue? (y/n): ");
        key = getch();
        if (key != 'y' && key != 'Y') {
            gotoxy(1, instr_line + 4);
            printf("Alarm setting cancelled.                           ");
            delay(1000);
            return;
        }
    }

    alarmData[0] = DECtoBCD(h);
    alarmData[1] = DECtoBCD(m);
    alarmData[2] = DECtoBCD(s);
    alarmOn = 1;

    gotoxy(1, instr_line + 4);
    printf("Alarm set successfully! %02d:%02d:%02d               ", h, m, s);
    gotoxy(1, instr_line + 6);
    printf("Press any key to return to menu...                     ");

    flushInput();
    getch();
}

void resetAlarm() {
    alarmOn = 0;
}

void SetRate(int rate) {
    disable();
    while (IsUpdating());
    outp(0x70, 0x0A);
    outp(0x71, (inp(0x71) & 0xF0) | (rate & 0x0F));
    enable();
    currentRate = rate;
}

void MyDelay(unsigned int ms) {
    unsigned int i;
    for (i = 0; i < ms; i++) {
        delay(1);
    }
}

void Menu() {
    char choice;
    int lastSecond = -1;
    int h, m, s;

    while (1) {
        GetTime(&h, &m, &s);

        if (CheckAlarm()) {
            resetAlarm();
            AlarmTriggered();
            continue;
        }

        if (s != lastSecond) {
            system("cls");

            ShowTime();

            printf("\n-----------------------------\n");
            printf("|        MAIN MENU          |\n");
            printf("-----------------------------\n");
            printf("| 1 - Set time              |\n");
            printf("| 2 - Set delay             |\n");
            printf("| 3 - Set alarm             |\n");
            printf("| 4 - Change rate           |\n");
            printf("| 0 - Exit                  |\n");
            printf("-----------------------------\n");

            if (alarmOn == 1) {
                printf("\n-----------------------------\n");
                printf("| ALARM: %02d:%02d:%02d        |\n",
                    BCDtoDEC(alarmData[0]),
                    BCDtoDEC(alarmData[1]),
                    BCDtoDEC(alarmData[2]));
                printf("-----------------------------\n");
            }

            printf("\nChoice: ");
            lastSecond = s;
        }

        if (kbhit()) {
            choice = getch();
            flushInput();

            switch (choice) {
            case '0':
                printf("\n-----------------------------\n");
                printf("| Exiting...                   |\n");
                printf("-------------------------------\n");
                delay(500);
                return;
            case '1':
                setTime();
                break;
            case '2':
                system("cls");
                printf("-----------------------------\n");
                printf("|       SET DELAY           |\n");
                printf("-----------------------------\n\n");
                printf("Delay (ms): ");
                int ms;
                flushInput();
                scanf("%d", &ms);
                delayMode = 1;
                printf("\n- Delay started...         -\n");
                MyDelay(ms);
                delayMode = 0;
                printf("\n-----------------------------\n");
                printf("| Delay finished!              |\n");
                printf("-----------------------------\n");
                printf("Press any key...");
                flushInput();
                getch();
                break;
            case '3':
                if (alarmOn == 1) {
                    system("cls");
                    printf("-----------------------------\n");
                    printf("| Alarm already set!        |\n");
                    printf("-----------------------------\n");
                    printf("Reset? (y/n): ");
                    flushInput();
                    if (getch() == 'y' || getch() == 'Y') {
                        resetAlarm();
                        printf("\n- Alarm reset!            -\n");
                    }
                    delay(1000);
                }
                else {
                    setAlarm();
                }
                break;
            case '4':
                system("cls");
                printf("-----------------------------\n");
                printf("|      CHANGE RATE          |\n");
                printf("-----------------------------\n");
                printf("| 0 = 8192 Hz               |\n");
                printf("| 1 = 4096 Hz               |\n");
                printf("| 2 = 2048 Hz               |\n");
                printf("| 3 = 1024 Hz (default)     |\n");
                printf("| 4-15 = lower              |\n");
                printf("-----------------------------\n\n");
                printf("Rate (0-15): ");
                int r;
                flushInput();
                scanf("%d", &r);
                if (r >= 0 && r <= 15) {
                    SetRate(r);
                    printf("\n-----------------------------\n");
                    printf("| Rate: %d Hz               |\n",
                        r == 0 ? 8192 : r == 1 ? 4096 : r == 2 ? 2048 : r == 3 ? 1024 : 256);
                    printf("-----------------------------\n");
                }
                else {
                    printf("\n-----------------------------\n");
                    printf("| Invalid rate!               |\n");
                    printf("-----------------------------\n");
                }
                delay(1000);
                break;
            }
            lastSecond = -1;
        }

        delay(50);
    }
}