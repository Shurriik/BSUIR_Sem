DATA SEGMENT
    PROMPT DB 'Enter a character: $'
    RESULT_MSG DB 0Dh, 0Ah, 'Result: $'
    NEW_LINE DB 0Dh, 0Ah, '$'
DATA ENDS

CODE SEGMENT
    ASSUME CS:CODE, DS:DATA
START:
    MOV AX, DATA
    MOV DS, AX
    
MAIN_LOOP:
    LEA DX, PROMPT
    MOV AH, 9
    INT 21h
    
    MOV AH, 1
    INT 21h
    
    CMP AL, 1Bh
    JE EXIT_PROGRAM
    
    MOV BL, AL
    
    LEA DX, RESULT_MSG
    MOV AH, 9
    INT 21h
    
    MOV AL, BL
    
    CMP AL, '0'
    JB NOT_DIGIT
    CMP AL, '9'
    JA NOT_DIGIT
    MOV DL, 'N'
    JMP DISPLAY_RESULT
    
NOT_DIGIT:
    CMP AL, 'A'
    JB NOT_UPPER
    CMP AL, 'Z'
    JA NOT_UPPER
    MOV DL, 'U'
    JMP DISPLAY_RESULT
    
NOT_UPPER:
    CMP AL, 'a'
    JB NOT_LOWER
    CMP AL, 'z'
    JA NOT_LOWER
    MOV DL, 'L'
    JMP DISPLAY_RESULT
    
NOT_LOWER:
    MOV DL, '*'
    
DISPLAY_RESULT:
    MOV AH, 2
    INT 21h
    
    LEA DX, NEW_LINE
    MOV AH, 9
    INT 21h
    
    JMP MAIN_LOOP
    
EXIT_PROGRAM:
    MOV AH, 4Ch
    INT 21h
    
CODE ENDS
END START