powint:
    li   x5,1  
    li   x6,0  
    li   x10,2  
    li   x11,3  
    li   x12, 0x10000000
loop_start:
    bge  x6,x11,loop_end 
    mul  x5,x5,x10       
    addi x6,x6,1         
    j loop_start         
loop_end:
    mv   x10,x5           
    sw   x10, 4(x12)
    ebreak