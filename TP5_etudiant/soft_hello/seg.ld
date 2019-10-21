
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

seg_kcode_base  = TO BE COMPLETED
seg_kdata_base  = TO BE COMPLETED
seg_kunc_base   = TO BE COMPLETED

seg_code_base   = TO BE COMPLETED
seg_data_base   = TO BE COMPLETED
seg_stack_base  = TO BE COMPLETED

seg_tty_base    = TO BE COMPLETED
seg_timer_base  = TO BE COMPLETED
seg_ioc_base    = TO BE COMPLETED
seg_dma_base    = TO BE COMPLETED
seg_gcd_base    = TO BE COMPLETED
seg_fb_base     = TO BE COMPLETED
seg_icu_base    = TO BE COMPLETED

