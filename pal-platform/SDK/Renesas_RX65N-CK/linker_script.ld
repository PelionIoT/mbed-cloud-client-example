MEMORY
{
	RAM : ORIGIN = 0x4, LENGTH = 262140
	RAM2 : ORIGIN = 0x00800000, LENGTH = 393216
    DATAFLASH (!rx) : ORIGIN = 0x100000, LENGTH = 32768 
	ROM : ORIGIN = 0xFFE00000, LENGTH = 2097152
	OFS : ORIGIN = 0xFE7F5D00, LENGTH = 128
}
SECTIONS
{
	.fvectors 0xFFE00000 : AT(0xFFE00000)
	{
		KEEP(*(.fvectors))
	} >ROM
	.exvectors :
	{
		"_exvectors_start" = .;
		KEEP(*(.exvectors))
		"_exvectors_end" = .;
	} >ROM
	.text :
	{
		*(.text)
		*(.text.*)
		*(P)
		etext = .;
	} >ROM
	.gcc_except_table :
	{
		*(.gcc_except_table.*)
	} >ROM
	.rvectors ALIGN(4) :
	{
		_rvectors_start = .;
		INCLUDE ../pal-platform/SDK/Renesas_RX65N-CK/rx65n-cloud-kit/lib/third_party/mcu_vendor/renesas/FIT/RDP_v1.15_modified/r_bsp/mcu/all/linker_script_rvectors.ld_inc
		_rvectors_end = .;
	} >ROM
	.init :
	{
		KEEP(*(.init))
		__preinit_array_start = .;
		KEEP(*(.preinit_array))
		__preinit_array_end = .;
		__init_array_start = (. + 3) & ~ 3;
		KEEP(*(.init_array))
		KEEP(*(SORT(.init_array.*)))
		__init_array_end = .;
		__fini_array_start = .;
		KEEP(*(.fini_array))
		KEEP(*(SORT(.fini_array.*)))
		__fini_array_end = .;
	} >ROM
	.fini :
	{
		KEEP(*(.fini))
	} >ROM
	.got :
	{
		*(.got)
		*(.got.plt)
	} >ROM
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
		*(C_1)
		*(C_2)
		*(C)
		_erodata = .;
	} >ROM
	.eh_frame_hdr :
	{
		*(.eh_frame_hdr)
	} >ROM
	.eh_frame :
	{
		*(.eh_frame)
	} >ROM
	.jcr :
	{
		*(.jcr)
	} >ROM
	.tors :
	{
		__CTOR_LIST__ = .;
		. = ALIGN(2);
		___ctors = .;
		*(.ctors)
		___ctors_end = .;
		__CTOR_END__ = .;
		__DTOR_LIST__ = .;
		___dtors = .;
		*(.dtors)
		___dtors_end = .;
		__DTOR_END__ = .;
		. = ALIGN(2);
		_mdata = .;
		. += _edata - _data;
	} >ROM
	.pfram ALIGN(4):
	{
		_PFRAM_start = .;
		. += _RPFRAM_end -_RPFRAM_start;
		_PFRAM_end = .;
	} >ROM
	.data : AT(_mdata)
	{
		_data = .;
		*(.data)
		*(.data.*)
		*(D)
		*(D_1)
		*(D_2)
		_edata = .;
	} >RAM
	.rpfram ALIGN(4): AT(_PFRAM_start)
	{
		_RPFRAM_start = .;
		*(PFRAM)
		. = ALIGN(4);
		_RPFRAM_end = .;
	} >RAM
	.gcc_exc :
	{
		*(.gcc_exc)
	} >RAM
	.bss :
	{
		_bss = .;
		*(.bss)
		*(.bss.**)
		*(COMMON)
		*(B)
		*(B_1)
		*(B_2)
		
		_ebss = .;
		_end = .;
	} >RAM2

	RPFRAM2   (NOLOAD) : 
{
	_RPFRAM2_start = .;
	*(RPFRAM2)
	_RPFRAM2_end = .;
	} >RAM
	
	PFRAM2  (NOLOAD) : 
{
	_PFRAM2_start = .;
	*(PFRAM2)
	_PFRAM2_end = .;
	} >ROM
	
	C_PKCS11_STORAGE_1 :
	{
	} >DATAFLASH C_PKCS11_STORAGE_MIRROR_1 :
    {
    } >DATAFLASH
	.r_bsp_NULL :
	{
		. += 0x100;
		"_r_bsp_NULL_end" = .;
	} >RAM AT>RAM
.r_bsp_istack BLOCK(0x4) (NOLOAD) :
	{
		KEEP(*(.r_bsp_istack))
	} >RAM AT>RAM
.istack :
	{
		"_istack" = .;
	} >RAM AT>RAM
.r_bsp_ustack BLOCK(0x4) (NOLOAD) :
	{
		KEEP(*(.r_bsp_ustack))
	} >RAM AT>RAM
.ustack :
	{
		"_ustack" = .;
	} >RAM AT>RAM
}
