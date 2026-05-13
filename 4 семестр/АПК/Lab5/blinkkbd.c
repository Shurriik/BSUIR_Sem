#include <dos.h>
#include <conio.h>
#include <stdio.h>

// Порты ввода-вывода для работы с клавиатурой
#define KBD_DATA_PORT        0x60  
#define KBD_STATUS_PORT      0x64  
#define PIC_COMMAND_PORT     0x20  
#define PIC_EOI              0x20   // Прерывание 

// Команды для отправки клавиатуре
#define KBD_CMD_SET_LEDS     0xED  
#define KBD_ACK              0xFA  
#define KBD_RESEND           0xFE  // Повтор отправки

// Таймауты и размеры очередей
#define MAX_RETRIES          3     
#define RESPONSE_TIMEOUT     18U   
#define CONTROLLER_TIMEOUT   18U  
#define KEY_QUEUE_SIZE       64   

// Определение типа указателя на функцию обработчика прерывания
typedef void interrupt(*isr_ptr)(void);

// Флаги состояния клавиш-модификаторов
#define SHIFT_FLAG_SHIFT   0x01  
#define SHIFT_FLAG_CTRL    0x02  
#define SHIFT_FLAG_ALT     0x04  
#define SHIFT_FLAG_CAPS    0x08  
#define SHIFT_FLAG_NUM     0x10  
#define SHIFT_FLAG_SCROLL  0x20  

// Глобальные переменные для работы с ответами клавиатуры
volatile unsigned char g_waiting_response = 0;     
volatile unsigned char g_response_ready = 0;       
volatile unsigned char g_last_response = 0;        
volatile unsigned char g_response_log[256];        
volatile unsigned int  g_response_count = 0;       

// Очередь скан-кодов нажатых клавиш (кольцевой буфер)
volatile unsigned char g_key_queue[KEY_QUEUE_SIZE]; 
volatile unsigned char g_key_head = 0;              
volatile unsigned char g_key_tail = 0;              

// Состояние клавиш-модификаторов и расширенных кодов
volatile unsigned char g_shift_flags = 0;           
volatile unsigned char g_extended_prefix = 0;       
volatile unsigned char g_last_make_code = 0;        
volatile unsigned long g_last_make_tick = 0;       

isr_ptr g_old_int09 = 0;  // Указатель на старый обработчик прерывания INT 09h


unsigned long ticks_now(void);

void queue_key(unsigned char scan)
{
    unsigned char next_tail = (unsigned char)((g_key_tail + 1) % KEY_QUEUE_SIZE);

    // Проверяем, не переполнена ли очередь
    if (next_tail != g_key_head) {
        g_key_queue[g_key_tail] = scan;
        g_key_tail = next_tail;
    }
}


void update_shift_state(unsigned char code)
{
    unsigned char is_break = (unsigned char)(code & 0x80);  // 0x80 = отпускание клавиши
    unsigned char scan = (unsigned char)(code & 0x7F);      

    switch (scan) {
    case 0x2A:  // Левый Shift
    case 0x36:  // Правый Shift
        if (is_break)
            g_shift_flags &= (unsigned char)(~SHIFT_FLAG_SHIFT);  // Снимаем флаг
        else
            g_shift_flags |= SHIFT_FLAG_SHIFT;                   // Устанавливаем флаг
        break;

    case 0x1D:  // Ctrl
        if (is_break)
            g_shift_flags &= (unsigned char)(~SHIFT_FLAG_CTRL);
        else
            g_shift_flags |= SHIFT_FLAG_CTRL;
        break;

    case 0x38:  // Alt
        if (is_break)
            g_shift_flags &= (unsigned char)(~SHIFT_FLAG_ALT);
        else
            g_shift_flags |= SHIFT_FLAG_ALT;
        break;

    case 0x3A:  // Caps Lock
        if (!is_break)  // Только нажатие
            g_shift_flags ^= SHIFT_FLAG_CAPS;  
        break;

    case 0x45:  // Num Lock
        if (!is_break)
            g_shift_flags ^= SHIFT_FLAG_NUM;
        break;

    case 0x46:  // Scroll Lock
        if (!is_break)
            g_shift_flags ^= SHIFT_FLAG_SCROLL;
        break;
    }
}

int should_queue_make_code(unsigned char scan)
{
    unsigned long now = ticks_now();

    // Если тот же код и прошло менее 1 тика - это автоповтор
    if (scan == g_last_make_code && (now - g_last_make_tick) <= 1UL) {
        return 0;
    }

    g_last_make_code = scan;
    g_last_make_tick = now;
    return 1;
}


//Обработчик прерывания клавиатуры (INT 09h)
void interrupt keyboard_isr(void)
{
    unsigned char code = inp(KBD_DATA_PORT);  // Читаем скан-код из порта данных

    // Обработка ответов клавиатуры (ACK или RESEND)
    if (g_waiting_response && (code == KBD_ACK || code == KBD_RESEND)) {
        g_last_response = code;
        g_response_ready = 1;  // Ответ получен

        if (g_response_count < sizeof(g_response_log)) {
            g_response_log[g_response_count++] = code;
        }

        outp(PIC_COMMAND_PORT, PIC_EOI);  // Отправляем EOI контроллеру прерываний
        return;
    }

    // Обработка расширенных префиксов (0xE0 для клавиш типа Insert, Home и т.д.)
    if (code == 0xE0 || code == 0xE1) {
        g_extended_prefix = 1;
        outp(PIC_COMMAND_PORT, PIC_EOI);
        return;
    }

    // Обновляем состояние модификаторов
    update_shift_state(code);

    // Если это нажатие (не отпускание), добавляем в очередь
    if ((code & 0x80) == 0) {
        if (!g_extended_prefix && should_queue_make_code(code)) {
            queue_key(code);
        }
    }

    g_extended_prefix = 0;  // Сбрасываем флаг расширенного кода
    outp(PIC_COMMAND_PORT, PIC_EOI);
}


//Устанавливает собственный обработчик прерывания клавиатуры
void install_keyboard_isr(void)
{
    disable();                     // Запрещаем прерывания
    setvect(0x09, keyboard_isr);   // Устанавливаем новый обработчик
    enable();                      // Разрешаем прерывания
}

// Стандартный обработчик прерывания
void restore_keyboard_isr(void)
{
    disable();
    setvect(0x09, g_old_int09);    // Восстанавливаем старый обработчик
    enable();
}


//текущее системное время в тиках (около 18.2 тиков в секунду)
unsigned long ticks_now(void)
{
    return biostime(0, 0L);
}

//Ожидает освобождения буфера ввода контроллера клавиатуры
int wait_input_buffer_empty(unsigned int timeout_ticks)
{
    unsigned long start = ticks_now();

    while ((ticks_now() - start) < timeout_ticks) {
        // Бит 0x02 = 1 означает, что буфер ввода занят
        if ((inp(KBD_STATUS_PORT) & 0x02) == 0) {
            return 1;  
        }
    }

    return 0; 
}


//Ожидает ответа от клавиатуры (ACK или RESEND)
int wait_for_response(unsigned char* response, unsigned int timeout_ticks)
{
    unsigned long start = ticks_now();

    while ((ticks_now() - start) < timeout_ticks) {
        if (g_response_ready) {
            disable();
            *response = g_last_response;
            g_response_ready = 0;
            g_waiting_response = 0;
            enable();
            return 1;
        }
    }

    // Таймаут - сбрасываем флаги
    disable();
    g_waiting_response = 0;
    enable();
    return 0;
}

//Отправляет байт данных клавиатуре с контролем ошибок
int send_keyboard_byte(unsigned char value)
{
    int attempt;

    for (attempt = 1; attempt <= MAX_RETRIES; ++attempt) {
        unsigned char response = 0;

        // Ожидаем освобождения буфера контроллера
        if (!wait_input_buffer_empty(CONTROLLER_TIMEOUT)) {
            cprintf("\r\nTimeout: controller input buffer is busy.\r\n");
            return 0;
        }

        // Отправляем команду
        disable();
        g_response_ready = 0;
        g_waiting_response = 1;
        outp(KBD_DATA_PORT, value);
        enable();

        // Ждем ответ
        if (!wait_for_response(&response, RESPONSE_TIMEOUT)) {
            cprintf("\r\nTimeout: no response for byte 0x%02X.\r\n", value);
            return 0;
        }

        cprintf("Return code: 0x%02X\r\n", response);

        if (response == KBD_ACK) {
            return 1;  
        }

        if (response != KBD_RESEND) {
            cprintf("Unexpected response for byte 0x%02X.\r\n", value);
            return 0; 
        }
  
    }

    cprintf("Transfer error: byte 0x%02X was not accepted after %d attempts.\r\n",
        value, MAX_RETRIES);
    return 0;
}


//Устанавливает состояние светодиодов клавиатуры
int set_keyboard_leds(unsigned char mask)
{
    // Отправляем команду установки LED
    if (!send_keyboard_byte(KBD_CMD_SET_LEDS)) {
        return 0;
    }

    // Отправляем маску LED
    if (!send_keyboard_byte(mask)) {
        return 0;
    }

    return 1;
}


//Формирует маску LED на основе текущего состояния флагов
unsigned char get_led_mask_from_bios_flags(void)
{
    unsigned char mask = 0;

    if (g_shift_flags & SHIFT_FLAG_CAPS) {
        mask |= 0x04;  // Caps Lock LED
    }
    if (g_shift_flags & SHIFT_FLAG_NUM) {
        mask |= 0x02;  // Num Lock LED
    }
    if (g_shift_flags & SHIFT_FLAG_SCROLL) {
        mask |= 0x01;  // Scroll Lock LED
    }

    return mask;
}


const char* key_name(unsigned char scan, unsigned char ascii)
{
    switch (scan) {
    case 0x01: return "Esc";
    case 0x0E: return "Backspace";
    case 0x0F: return "Tab";
    case 0x1C: return "Enter";
    case 0x1D: return "Ctrl";
    case 0x2A: return "Left Shift";
    case 0x36: return "Right Shift";
    case 0x38: return "Alt";
    case 0x39: return "Space";
    case 0x3A: return "Caps Lock";
    case 0x45: return "Num Lock";
    case 0x46: return "Scroll Lock";
    case 0x47: return "Home";
    case 0x48: return "Up";
    case 0x49: return "PgUp";
    case 0x4B: return "Left";
    case 0x4D: return "Right";
    case 0x4F: return "End";
    case 0x50: return "Down";
    case 0x51: return "PgDn";
    case 0x52: return "Ins";
    case 0x53: return "Del";
    case 0x3B: return "F1";
    case 0x3C: return "F2";
    case 0x3D: return "F3";
    case 0x3E: return "F4";
    case 0x3F: return "F5";
    case 0x40: return "F6";
    case 0x41: return "F7";
    case 0x42: return "F8";
    case 0x43: return "F9";
    case 0x44: return "F10";
    }

    // Печатные символы ASCII
    if (ascii >= 32 && ascii <= 126) {
        return "Printable";
    }

    return "Special";
}


unsigned char is_shift_active(void)
{
    return (unsigned char)((g_shift_flags & SHIFT_FLAG_SHIFT) != 0);
}


unsigned char is_caps_active(void)
{
    return (unsigned char)((g_shift_flags & SHIFT_FLAG_CAPS) != 0);
}


unsigned char translate_ascii(unsigned char scan)
{
    unsigned char shift = is_shift_active();
    unsigned char caps = is_caps_active();

    switch (scan) {
       
    case 0x01: return 0x1B;  // Escape
    case 0x02: return shift ? '!' : '1';
    case 0x03: return shift ? '@' : '2';
    case 0x04: return shift ? '#' : '3';
    case 0x05: return shift ? '$' : '4';
    case 0x06: return shift ? '%' : '5';
    case 0x07: return shift ? '^' : '6';
    case 0x08: return shift ? '&' : '7';
    case 0x09: return shift ? '*' : '8';
    case 0x0A: return shift ? '(' : '9';
    case 0x0B: return shift ? ')' : '0';
    case 0x0C: return shift ? '_' : '-';
    case 0x0D: return shift ? '+' : '=';
    case 0x0E: return 0x08;  // Backspace
    case 0x0F: return 0x09;  // Tab

    case 0x10: return (shift ^ caps) ? 'Q' : 'q';
    case 0x11: return (shift ^ caps) ? 'W' : 'w';
    case 0x12: return (shift ^ caps) ? 'E' : 'e';
    case 0x13: return (shift ^ caps) ? 'R' : 'r';
    case 0x14: return (shift ^ caps) ? 'T' : 't';
    case 0x15: return (shift ^ caps) ? 'Y' : 'y';
    case 0x16: return (shift ^ caps) ? 'U' : 'u';
    case 0x17: return (shift ^ caps) ? 'I' : 'i';
    case 0x18: return (shift ^ caps) ? 'O' : 'o';
    case 0x19: return (shift ^ caps) ? 'P' : 'p';
    case 0x1A: return shift ? '{' : '[';
    case 0x1B: return shift ? '}' : ']';
    case 0x1C: return 0x0D;  // Enter

    case 0x1E: return (shift ^ caps) ? 'A' : 'a';
    case 0x1F: return (shift ^ caps) ? 'S' : 's';
    case 0x20: return (shift ^ caps) ? 'D' : 'd';
    case 0x21: return (shift ^ caps) ? 'F' : 'f';
    case 0x22: return (shift ^ caps) ? 'G' : 'g';
    case 0x23: return (shift ^ caps) ? 'H' : 'h';
    case 0x24: return (shift ^ caps) ? 'J' : 'j';
    case 0x25: return (shift ^ caps) ? 'K' : 'k';
    case 0x26: return (shift ^ caps) ? 'L' : 'l';
    case 0x27: return shift ? ':' : ';';
    case 0x28: return shift ? '"' : '\'';
    case 0x29: return shift ? '~' : '`';
    case 0x2B: return shift ? '|' : '\\';

    case 0x2C: return (shift ^ caps) ? 'Z' : 'z';
    case 0x2D: return (shift ^ caps) ? 'X' : 'x';
    case 0x2E: return (shift ^ caps) ? 'C' : 'c';
    case 0x2F: return (shift ^ caps) ? 'V' : 'v';
    case 0x30: return (shift ^ caps) ? 'B' : 'b';
    case 0x31: return (shift ^ caps) ? 'N' : 'n';
    case 0x32: return (shift ^ caps) ? 'M' : 'm';
    case 0x33: return shift ? '<' : ',';
    case 0x34: return shift ? '>' : '.';
    case 0x35: return shift ? '?' : '/';
    case 0x39: return ' ';  // Пробел
    }

    return 0;  
}


void print_key_info(unsigned char scan, unsigned char ascii)
{
    cprintf("Key: %-12s  Scan=0x%02X  ASCII=0x%02X",
        key_name(scan, ascii), scan, ascii);

    // Если символ печатный, выводим его
    if (ascii >= 32 && ascii <= 126) {
        cprintf("  Char='%c'", ascii);
    }

    // Для клавиш-индикаторов
    if (scan == 0x3A || scan == 0x45 || scan == 0x46) {
        cprintf("  Indicator key");
    }

    cprintf("\r\n");
}

//Мигает одним светодиодом клавиатуры
int blink_single_led(unsigned char led_bit)
{
    unsigned char base_mask = get_led_mask_from_bios_flags();
    unsigned char off_mask = base_mask & (unsigned char)(~led_bit);
    unsigned char on_mask = base_mask | led_bit;

    // Выключаем LED
    if (!set_keyboard_leds(off_mask)) return 0;
    delay(120);  // Пауза 120 мс

    // Включаем LED
    if (!set_keyboard_leds(on_mask)) return 0;
    delay(120);

    // Снова выключаем
    if (!set_keyboard_leds(off_mask)) return 0;
    delay(120);

    // Восстанавливаем исходное состояние
    if (!set_keyboard_leds(base_mask)) return 0;

    return 1;
}


//Мигает соответствующим индикатором для клавиши Lock
int blink_indicator_for_key(unsigned char scan)
{
    switch (scan) {
    case 0x3A:  // Caps Lock
        cprintf("Blinking Caps Lock LED\r\n");
        return blink_single_led(0x04);
    case 0x45:  // Num Lock
        cprintf("Blinking Num Lock LED\r\n");
        return blink_single_led(0x02);
    case 0x46:  // Scroll Lock
        cprintf("Blinking Scroll Lock LED\r\n");
        return blink_single_led(0x01);
    }

    return 1;  
}

int main(void)
{
    unsigned int i;

    clrscr();  
    cprintf("Press keys to display scan and ASCII codes in hex.\r\n");
    cprintf("Press Esc to finish.\r\n");
    cprintf("Lock keys blink their own indicators.\r\n\r\n");

    // Сохраняем старый обработчик и устанавливаем свой
    g_old_int09 = getvect(0x09);
    install_keyboard_isr();

    while (1) {
        unsigned char scan;
        unsigned char ascii;

        // Ожидаем появления клавиши в очереди
        while (g_key_head == g_key_tail) {
        }

        // Извлекаем скан-код из очереди
        disable();
        scan = g_key_queue[g_key_head];
        g_key_head = (unsigned char)((g_key_head + 1) % KEY_QUEUE_SIZE);
        enable();

        // Преобразуем в ASCII
        ascii = translate_ascii(scan);

        // Выводим информацию о клавише
        print_key_info(scan, ascii);

        // Мигаем индикатором для Lock-клавиш
        if (!blink_indicator_for_key(scan)) {
            restore_keyboard_isr();
            return 1;  // Ошибка при мигании
        }

        // Если нажата Esc - выходим
        if (scan == 0x01) {
            break;
        }
    }

    // Восстанавливаем старый обработчик прерываний
    restore_keyboard_isr();

    cprintf("\r\nAll received return codes in hex:\r\n");
    for (i = 0; i < g_response_count; ++i) {
        cprintf("0x%02X ", g_response_log[i]);
        if ((i + 1) % 16 == 0) {
            cprintf("\r\n");
        }
    }

    cprintf("\r\n\r\nDone. Press any key.\r\n");
    getch(); 
    return 0;
}