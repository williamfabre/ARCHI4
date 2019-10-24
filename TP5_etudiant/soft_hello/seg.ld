
/**********************************************************
        File : seg.ld
        Author : Alain Greiner
        Date : December 2011
**********************************************************/

/* definition of the base address for all segments
The peripherals base addresses are referenced by the
software drivers and must be defined, even if these
peripherals are not present in the architecture */

seg_reset_base  = 0xBFC00000;

seg_kcode_base  = 0x80100000;
seg_kdata_base  = 0x80200000;
seg_kunc_base   = 0x80300000;

seg_code_base   = 0x30100000;
seg_data_base   = 0x30200000;
seg_stack_base  = 0x00000000;

seg_tty_base    = 0xB1000000;
seg_timer_base  = 0xA1000000;
seg_ioc_base    = 0x81000000;
seg_dma_base    = 0x92000000;
seg_gcd_base    = 0x93000000;
seg_fb_base     = 0xA2000000;
seg_icu_base    = 0x91000000;
