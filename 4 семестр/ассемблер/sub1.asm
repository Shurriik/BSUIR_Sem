DATA SEGMENT
    ANS DW ?
    OUT_MSG DB 'Result: $'
    ENDL DB 0Dh, 0Ah, '$'
DATA ENDS

CODE SEGMENT
    ASSUME CS:CODE, DS:DATA
MAIN:
    MOV AX, DATA
    MOV DS, AX
    
    MOV BX, 3
    CALL SUMPROC
    MOV CX, AX
    
    MOV BX, 4
    CALL SUMPROC
    ADD CX, AX
    
    MOV BX, 5
    CALL SUMPROC
    ADD CX, AX
    
    MOV ANS, CX
    
    LEA DX, OUT_MSG
    MOV AH, 9
    INT 21h
    
    MOV AX, ANS
    CALL HEXOUT
    
    LEA DX, ENDL
    MOV AH, 9
    INT 21h
    
    MOV AX, 4C00h
    INT 21h

SUMPROC PROC
    PUSH CX
    MOV AX, 0
    MOV CX, 1
SUM_REPEAT:
    ADD AX, CX
    INC CX
    CMP CX, BX
    JBE SUM_REPEAT
    POP CX
    RET
SUMPROC ENDP

HEXOUT PROC
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    
    MOV BX, AX
    MOV CX, 4
    
HEX_REPEAT:
    MOV DL, BH
    SHR DL, 1
    SHR DL, 1
    SHR DL, 1
    SHR DL, 1
    AND DL, 0Fh
    
    CMP DL, 10
    JL NUM_DIGIT
    SUB DL, 10
    ADD DL, 'A'
    JMP SHOW_DIGIT
    
NUM_DIGIT:
    ADD DL, '0'
    
SHOW_DIGIT:
    MOV AH, 2
    INT 21h
    
    ROL BX, 1
    ROL BX, 1
    ROL BX, 1
    ROL BX, 1
    LOOP HEX_REPEAT
    
    POP DX
    POP CX
    POP BX
    POP AX
    RET
HEXOUT ENDP

CODE ENDS
END MAIN