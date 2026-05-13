DATA SEGMENT
    ARRAY DB 2, -3, 0, 5, -7, 0, 0, 8, 6, -9
    ARRAY_SIZE DW 10
    POS_COUNT DB 0
    NEG_COUNT DB 0
    ZERO_COUNT DB 0
    MSG_TITLE DB 'Array Statistics Results$'
    MSG_ARRAY DB 0Dh, 0Ah, 'Original array: $'
    MSG_POS DB 0Dh, 0Ah, 'Positive numbers: $'
    MSG_NEG DB 0Dh, 0Ah, 'Negative numbers: $'
    MSG_ZERO DB 0Dh, 0Ah, 'Zeros: $'
    MSG_SPACE DB ' $'
DATA ENDS

CODE SEGMENT
    ASSUME CS:CODE, DS:DATA
START:
    MOV AX, DATA
    MOV DS, AX
    
    MOV DX, OFFSET MSG_TITLE
    MOV AH, 9
    INT 21h
    
    MOV DX, OFFSET MSG_ARRAY
    MOV AH, 9
    INT 21h
    
    MOV SI, OFFSET ARRAY
    MOV CX, ARRAY_SIZE
    
DISPLAY_ARRAY_LOOP:
    MOV AL, [SI]
    CALL PRINT_NUMBER
    MOV DX, OFFSET MSG_SPACE
    MOV AH, 9
    INT 21h
    INC SI
    LOOP DISPLAY_ARRAY_LOOP
    
    MOV POS_COUNT, 0
    MOV NEG_COUNT, 0
    MOV ZERO_COUNT, 0
    
    MOV SI, OFFSET ARRAY
    MOV CX, ARRAY_SIZE
    
COUNT_LOOP:
    MOV AL, [SI]
    CMP AL, 0
    JE IS_ZERO
    JG IS_POSITIVE
    
IS_NEGATIVE:
    INC NEG_COUNT
    JMP NEXT_ELEMENT
    
IS_POSITIVE:
    INC POS_COUNT
    JMP NEXT_ELEMENT
    
IS_ZERO:
    INC ZERO_COUNT
    
NEXT_ELEMENT:
    INC SI
    LOOP COUNT_LOOP
    
    MOV DX, OFFSET MSG_POS
    MOV AH, 9
    INT 21h
    MOV AL, POS_COUNT
    CALL PRINT_NUMBER
    
    MOV DX, OFFSET MSG_NEG
    MOV AH, 9
    INT 21h
    MOV AL, NEG_COUNT
    CALL PRINT_NUMBER
    
    MOV DX, OFFSET MSG_ZERO
    MOV AH, 9
    INT 21h
    MOV AL, ZERO_COUNT
    CALL PRINT_NUMBER
    
    MOV AH, 4Ch
    INT 21h

PRINT_NUMBER PROC
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    
    CMP AL, 0
    JGE POSITIVE_NUM
    PUSH AX
    MOV DL, '-'
    MOV AH, 2
    INT 21h
    POP AX
    NEG AL
    
POSITIVE_NUM:
    MOV AH, 0
    MOV BL, 10
    DIV BL
    CMP AL, 0
    JE PRINT_UNITS
    ADD AL, '0'
    MOV DL, AL
    MOV AH, 2
    INT 21h
    
PRINT_UNITS:
    ADD AH, '0'
    MOV DL, AH
    MOV AH, 2
    INT 21h
    
    POP DX
    POP CX
    POP BX
    POP AX
    RET
PRINT_NUMBER ENDP

CODE ENDS
END START