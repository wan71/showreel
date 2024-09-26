assume cs:codesg,ds:datasg
datasg segment
    db '1. file         '
    db '2. edit         '
    db '3. search       '
    db '4. view         '
    db '5. options      '
    db '6. help         ' 
datasg ends

stacksg segment
    dw 0,0,0,0,0,0,0,0
stacksg ends

codesg segment

start:
    mov ax,stacksg
    mov ss,ax
    mov sp,16
    mov ax,datasg
    mov ds,ax
    mov bx,0
    mov cx,6
s0: push cx
    mov si,0
    mov cx,4
 s: mov al,[bx+si]
    and al,11011111b
    mov [bx+si],al
    inc si
    loop s 
    add bx,16
    pop cx
    loop s0
codesg ends
end start