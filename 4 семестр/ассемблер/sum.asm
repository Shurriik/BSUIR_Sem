DATA SEGMENT
    A      DW 3     
    B      DW 5       
    RESULT DW ?        
DATA ENDS

CODE SEGMENT
    ASSUME CS:CODE, DS:DATA
START:
    MOV AX, DATA      
    MOV DS, AX        

    MOV AX, A         
    ADD AX, B          
    MOV RESULT, AX     

    MOV AH, 4Ch      
    INT 21h       
CODE ENDS
END START         