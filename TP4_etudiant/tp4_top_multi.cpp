/**********************************************************************
 * File : tp4_top.cpp
 * Date : 15/11/2010
 * Author :  Alain Greiner
 * UPMC - LIP6
 * This program is released under the GNU public license
 *********************************************************************/

/*
 * SOCLIB_LGPL_HEADER_BEGIN
 *
 * This file is part of SoCLib, GNU LGPLv2.1.
 *
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 *
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, Asim
 *         alain.greiner@lip6.fr
 *
 * Maintainers: alain
 */
//#include <iostream>
//#include <systemc>
//#include <limits>
//#include <stdio.h>
//#include <stdarg.h>
//#include <sstream>

#include "vci_signals.h"
#include "vci_param.h"
#include "mapping_table.h"
#include "vci_vgsb.h"
#include "vci_xcache_wrapper.h"
#include "mips32.h"
#include "vci_multi_tty.h"
#include "vci_timer.h"
#include "vci_multi_icu.h" // PATCHED
#include "vci_gcd_coprocessor.h"
#include "vci_multi_dma.h" // PATCHED
#include "vci_block_device.h"
#include "vci_framebuffer.h"
#include "vci_simple_ram.h"
#include "alloc_elems.h" // PATCHED

#define NPROCS 4
#define FB_NPIXEL	128	// Frame buffer width
#define FB_NLINE	128	// Frame buffer heigth

#define SEG_RESET_BASE  0xBFC00000
#define SEG_RESET_SIZE  0x00001000

#define SEG_KERNEL_BASE 0x80000000
#define SEG_KERNEL_SIZE 0x00004000

#define SEG_KUNC_BASE   0x81000000
#define SEG_KUNC_SIZE   0x00001000

#define SEG_KDATA_BASE  0x82000000
#define SEG_KDATA_SIZE  0x00004000

#define SEG_DATA_BASE   0x01000000
#define SEG_DATA_SIZE   0x00004000

#define SEG_CODE_BASE   0x00400000
#define SEG_CODE_SIZE   0x00004000

#define SEG_STACK_BASE  0x02000000
#define SEG_STACK_SIZE  0x01000000

#define SEG_TTY_BASE    0x90000000
#define SEG_TTY_SIZE    16*NPROCS*4 // 4 registres d'un mot par terminal pour 32 terminaux ?

#define SEG_TIM_BASE    0x91000000
#define SEG_TIM_SIZE    16*NPROCS// 4 registres

#define SEG_IOC_BASE    0x92000000
#define SEG_IOC_SIZE    36 // 9 registres

#define SEG_DMA_BASE    0x93000000
#define SEG_DMA_SIZE    32*NPROCS// 5 registres d'un mot ?

#define SEG_FBF_BASE    0x96000000
//#define SEG_FBF_SIZE    FB_NPIXEL*FB_NLINE // image 128x128 1 octet par pixel
#define SEG_FBF_SIZE    0x411000

#define SEG_ICU_BASE    0x9F000000
#define SEG_ICU_SIZE    32*NPROCS// 5 registres d'un mot ?

#define SEG_GCD_BASE    0x95000000
#define SEG_GCD_SIZE    16// 4 registres d'un mot ?

// SRCID definition
#define SRCID_PROC      0
#define SRCID_IOC       1
#define SRCID_DMA       2

// TGTID definition
#define TGTID_ROM       0
#define TGTID_RAM       1
#define TGTID_TTY       2
#define TGTID_GCD       3
#define TGTID_TIM       4
#define TGTID_IOC       5
#define TGTID_DMA       6
#define TGTID_FBF       7
#define TGTID_ICU       8

// VCI fields width definition
#define cell_size       4
#define plen_size       8
#define addr_size       32
#define rerror_size     1
#define clen_size       1
#define rflag_size      1
#define srcid_size      12
#define trdid_size      4 // minumum a cause du multi dma?
#define pktid_size      1
#define wrplen_size     1

// Cache parameters definition
#define icache_ways     4
#define icache_sets     128
#define icache_words    8
#define dcache_ways     4
#define dcache_sets     128
#define dcache_words    8

int _main(int argc, char *argv[])
{
    using namespace sc_core;
    using namespace soclib::caba;
    using namespace soclib::common;

    typedef VciParams<cell_size,
            plen_size,
            addr_size,
            rerror_size,
            clen_size,
            rflag_size,
            srcid_size,
            pktid_size,
            trdid_size,
            wrplen_size> vci_param;

    ///////////////////////////////////////////////////////////////
    // command line arguments
    ///////////////////////////////////////////////////////////////
    int     nprocs              = 4;           // number of processors
    int     ncycles             = 1000000000;       // simulated cycles
    char    sys_path[256]       = "soft_multi/sys.bin";   // pathname for system code
    char    app_path[256]       = "soft_multi/app.bin";   // pathname for application code
    char    ioc_filename[256]   = "images.raw";  // pathname for the ioc file
    size_t  fbf_size            = 128;              // number of lines = number of pixels
    bool    debug               = false;            // debug activated
    int     from_cycle          = 0;                // debug start cycle

    std::cout << std::endl << "********************************************************" << std::endl;
    std::cout << std::endl << "******        tp4_soclib_multi                    ******" << std::endl;
    std::cout << std::endl << "********************************************************" << std::endl;

    if (argc > 1)
    {
        for( int n=1 ; n<argc ; n=n+2 )
        {
            if( (strcmp(argv[n],"-NCYCLES") == 0) && (n+1<argc) )
            {
                ncycles = atoi(argv[n+1]);
            }
            else if( (strcmp(argv[n],"-DEBUG") == 0) && (n+1<argc) )
            {
                debug = true;
                from_cycle = atoi(argv[n+1]);
            }
            else if( (strcmp(argv[n],"-SYS") == 0) && (n+1<argc) )
            {
                strcpy(sys_path, argv[n+1]) ;
            }
            else if( (strcmp(argv[n],"-APP") == 0) && (n+1<argc) )
            {
                strcpy(app_path, argv[n+1]) ;
            }
            else if( (strcmp(argv[n],"-IOCFILE") == 0) && (n+1<argc) )
            {
                strcpy(ioc_filename, argv[n+1]) ;
            }
            else if( (strcmp(argv[n],"-FBFSIZE") == 0) && (n+1<argc) )
            {
                fbf_size = atoi(argv[n+1]) ;
            }
            else if( (strcmp(argv[n],"-NPROCS") == 0) && (n+1<argc) )
            {
                nprocs = atoi(argv[n+1]);
                //if (!(nprocs < 0 || nprocs > 4))
                //nprocs = 1;
            }
            else
            {
                std::cout << "   Arguments on the command line are (key,value) couples." << std::endl;
                std::cout << "   The order is not important." << std::endl;
                std::cout << "   Accepted arguments are :" << std::endl << std::endl;
                std::cout << "   -NCYCLES number_of_simulated_cycles" << std::endl;
                std::cout << "   -NPROCS number_of_simulated_procs" << std::endl;
                std::cout << "   -IOCFILE file_name" << std::endl;
                std::cout << "   -FBFSIZE number_of_pixels" << std::endl;
                std::cout << "   -SYS sys_elf_pathname" << std::endl;
                std::cout << "   -APP app_elf_pathname" << std::endl;
                std::cout << "   -DEBUG debug_start_cycle" << std::endl;
                exit(0);
            }
        }
    }
    std::cout << std::endl;
    std::cout << "    ncycles      = " << ncycles << std::endl;
    std::cout << "    nprocs      =  "  << nprocs << std::endl;
    std::cout << "    sys_pathname = " << sys_path << std::endl;
    std::cout << "    app_pathname = " << app_path << std::endl;
    std::cout << "    ioc_filename = " << ioc_filename << std::endl;
    std::cout << "    icache_sets  = " << icache_sets << std::endl;
    std::cout << "    icache_words = " << icache_words << std::endl;
    std::cout << "    icache_ways  = " << icache_ways << std::endl;
    std::cout << "    dcache_sets  = " << dcache_sets << std::endl;
    std::cout << "    dcache_words = " << dcache_words << std::endl;
    std::cout << "    dcache_ways  = " << dcache_ways << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // Mapping Table
    //////////////////////////////////////////////////////////////////////////
    MappingTable maptab(32, IntTab(8), IntTab(2), 0xFF000000);

    maptab.add(Segment("seg_reset" , SEG_RESET_BASE , SEG_RESET_SIZE , IntTab(TGTID_ROM), true));

    maptab.add(Segment("seg_kernel", SEG_KERNEL_BASE, SEG_KERNEL_SIZE, IntTab(TGTID_RAM), true));
    maptab.add(Segment("seg_kdata" , SEG_KDATA_BASE , SEG_KDATA_SIZE , IntTab(TGTID_RAM), true));
    maptab.add(Segment("seg_kunc"  , SEG_KUNC_BASE  , SEG_KUNC_SIZE  , IntTab(TGTID_RAM), false));
    maptab.add(Segment("seg_code"  , SEG_CODE_BASE  , SEG_CODE_SIZE  , IntTab(TGTID_RAM), true));
    maptab.add(Segment("seg_data"  , SEG_DATA_BASE  , SEG_DATA_SIZE  , IntTab(TGTID_RAM), true));
    maptab.add(Segment("seg_stack" , SEG_STACK_BASE , SEG_STACK_SIZE , IntTab(TGTID_RAM), true));

    maptab.add(Segment("seg_tty"   , SEG_TTY_BASE   , SEG_TTY_SIZE*nprocs   , IntTab(TGTID_TTY), false));
    maptab.add(Segment("seg_timer" , SEG_TIM_BASE   , SEG_TIM_SIZE*nprocs   , IntTab(TGTID_TIM), false));
    maptab.add(Segment("seg_icu"   , SEG_ICU_BASE   , SEG_ICU_SIZE*nprocs   , IntTab(TGTID_ICU), false));
    maptab.add(Segment("seg_dma"   , SEG_DMA_BASE   , SEG_DMA_SIZE*nprocs   , IntTab(TGTID_DMA), false));
    maptab.add(Segment("seg_fbf"   , SEG_FBF_BASE   , SEG_FBF_SIZE          , IntTab(TGTID_FBF), false));
    maptab.add(Segment("seg_ioc"   , SEG_IOC_BASE   , SEG_IOC_SIZE          , IntTab(TGTID_IOC), false));
    maptab.add(Segment("seg_gcd"   , SEG_GCD_BASE   , SEG_GCD_SIZE          , IntTab(TGTID_GCD), false));

    std::cout << std::endl << maptab << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // Signals
    //////////////////////////////////////////////////////////////////////////
    sc_clock        signal_clk("signal_clk", sc_time( 1, SC_NS ), 0.5 );
    sc_signal<bool> signal_resetn("signal_resetn");

    VciSignals<vci_param>*  signal_vci_init_proc;
    signal_vci_init_proc = alloc_elems<VciSignals<vci_param> >("signal_vci_init_proc", nprocs);
    //VciSignals<vci_param>   signal_vci_init_proc("signal_vci_init_proc");
    //
    VciSignals<vci_param>   signal_vci_init_dma("signal_vci_init_dma");
    VciSignals<vci_param>   signal_vci_init_ioc("signal_vci_init_ioc");

    VciSignals<vci_param>   signal_vci_tgt_rom("signal_vci_tgt_rom");
    VciSignals<vci_param>   signal_vci_tgt_ram("signal_vci_tgt_ram");
    VciSignals<vci_param>   signal_vci_tgt_tty("signal_vci_tgt_tty");
    VciSignals<vci_param>   signal_vci_tgt_gcd("signal_vci_tgt_gcd");
    VciSignals<vci_param>   signal_vci_tgt_tim("signal_vci_tgt_tim");
    VciSignals<vci_param>   signal_vci_tgt_icu("signal_vci_tgt_icu");
    VciSignals<vci_param>   signal_vci_tgt_fbf("signal_vci_tgt_fbf");
    VciSignals<vci_param>   signal_vci_tgt_ioc("signal_vci_tgt_ioc");
    VciSignals<vci_param>   signal_vci_tgt_dma("signal_vci_tgt_dma");

    sc_signal<bool> signal_false("signal_false");

    //sc_signal<bool> signal_irq_proc("signal_irq_proc");
    sc_signal<bool>* signal_irq_proc = alloc_elems<sc_signal<bool> >("signal_irq_proc", nprocs);
    sc_signal<bool>* signal_irq_tim = alloc_elems<sc_signal<bool> >("signal_irq_tim", nprocs);
    sc_signal<bool>* signal_irq_tty = alloc_elems<sc_signal<bool> >("signal_irq_tty", nprocs);
    sc_signal<bool>* signal_irq_dma = alloc_elems<sc_signal<bool> >("signal_irq_dma", nprocs);
    sc_signal<bool> signal_irq_ioc("signal_irq_ioc");

    ///////////////////////////////////////////////////////////////
    // VCI Components : 3 initiators / 9 targets
    // The IOC & DMA components are both initiator & target.
    ///////////////////////////////////////////////////////////////
    // The ICU controls 4 input IRQs :
    // - IRQ[0] : timer
    // - IRQ[1] : tty
    // - IRQ[2] : ioc
    // - IRQ[3] : dma
    //////////////////////////////////////////////////////////////

    Loader loader(sys_path, app_path);

    VciXcacheWrapper<vci_param, Mips32ElIss >* proc[nprocs];
    char* name_proc[nprocs];
    char* name_tty[nprocs];
    std::vector<std::string> names_for_tty;

    for (int i=0; i<nprocs; i++)
    {
        name_proc[i] = new char[16];
        name_tty[i] = new char[16];
        sprintf(name_proc[i], "proc[%d]", i);
        proc[i] = new VciXcacheWrapper<vci_param,Mips32ElIss>(name_proc[i], i, maptab, IntTab(i),
                                                              icache_ways, icache_sets, icache_words,
                                                              dcache_ways, dcache_sets, dcache_words);
        // definition des noms des tty
        sprintf(name_tty[i], "tty[%d]", i);
        names_for_tty.push_back(name_tty[i]);
    }

    std::cout << "procs constructed" << std::endl;

    VciSimpleRam<vci_param>* rom;
    rom = new VciSimpleRam<vci_param>("rom", IntTab(TGTID_ROM), maptab, loader);

    std::cout << "rom constructed" << std::endl;

    VciSimpleRam<vci_param>* ram;
    ram = new VciSimpleRam<vci_param>("ram", IntTab(TGTID_RAM), maptab, loader);

    std::cout << "ram constructed" << std::endl;

    //VciMultiTty<vci_param>* tty;
    //tty = new VciMultiTty<vci_param>("tty", IntTab(TGTID_TTY), maptab, "tty", NULL);
    VciMultiTty<vci_param>* tty;
    tty = new VciMultiTty<vci_param>("tty",
                                     IntTab(TGTID_TTY),
                                     maptab,
                                     names_for_tty);

    std::cout << "multi tty constructed" << std::endl;

    VciGcdCoprocessor<vci_param>* gcd;
    gcd = new VciGcdCoprocessor<vci_param>("gcd",
                                           IntTab(TGTID_GCD),
                                           maptab);

    std::cout << "gcd constructed" << std::endl;

    VciTimer<vci_param>* timer;
    timer = new VciTimer<vci_param>("timer",
                                    IntTab(TGTID_TIM),
                                    maptab,
                                    nprocs);
    std::cout << "timer constructed" << std::endl;

    //VciIcu<vci_param>* icu;
    //icu = new VciIcu<vci_param>("icu", IntTab(TGTID_ICU), maptab, 4);
    // name,
    // tgtig
    // mapping
    // number of IT
    // number of output channelWHY?
    VciMultiIcu<vci_param>* icu;
    icu = new VciMultiIcu<vci_param>("icu",
                                     IntTab(TGTID_ICU),
                                     maptab,
                                     32, // 32 a mettre sinnon ca plante
                                     nprocs);
    std::cout << "Multi ICU constructed" << std::endl;

    //VciDma<vci_param>* dma;
    //dma = new VciDma<vci_param>("dma", maptab ,IntTab(SRCID_DMA) ,IntTab(TGTID_DMA), 4); // Burst size 4 ?

    VciMultiDma<vci_param>* dma; // dernier arg c'est le nb channel, le SRCID c'est maibntenant les procs
    dma = new VciMultiDma<vci_param>("dma",
                                     maptab,
                                     IntTab(nprocs),
                                     IntTab(TGTID_DMA),
                                     4*4,
                                     nprocs);
    // Burst size 4 ?
    std::cout << "dma constructed" << std::endl;

    VciFrameBuffer<vci_param>* fbf;
    fbf = new VciFrameBuffer<vci_param>("fbf",
                                        IntTab(TGTID_FBF),
                                        maptab,
                                        fbf_size,
                                        fbf_size);
    // on laisse le sub-sampling par defaut
    std::cout << "fbf constructed" << std::endl;

    VciBlockDevice<vci_param>* ioc;
    ioc = new VciBlockDevice<vci_param>("ioc",
                                        maptab,
                                        IntTab(nprocs),
                                        IntTab(TGTID_IOC),
                                        ioc_filename); // block size par defaut (512) et latency par defaut (0)
    std::cout << "ioc constructed" << std::endl;

    VciVgsb<vci_param>* bus; // ioc et l'autre + les procs
    bus = new VciVgsb<vci_param>("bus",
                                 maptab,
                                 nprocs + 2,
                                 9);
    std::cout << "bus constructed" << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // Net-List
    //////////////////////////////////////////////////////////////////////////
    for (int i=0; i<nprocs; i++){
        proc[i]->p_clk              (signal_clk);
        proc[i]->p_resetn           (signal_resetn);
        proc[i]->p_vci              (signal_vci_init_proc[i]);
        proc[i]->p_irq[0]           (signal_irq_proc[i]);
        proc[i]->p_irq[1]           (signal_false);
        proc[i]->p_irq[2]           (signal_false);
        proc[i]->p_irq[3]           (signal_false);
        proc[i]->p_irq[4]           (signal_false);
        proc[i]->p_irq[5]           (signal_false);
    }
    std::cout << "proc connected" << std::endl;

    rom->p_clk                      (signal_clk);
    rom->p_resetn                   (signal_resetn);
    rom->p_vci                      (signal_vci_tgt_rom);
    std::cout << "rom connected" << std::endl;

    ram->p_clk                      (signal_clk);
    ram->p_resetn                   (signal_resetn);
    ram->p_vci                      (signal_vci_tgt_ram);
    std::cout << "ram connected" << std::endl;

    gcd->p_clk                      (signal_clk);
    gcd->p_resetn                   (signal_resetn);
    gcd->p_vci                      (signal_vci_tgt_gcd);
    std::cout << "gcd connected" << std::endl;

    tty->p_clk                      (signal_clk);
    tty->p_resetn                   (signal_resetn);
    tty->p_vci                      (signal_vci_tgt_tty);
    for (int i=0; i<nprocs; i++){
        tty->p_irq[i]               (signal_irq_tty[i]);
    }
    std::cout << "tty connected" << std::endl;

    timer->p_clk                    (signal_clk);
    timer->p_resetn                 (signal_resetn);
    timer->p_vci                    (signal_vci_tgt_tim);
    for (int i=0; i<nprocs; i++){
        timer->p_irq[i]             (signal_irq_tim[i]);
    }
    std::cout << "timer connected" << std::endl;

    icu->p_clk                      (signal_clk);
    icu->p_resetn                   (signal_resetn);
    icu->p_vci                      (signal_vci_tgt_icu);
    std::cout << "icu connected" << std::endl;

    for (int i=0; i<nprocs; i++){
        //IRQ_OUT
        icu->p_irq_out[i]            (signal_irq_proc[i]);
    }
    //IRQ_IN
    //
    //IRQ IOC
    icu->p_irq_in[0]                (signal_irq_ioc);
    icu->p_irq_in[1]                (signal_false);
    icu->p_irq_in[2]                (signal_false);
    icu->p_irq_in[3]                (signal_false);
    // IRQ DMA
    icu->p_irq_in[4]                (signal_irq_dma[0]);
    icu->p_irq_in[5]                (signal_irq_dma[1]);
    icu->p_irq_in[6]                (signal_irq_dma[2]);
    icu->p_irq_in[7]                (signal_irq_dma[3]);
    // IRQ TIM
    icu->p_irq_in[8]                (signal_irq_tim[0]);
    icu->p_irq_in[9]                (signal_irq_tim[1]);
    icu->p_irq_in[10]               (signal_irq_tim[2]);
    icu->p_irq_in[11]               (signal_irq_tim[3]);
    // IRQ TTY
    icu->p_irq_in[12]               (signal_irq_tty[0]);
    icu->p_irq_in[13]               (signal_irq_tty[1]);
    icu->p_irq_in[14]               (signal_irq_tty[2]);
    icu->p_irq_in[15]               (signal_irq_tty[3]);


    for (int i=16; i<32; i++){
        icu->p_irq_in[i]            (signal_false);
    }

    //icu->p_irq_in[0]                (signal_irq_tim);
    //icu->p_irq_in[1]                (signal_irq_tty);
    //icu->p_irq_in[2]                (signal_irq_ioc);
    //icu->p_irq_in[3]                (signal_irq_dma);
    std::cout << "irq connected" << std::endl;

    fbf->p_clk                      (signal_clk);
    fbf->p_resetn                   (signal_resetn);
    fbf->p_vci                      (signal_vci_tgt_fbf);
    std::cout << "fbf connected" << std::endl;

    ioc->p_clk                      (signal_clk);
    ioc->p_resetn                   (signal_resetn);
    ioc->p_vci_initiator            (signal_vci_init_ioc);
    ioc->p_vci_target               (signal_vci_tgt_ioc);
    ioc->p_irq                      (signal_irq_ioc);
    std::cout << "ioc connected" << std::endl;

    dma->p_clk                      (signal_clk);
    dma->p_resetn                   (signal_resetn);
    dma->p_vci_initiator            (signal_vci_init_dma);
    dma->p_vci_target               (signal_vci_tgt_dma);
    for (int i=0; i<nprocs; i++){
        dma->p_irq[i]               (signal_irq_dma[i]);
    }
    std::cout << "dma connected" << std::endl;

    bus->p_clk                      (signal_clk);
    bus->p_resetn                   (signal_resetn);


    for (int i=0; i<nprocs; i++){
        bus->p_to_initiator[i]      (signal_vci_init_proc[i]);
    }

    bus->p_to_initiator[nprocs]     (signal_vci_init_ioc);
    bus->p_to_initiator[nprocs+1]   (signal_vci_init_dma);

    bus->p_to_target[TGTID_ROM]     (signal_vci_tgt_rom);
    bus->p_to_target[TGTID_RAM]     (signal_vci_tgt_ram);
    bus->p_to_target[TGTID_TTY]     (signal_vci_tgt_tty);
    bus->p_to_target[TGTID_GCD]     (signal_vci_tgt_gcd);
    bus->p_to_target[TGTID_TIM]     (signal_vci_tgt_tim);
    bus->p_to_target[TGTID_ICU]     (signal_vci_tgt_icu);
    bus->p_to_target[TGTID_DMA]     (signal_vci_tgt_dma);
    bus->p_to_target[TGTID_FBF]     (signal_vci_tgt_fbf);
    bus->p_to_target[TGTID_IOC]     (signal_vci_tgt_ioc);

    //////////////////////////////////////////////////////////////////////////
    // simulation
    //////////////////////////////////////////////////////////////////////////

    signal_false = false;
    signal_resetn = false;
    sc_start( sc_time( 1, SC_NS ) ) ;

    signal_resetn = true;
    for ( int n=1 ; n<ncycles ; n++ )
    {
        if( debug && (n > from_cycle) )
        {
            std::cout << "***************** cycle " << std::dec << n << std::endl;
            for (int i=0; i<nprocs; i++){
                proc[i]->print_trace(1);
            }
            bus->print_trace();
            timer->print_trace();
            rom->print_trace();
            ram->print_trace();

            for (int i=0; i<nprocs; i++){
                if( signal_irq_proc[i].read() ) std::cout << "IRQ_PROC " << std::dec << i << std::endl;
                if( signal_irq_tim[i].read() )  std::cout << "IRQ_TIM"   << std::dec << i << std::endl;
                if( signal_irq_tty[i].read() )  std::cout << "IRQ_TTY"   << std::dec << i << std::endl;
                if( signal_irq_dma[i].read() )  std::cout << "IRQ_DMA"   << std::dec << i << std::endl;
            }

            if( signal_irq_ioc.read() )  std::cout << "IRQ_IOC"  << std::endl;
        }
        sc_start( sc_time( 1 , SC_NS ) ) ;
    }

    sc_stop();

    return(0);

} // end _main

int sc_main (int argc, char *argv[])
{
    try {
        return _main(argc, argv);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown exception occured" << std::endl;
        throw;
    }
    return 1;
}

// Local Variables:
// tab-width: 4;
// c-basic-offset: 4;
// c-file-offsets:((innamespace . 0)(inline-open . 0));
// indent-tabs-mode: nil;
// End:
//
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

