void io_hlt(void);
void io_cli(void);
void io_sti(void);

int io_load_eflags(void);
void io_store_eflags(int eflags);

unsigned int io_load_cr0(void);
void io_store_cr0(unsigned int cr0);

int io_in8(int port);
void io_out8(int port, int data);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void load_tr(int tr);
void taskswitch4(void);

void asm_inthandler20(int* esp);
void asm_inthandler21(int* esp);
void asm_inthandler27(int* esp);
void asm_inthandler2c(int* esp);
