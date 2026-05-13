DATA SEGMENT
    INPUT_MSG DB 'Enter a character: $'
    OUTPUT_MSG DB 0Dh, 0Ah, 'Result: $'
    CRLF DB 0Dh, 0Ah, '$'
DATA ENDS

CODE SEGMENT
    ASSUME CS:CODE, DS:DATA
BEGIN:
    MOV AX, DATA
    MOV DS, AX
    
CYCLE:
    LEA DX, INPUT_MSG
    MOV AH, 9
    INT 21h
    
    MOV AH, 1
    INT 21h
    
    CMP AL, 1Bh
    JZ FINISH
    
    MOV BH, AL
    
    LEA DX, OUTPUT_MSG
    MOV AH, 9
    INT 21h
    
    MOV AL, BH
    
    CMP AL, '0'
    JC CHECK_UPPER
    CMP AL, '9'
    JNBE CHECK_UPPER
    MOV DL, 'N'
    JMP SHOW
    
CHECK_UPPER:
    CMP AL, 'A'
    JC CHECK_LOWER
    CMP AL, 'Z'
    JNBE CHECK_LOWER
    MOV DL, 'U'
    JMP SHOW
    
CHECK_LOWER:
    CMP AL, 'a'
    JC OTHER_CHAR
    CMP AL, 'z'
    JNBE OTHER_CHAR
    MOV DL, 'L'
    JMP SHOW
    
OTHER_CHAR:
    MOV DL, '*'
    
SHOW:
    MOV AH, 2
    INT 21h
    
    LEA DX, CRLF
    MOV AH, 9
    INT 21h
    
    JMP CYCLE
    
FINISH:
    MOV AH, 4Ch
    INT 21h
    
CODE ENDS
END BEGIN