DATA SEGMENT
    X DB 06H      ; X = 6
    Y DB 07H      ; Y = 7
    Z DB ?        ; Результат 
DATA ENDS

CODE SEGMENT
    ASSUME CS:CODE, DS:DATA
START:
    MOV AX, DATA
    MOV DS, AX
    
    MOV AL, X      ; AL = 06H
    ADD AL, Y      ; AL = 06H + 07H = 0DH 
    
    MOV AH, 0      ; AH = 0
    MOV BL, 8      ; BL = 8
    MUL BL         ; AX = 0DH × 8 = 68H 
    
    SUB AL, X      ; AL = 68H - 06H = 62H
    
    MOV BL, 2      ; BL = 2
    DIV BL         ; AL = 62H ÷ 2 = 31H
    
    MOV Z, AL      
    
    MOV BL, AL    
    
    MOV CL, 4
    SHR AL, CL     ; Сдвиг вправо на 4 бита
    CALL PRINT_HEX ; Вывод 16-ой цифры
    

    MOV AL, BL     
    AND AL, 0FH    ; младшие 4 бита = 1
    CALL PRINT_HEX 
    
    MOV AH, 4Ch
    INT 21h

PRINT_HEX PROC
    CMP AL, 10     ; Сравниг с 10
    JB DIGIT       ; Если меньше 10
    ADD AL, 'A'-10 ; Иначе - в букву
    JMP PRINT
DIGIT:
    ADD AL, '0'    ; Преобразование в цифру
PRINT:
    MOV DL, AL     
    MOV AH, 2      
    INT 21h        
    RET
PRINT_HEX ENDP

CODE ENDS
END START