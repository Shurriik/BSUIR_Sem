DATA SEGMENT
    RESULT DW ?
    MSG_RES DB 'Result: $'
    NEW_LINE DB 0Dh, 0Ah, '$'
DATA ENDS

CODE SEGMENT
    ASSUME CS:CODE, DS:DATA
START:
    MOV AX, DATA
    MOV DS, AX
    
    MOV BX, 3
    CALL SUM_SUBROUTINE
    MOV CX, AX
    
    MOV BX, 4
    CALL SUM_SUBROUTINE
    ADD CX, AX
    
    MOV BX, 5
    CALL SUM_SUBROUTINE
    ADD CX, AX
    
    MOV RESULT, CX
    
    LEA DX, MSG_RES
    MOV AH, 9
    INT 21h
    
    MOV AX, RESULT
    CALL PRINT_HEX
    
    LEA DX, NEW_LINE
    MOV AH, 9
    INT 21h
    
    MOV AH, 4Ch
    INT 21h

SUM_SUBROUTINE PROC
    PUSH CX
    MOV AX, 0
    MOV CX, 1
SUM_LOOP:
    ADD AX, CX
    INC CX
    CMP CX, BX
    JBE SUM_LOOP
    POP CX
    RET
SUM_SUBROUTINE ENDP

PRINT_HEX PROC
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    
    MOV BX, AX
    MOV CX, 4
    MOV AH, 2
    
HEX_LOOP:
    MOV DL, BH
    SHR DL, 1
    SHR DL, 1
    SHR DL, 1
    SHR DL, 1
    CALL PRINT_DIGIT
    ROL BX, 1
    ROL BX, 1
    ROL BX, 1
    ROL BX, 1
    LOOP HEX_LOOP
    
    POP DX
    POP CX
    POP BX
    POP AX
    RET
PRINT_HEX ENDP

PRINT_DIGIT PROC
    PUSH AX
    
    AND DL, 0Fh
    CMP DL, 10
    JB DIGIT_0_9
    MOV AL, 'A'
    ADD AL, DL
    SUB AL, 10
    MOV DL, AL
    JMP PRINT_IT
    
DIGIT_0_9:
    MOV AL, '0'
    ADD AL, DL
    MOV DL, AL
    
PRINT_IT:
    MOV AH, 2
    INT 21h
    
    POP AX
    RET
PRINT_DIGIT ENDP

CODE ENDS
END START