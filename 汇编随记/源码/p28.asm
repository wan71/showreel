assume cs:code
code segment
          dw   0123H,0456H,0789H,0abcH,0defH,0fedH,0cbaH,0987H

    start:
          mov  bx,0
          mov  ax,0
          mov  cx,8

    s:    add  ax,cs:[bx]
          add  bx,2
          loop s

          mov  ax,4c00h
          int  21h
code ends
end start


