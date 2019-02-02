void io_hlt(void);
void io_cli(void);
void io_sti(void);
int io_load_eflags(void);
void io_store_eflags(int eflags);
int io_in8(int port);
void io_out8(int port, int data);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void asm_inthandler21(int* esp);
void asm_inthandler27(int* esp);
void asm_inthandler2c(int* esp);