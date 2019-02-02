#include <bootpack.h>
#include <nasmfunc.h>

void set_segment_descriptor(struct SegmentDescriptor* sd, unsigned int limit, int base, int ar)
{
    if (limit > 0xfffff) {
        ar |= 0x8000; // G_bit
        limit /= 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
}


void set_gate_descriptor(struct GateDescriptor* gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
}

void init_gdtidt(void)
{
    // 0x00270000 ~ 0x0027ffff
    struct SegmentDescriptor* segmentDescriptors = (struct SegmentDescriptor*) ADDR_GDT;
    // 0x0026f800 ~ 0x0026ffff
    struct GateDescriptor* gateDescriptors = (struct GateDescriptor*) ADDR_IDT;
    for (int i = 0; i < LIMIT_GDT / 8; i++)
        set_segment_descriptor(&segmentDescriptors[i], 0, 0, 0);

    // entire system memory
    set_segment_descriptor(&segmentDescriptors[1], 0xffffffff, 0x00000000, AR_DATA32_RW);
    // bootpack.mil
    set_segment_descriptor(&segmentDescriptors[2], LIMIT_BOOTPACK, ADDR_BOOTPACK, AR_CODE32_ER);

    load_gdtr(0xffff, ADDR_GDT);

    for (int i = 0; i <= LIMIT_IDT; i++)
        set_gate_descriptor(&gateDescriptors[i], 0, 0, 0);

    load_idtr(LIMIT_IDT, ADDR_IDT);

}