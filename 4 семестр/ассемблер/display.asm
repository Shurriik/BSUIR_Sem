CODE SEGMENT
    ASSUME CS:CODE
START:
    MOV DL, 'A'
    MOV AH, 2
    INT 21h
    MOV AH, 4Ch
    INT 21h
CODE ENDS
END START         
