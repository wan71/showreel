# 改变CS,IP的指令
事实：执行何处的指令，取决于CS:IP
应用： 可以通过改变CS:IP中的内容，来控制CPU要执行的目标指令
问题：如何改变CS:IP的值？

方法1：Debug中的R命令可以改变寄存器的值   ---rcs,----rip
        Dedug是调试手段，并非程序方式

方法2：用指令修改 
       mov cs 2000H
       mov ip 0000H
       (此方法有问题)

方法3：
用转移指令 jmp
jmp 1000:0003