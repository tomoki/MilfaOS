void io_hlt(void);
void write_mem8(int addr, int data);


void MilfaMain(void)
{
    // 0xa0000 ~ 0xaffff is framebuffer
    for (int i = 0xa0000; i <= 0xaffff; i++)
        write_mem8(i, 10);

fin:
    io_hlt();
    goto fin;
}