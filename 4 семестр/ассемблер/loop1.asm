DATA SEGMENT
    NUMBERS DB 2, -3, 0, 5, -7, 0, 0, 8, 6, -9
    ARR_LEN DW 10
    P_CNT DB 0
    N_CNT DB 0
    Z_CNT DB 0
    TTL_MSG DB 'Array Statistics Results$'
    ARR_MSG DB 0Dh, 0Ah, 'Original array: $'
    POS_MSG DB 0Dh, 0Ah, 'Positive numbers: $'
    NEG_MSG DB 0Dh, 0Ah, 'Negative numbers: $'
    ZER_MSG DB 0Dh, 0Ah, 'Zeros: $'
    SPACE_CHAR DB ' $'
DATA ENDS

CODE SEGMENT
    ASSUME CS:CODE, DS:DATA
MAIN:
    MOV AX, DATA
    MOV DS, AX
    
    LEA DX, TTL_MSG
    MOV AH, 9
    INT 21h
    
    LEA DX, ARR_MSG
    MOV AH, 9
    INT 21h
    
    LEA SI, NUMBERS
    MOV CX, ARR_LEN
    
DISPLAY_LOOP:
    MOV AL, [SI]
    CALL WRITE_NUM
    LEA DX, SPACE_CHAR
    MOV AH, 9
    INT 21h
    INC SI
    DEC CX
    JNZ DISPLAY_LOOP
    
    MOV P_CNT, 0
    MOV N_CNT, 0
    MOV Z_CNT, 0
    
    LEA SI, NUMBERS
    MOV CX, ARR_LEN
    
ANALYZE_LOOP:
    MOV AL, [SI]
    OR AL, AL
    JZ ZERO_CASE
    JS NEG_CASE
    
POS_CASE:
    INC P_CNT
    JMP NEXT_ITEM
    
NEG_CASE:
    INC N_CNT
    JMP NEXT_ITEM
    
ZERO_CASE:
    INC Z_CNT
    
NEXT_ITEM:
    INC SI
    DEC CX
    JNZ ANALYZE_LOOP
    
    LEA DX, POS_MSG
    MOV AH, 9
    INT 21h
    MOV AL, P_CNT
    CALL WRITE_NUM
    
    LEA DX, NEG_MSG
    MOV AH, 9
    INT 21h
    MOV AL, N_CNT
    CALL WRITE_NUM
    
    LEA DX, ZER_MSG
    MOV AH, 9
    INT 21h
    MOV AL, Z_CNT
    CALL WRITE_NUM
    
    MOV AX, 4C00h
    INT 21h

WRITE_NUM PROC
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    
    TEST AL, AL
    JNS NUM_POS
    PUSH AX
    MOV DL, '-'
    MOV AH, 2
    INT 21h
    POP AX
    NEG AL
    
NUM_POS:
    XOR AH, AH
    MOV BL, 10
    DIV BL
    CMP AL, 0
    JZ SHOW_UNITS
    ADD AL, '0'
    MOV DL, AL
    MOV AH, 2
    INT 21h
    
SHOW_UNITS:
    ADD AH, '0'
    MOV DL, AH
    MOV AH, 2
    INT 21h
    
    POP DX
    POP CX
    POP BX
    POP AX
    RET
WRITE_NUM ENDP

CODE ENDS
END MAIN