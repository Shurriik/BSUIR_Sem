.model small
.stack 100h

.data

Information db "Byte sent: $"
Received_Byte db ?
           
.code

Init_COM1 proc
   mov ah,0
   mov al,11100011b
   mov dx,0
   int 14h
   ret            
Init_COM1 endp

Init_COM2 proc
   mov ah,0
   mov al,11100011b
   mov dx,1
   int 14h
   ret            
Init_COM2 endp

Write_COM1 proc
   mov ah,1
   mov al,'A'
   mov dx,0
   int 14h
   ret 
Write_COM1 endp

Delay proc
   push cx
   mov cx,0FFFFh
DelayLoop:
   loop DelayLoop
   pop cx
   ret
Delay endp

Read_COM2 proc
    mov ah,2
    mov dx,1
    int 14h
    mov [Received_Byte], al
    ret
Read_COM2 endp

Output proc
   mov dl, [Received_Byte]
   mov ah,02h
   int 21h
   ret
Output endp

PrintInfo proc
   mov ah,9
   mov dx,offset Information
   int 21h
   ret
PrintInfo endp

Exit proc
    mov ax,4C00h
    int 21h
Exit endp

start:
   mov ax,@data
   mov ds,ax
   
   call Init_COM1
   call Init_COM2
   call Write_COM1
   call Delay         ; ∆дем передачи
   call PrintInfo
   call Read_COM2
   call Output
   call Exit

end start