void io_hlt(void);
void io_cli(void);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void io_out8(int port, int data);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);