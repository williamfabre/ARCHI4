/**********************************************************************
 * File : tp5_cluster_top.cpp
 * Date : 15/12/2013
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

#include <systemc>
#include <limits>

#include "vci_signals.h"
#include "vci_param.h"
#include "mapping_table.h"
#include "vci_vgmn.h"
#include "vci_local_crossbar.h"
#include "vci_xcache_wrapper.h"
#include "mips32.h"
#include "vci_multi_tty.h"
#include "vci_timer.h"
#include "vci_multi_icu.h"
#include "vci_multi_dma.h"
#include "vci_block_device.h"
#include "vci_framebuffer.h"
#include "vci_simple_ram.h"
#include "alloc_elems.h"
#include "gdbserver.h"

// Segments definition
// Bits [29:28] define the cluster index
// Bit 31 define the kernel protection
// - IOC in cluster 0
// - DMA in cluster 1
// - ICU in cluster 1
// - TIM in cluster 2
// - FBF in cluster 2
// - ROM in cluster 3
// - TTY in cluster 3

#define SEG_KERNEL_BASE  TO BE COMPLETED   // cluster 0 / tgt 0 / protected
#define SEG_KERNEL_SIZE  0x00004000

#define SEG_KDATA_BASE   TO BE COMPLETED    // cluster 0 / tgt 0 / protected
#define SEG_KDATA_SIZE   0x00004000

#define SEG_KUNC_BASE    TO BE COMPLETED    // cluster 0 / tgt 0 / protected
#define SEG_KUNC_SIZE    0x00001000

#define SEG_CODE_BASE    TO BE COMPLETED    // cluster 3 / tgt 0
#define SEG_CODE_SIZE    0x00004000

#define SEG_DATA_BASE    TO BE COMPLETED    // cluster 3 / tgt 0
#define SEG_DATA_SIZE    0x00010000

#define SEG_STACK0_BASE  TO BE COMPLETED    // cluster 0 / tgt 0
#define SEG_STACK1_BASE  TO BE COMPLETED    // cluster 1 / tgt 0
#define SEG_STACK2_BASE  TO BE COMPLETED    // cluster 2 / tgt 0
#define SEG_STACK3_BASE  TO BE COMPLETED    // cluster 3 / tgt 0
#define SEG_STACK_SIZE   0x00010000

#define SEG_TTY_BASE     TO BE COMPLETED    // cluster 3 / tgt 1 / protected
#define SEG_TTY_SIZE     0x00001000

#define SEG_TIM_BASE     TO BE COMPLETED    // cluster 2 / tgt 1 / protected
#define SEG_TIM_SIZE     0x00001000

#define SEG_IOC_BASE     TO BE COMPLETED    // cluster 0 / tgt 1 / protected
#define SEG_IOC_SIZE     0x00001000

#define SEG_DMA_BASE     TO BE COMPLETED    // cluster 1 / tgt 2 / protected
#define SEG_DMA_SIZE     0x00001000

#define SEG_FBF_BASE     TO BE COMPLETED    // cluster 2 / tgt 2 / protected
#define SEG_FBF_SIZE     0x00010000

#define SEG_ICU_BASE     TO BE COMPLETED    // cluster 1 / tgt 1 / protected
#define SEG_ICU_SIZE     0x00001000

#define SEG_ROM_BASE     TO BE COMPLETED    // cluster 3 / tgt 2 / protected
#define SEG_ROM_SIZE     0x00010000

// SRCID.LID  definition
// IOC and DMA are in different clusters
#define SRCID_PROC      TO BE COMPLETED
#define SRCID_IOC       TO BE COMPLETED
#define SRCID_DMA       TO BE COMPLETED

// TGTID.LID  definition: 
// IOC, TTY, TIM, ICU are in different clusters
// DMA, FBF, ROM are in different clusters
#define TGTID_RAM       TO BE COMPLETED
#define TGTID_TTY       TO BE COMPLETED
#define TGTID_TIM       TO BE COMPLETED
#define TGTID_ICU       TO BE COMPLETED
#define TGTID_IOC       TO BE COMPLETED
#define TGTID_DMA       TO BE COMPLETED
#define TGTID_FBF       TO BE COMPLETED
#define TGTID_ROM       TO BE COMPLETED

// VCI fields width definition
#define cell_size       4
#define plen_size       8
#define addr_size       32
#define rerror_size     1
#define clen_size       1
#define rflag_size      1
#define srcid_size      3
#define pktid_size      1
#define trdid_size      4
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
    int     ncycles             = 1000000000;       // simulated cycles
    char    sys_path[256]       = "soft/sys.bin";   // pathname for system code
    char    app_path[256]       = "soft/app.bin";   // pathname for application code
    char    ioc_filename[256]   = "to_be_defined";  // pathname for the ioc file
    size_t  fbf_size            = 128;              // number of lines = number of pixels
    bool    debug               = false;            // debug activated
    int     from_cycle          = 0;                // debug start cycle

    std::cout << std::endl << "********************************************************" << std::endl;
    std::cout << std::endl << "******        tp5_top                             ******" << std::endl;
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
            else
            {
                std::cout << "   Arguments on the command line are (key,value) couples." << std::endl;
                std::cout << "   The order is not important." << std::endl;
                std::cout << "   Accepted arguments are :" << std::endl << std::endl;
                std::cout << "   -NCYCLES number_of_simulated_cycles" << std::endl;
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
    std::cout << "    ncycles       = " << ncycles << std::endl;
    std::cout << "    sys_pathname  = " << sys_path << std::endl;
    std::cout << "    app_pathname  = " << app_path << std::endl;
    std::cout << "    ioc_filename  = " << ioc_filename << std::endl;
    std::cout << "    icache_sets   = " << icache_sets << std::endl;
    std::cout << "    icache_words  = " << icache_words << std::endl;
    std::cout << "    icache_ways   = " << icache_ways << std::endl;
    std::cout << "    dcache_sets   = " << dcache_sets << std::endl;
    std::cout << "    dcache_words  = " << dcache_words << std::endl;
    std::cout << "    dcache_ways   = " << dcache_ways << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // Mapping Table
    //////////////////////////////////////////////////////////////////////////
    MappingTable maptab(32, IntTab(4,4), IntTab(2,1), 0xFFF00000);

    maptab.add(Segment("seg_kernel", SEG_KERNEL_BASE, SEG_KERNEL_SIZE, IntTab(0,TGTID_RAM), true));
    maptab.add(Segment("seg_kdata" , SEG_KDATA_BASE , SEG_KDATA_SIZE , IntTab(0,TGTID_RAM), true));
    maptab.add(Segment("seg_kunc"  , SEG_KUNC_BASE  , SEG_KUNC_SIZE  , IntTab(0,TGTID_RAM), false));
    maptab.add(Segment("seg_code"  , SEG_CODE_BASE  , SEG_CODE_SIZE  , IntTab(3,TGTID_RAM), true));
    maptab.add(Segment("seg_data"  , SEG_DATA_BASE  , SEG_DATA_SIZE  , IntTab(3,TGTID_RAM), false));

    maptab.add(Segment("seg_stack0", SEG_STACK0_BASE, SEG_STACK_SIZE , IntTab(0,TGTID_RAM), true));
    maptab.add(Segment("seg_stack1", SEG_STACK1_BASE, SEG_STACK_SIZE , IntTab(1,TGTID_RAM), true));
    maptab.add(Segment("seg_stack2", SEG_STACK2_BASE, SEG_STACK_SIZE , IntTab(2,TGTID_RAM), true));
    maptab.add(Segment("seg_stack3", SEG_STACK3_BASE, SEG_STACK_SIZE , IntTab(3,TGTID_RAM), true));

    maptab.add(Segment("seg_tty"   , SEG_TTY_BASE   , SEG_TTY_SIZE   , IntTab(3,TGTID_TTY), false));
    maptab.add(Segment("seg_tim"   , SEG_TIM_BASE   , SEG_TIM_SIZE   , IntTab(2,TGTID_TIM), false));
    maptab.add(Segment("seg_icu"   , SEG_ICU_BASE   , SEG_ICU_SIZE   , IntTab(1,TGTID_ICU), false));
    maptab.add(Segment("seg_ioc"   , SEG_IOC_BASE   , SEG_IOC_SIZE   , IntTab(0,TGTID_IOC), false));
    maptab.add(Segment("seg_dma"   , SEG_DMA_BASE   , SEG_DMA_SIZE   , IntTab(1,TGTID_DMA), false));
    maptab.add(Segment("seg_fbf"   , SEG_FBF_BASE   , SEG_FBF_SIZE   , IntTab(2,TGTID_FBF), false));
    maptab.add(Segment("seg_rom"   , SEG_ROM_BASE   , SEG_ROM_SIZE   , IntTab(3,TGTID_ROM), true));

    std::cout << std::endl << maptab << std::endl;

    std::cout << "mapping table OK" << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // Signals
    //////////////////////////////////////////////////////////////////////////
    sc_clock        signal_clk("signal_clk", sc_time( 1, SC_NS ), 0.5 );
    sc_signal<bool> signal_resetn("signal_resetn");

    VciSignals<vci_param>*  signal_vci_l2g = alloc_elems<VciSignals<vci_param> >("signal_vci_l2g", 4);
    VciSignals<vci_param>*  signal_vci_g2l = alloc_elems<VciSignals<vci_param> >("signal_vci_g2l", 4);

    VciSignals<vci_param>*  signal_vci_proc = alloc_elems<VciSignals<vci_param> >("signal_vci_proc", 4);
    VciSignals<vci_param>*  signal_vci_ram  = alloc_elems<VciSignals<vci_param> >("signal_vci_ram", 4);

    VciSignals<vci_param>   signal_vci_ini_dma("signal_vci_ini_dma");
    VciSignals<vci_param>   signal_vci_tgt_dma("signal_vci_tgt_dma");

    VciSignals<vci_param>   signal_vci_ini_ioc("signal_vci_ini_ioc");
    VciSignals<vci_param>   signal_vci_tgt_ioc("signal_vci_tgt_ioc");
    VciSignals<vci_param>   signal_vci_tgt_icu("signal_vci_tgt_icu");
    VciSignals<vci_param>   signal_vci_tgt_tty("signal_vci_tgt_tty");
    VciSignals<vci_param>   signal_vci_tgt_tim("signal_vci_tgt_tim");
    VciSignals<vci_param>   signal_vci_tgt_rom("signal_vci_tgt_rom");
    VciSignals<vci_param>   signal_vci_tgt_fbf("signal_vci_tgt_fbf");

    sc_signal<bool> signal_false("signal_false");

    sc_signal<bool>* signal_irq_proc = alloc_elems<sc_signal<bool> >("signal_irq_proc", 4);
    sc_signal<bool>* signal_irq_tty  = alloc_elems<sc_signal<bool> >("signal_irq_tty", 4);
    sc_signal<bool>* signal_irq_tim  = alloc_elems<sc_signal<bool> >("signal_irq_tim", 4);
    sc_signal<bool>* signal_irq_dma  = alloc_elems<sc_signal<bool> >("signal_irq_dma", 4);
    sc_signal<bool> signal_irq_ioc("signal_irq_ioc");

    std::cout << "signals declaration OK" << std::endl;

    ////////////////////////////////////////////////////////////////////
    // VCI Components : 4 clusters
    // Each cluster contains 1 processor, 1 RAM, 1 TTY, 1 TIMER, 
    // 1 ICU and a LOCAL_CROSSBAR.
    // The cluster 0 contains IOC.
    // The cluster 1 contains DMA and ICU.
    // The cluster 2 contains FBF and TIM.
    // The cluster 3 contains ROM and TTY.
    // The global interconnect is a VGMN.
    ////////////////////////////////////////////////////////////////////

    Loader loader( sys_path, app_path );

    VciXcacheWrapper<vci_param, GdbServer<Mips32ElIss> >* 		proc[4];
    VciSimpleRam<vci_param>* 				                    ram[4];
    VciLocalCrossbar<vci_param>*			                    xbar[4];
    VciMultiTty<vci_param>* 				                    tty;
    VciTimer<vci_param>* 				                        tim;
    VciMultiIcu<vci_param>* 			                        icu;
    VciSimpleRam<vci_param>* 				                    rom;
    VciMultiDma<vci_param>* 			                        dma;
    VciFrameBuffer<vci_param>* 				                    fbf;
    VciBlockDevice<vci_param>* 				                    ioc;
    VciVgmn<vci_param>* 				                        noc;

    char* proc_name[4] = { "proc0", "proc1", "proc2", "proc3" };
    char* ram_name[4] = { "ram0", "ram1", "ram2", "ram3" };

    for ( size_t i=0 ; i<4 ; i++ )
    {
        proc[i] = new VciXcacheWrapper<vci_param, GdbServer<Mips32ElIss> > (
                      proc_name[i], 
                      i,
                      maptab,
                      IntTab(i,SRCID_PROC),
                      icache_ways, icache_sets, icache_words,
                      dcache_ways, dcache_sets, dcache_words );

        ram[i]  = new VciSimpleRam<vci_param>(
                      ram_name[i], 
                      IntTab(i, TGTID_RAM), 
                      maptab, 
                      loader );
    }

    std::cout << " - procs and rams constructed" << std::endl;

    xbar[0] = new VciLocalCrossbar<vci_param>( TO BE COMPLETED );
    xbar[1] = new VciLocalCrossbar<vci_param>( TO BE COMPLETED );
    xbar[2] = new VciLocalCrossbar<vci_param>( TO BE COMPLETED );
    xbar[3] = new VciLocalCrossbar<vci_param>( TO BE COMPLETED );

    std::cout << " - crossbars constructed" << std::endl;

    tty     = new VciMultiTty<vci_param>( TO BE COMPLETED );

    std::cout << " - tty constructed" << std::endl;

    tim    = new VciTimer<vci_param>( TO BE COMPLETED );

    std::cout << " - timer constructed" << std::endl;

    icu    = new VciMultiIcu<vci_param>( TO BE COMPLETED );

    std::cout << " - icu constructed" << std::endl;

    ioc    = new VciBlockDevice<vci_param>( TO BE COMPLETED );

    std::cout << " - ioc constructed" << std::endl;

    dma    = new VciMultiDma<vci_param>( TO BE COMPLETED );

    std::cout << " - dma constructed" << std::endl;

    fbf    = new VciFrameBuffer<vci_param>( TO BE COMPLETED );

    std::cout << " - fbf constructed" << std::endl;

    rom = new VciSimpleRam<vci_param>( TO BE COMPLETED );

    std::cout << " - rom constructed" << std::endl;

    noc = new VciVgmn<vci_param>( TO BE COMPLETED );

    std::cout << " - noc constructed" << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // Net-List
    //////////////////////////////////////////////////////////////////////////

    std::cout << std::endl;

    noc->p_clk					        (signal_clk);
    noc->p_resetn				        (signal_resetn);
    noc->p_to_initiator[0]			    (signal_vci_l2g[0]);
    noc->p_to_initiator[1]			    (signal_vci_l2g[1]);
    noc->p_to_initiator[2]			    (signal_vci_l2g[2]);
    noc->p_to_initiator[3]			    (signal_vci_l2g[3]);
    noc->p_to_target[0]				    (signal_vci_g2l[0]);
    noc->p_to_target[1]				    (signal_vci_g2l[1]);
    noc->p_to_target[2]				    (signal_vci_g2l[2]);
    noc->p_to_target[3]				    (signal_vci_g2l[3]);

    std::cout << " - noc connected" << std::endl;

    xbar[0]->p_clk				        (signal_clk);
    xbar[0]->p_resetn			        (signal_resetn);
    xbar[0]->p_initiator_to_up		    (signal_vci_l2g[0]);
    xbar[0]->p_target_to_up			    (signal_vci_g2l[0]);
    xbar[0]->p_to_initiator[SRCID_PROC] (signal_vci_proc[0]);
    xbar[0]->p_to_initiator[SRCID_IOC]  (signal_vci_ini_ioc);
    xbar[0]->p_to_target[TGTID_RAM]     (signal_vci_ram[0]);
    xbar[0]->p_to_target[TGTID_IOC]     (signal_vci_tgt_ioc);

    std::cout << " - xbar[0] connected" << std::endl;

    xbar[1]->p_clk				        (signal_clk);
    xbar[1]->p_resetn			        (signal_resetn);
    xbar[1]->p_initiator_to_up		    (signal_vci_l2g[1]);
    xbar[1]->p_target_to_up			    (signal_vci_g2l[1]);
    xbar[1]->p_to_initiator[SRCID_PROC] (signal_vci_proc[1]);
    xbar[1]->p_to_initiator[SRCID_DMA]  (signal_vci_ini_dma);
    xbar[1]->p_to_target[TGTID_RAM]     (signal_vci_ram[1]);
    xbar[1]->p_to_target[TGTID_DMA]     (signal_vci_tgt_dma);
    xbar[1]->p_to_target[TGTID_ICU]     (signal_vci_tgt_icu);

    std::cout << " - xbar[1] connected" << std::endl;

    xbar[2]->p_clk				        (signal_clk);
    xbar[2]->p_resetn			        (signal_resetn);
    xbar[2]->p_initiator_to_up		    (signal_vci_l2g[2]);
    xbar[2]->p_target_to_up			    (signal_vci_g2l[2]);
    xbar[2]->p_to_initiator[SRCID_PROC] (signal_vci_proc[2]);
    xbar[2]->p_to_target[TGTID_RAM]     (signal_vci_ram[2]);
    xbar[2]->p_to_target[TGTID_FBF]     (signal_vci_tgt_fbf);
    xbar[2]->p_to_target[TGTID_TIM]	    (signal_vci_tgt_tim);

    std::cout << " - xbar[2] connected" << std::endl;

    xbar[3]->p_clk				        (signal_clk);
    xbar[3]->p_resetn			        (signal_resetn);
    xbar[3]->p_initiator_to_up		    (signal_vci_l2g[3]);
    xbar[3]->p_target_to_up			    (signal_vci_g2l[3]);
    xbar[3]->p_to_initiator[SRCID_PROC] (signal_vci_proc[3]);
    xbar[3]->p_to_target[TGTID_RAM]     (signal_vci_ram[3]);
    xbar[3]->p_to_target[TGTID_ROM]     (signal_vci_tgt_rom);
    xbar[3]->p_to_target[TGTID_TTY]	    (signal_vci_tgt_tty);

    std::cout << " - xbar[3] connected" << std::endl;

    for ( size_t i=0 ; i<4 ; i++ )
    {
        proc[i]->p_clk                  (signal_clk);
        proc[i]->p_resetn               (signal_resetn);
        proc[i]->p_vci                  (signal_vci_proc[i]);
        proc[i]->p_irq[0]               (signal_irq_proc[i]);
        proc[i]->p_irq[1]               (signal_false);
        proc[i]->p_irq[2]               (signal_false);
        proc[i]->p_irq[3]               (signal_false);
        proc[i]->p_irq[4]               (signal_false);
        proc[i]->p_irq[5]               (signal_false);

        ram[i]->p_clk                   (signal_clk);
        ram[i]->p_resetn                (signal_resetn);
        ram[i]->p_vci                   (signal_vci_ram[i]);
    }

    std::cout << " - procs & rams connected" << std::endl;

    tty->p_clk                          (signal_clk);
    tty->p_resetn                       (signal_resetn);
    tty->p_vci                          (signal_vci_tgt_tty);
    tty->p_irq[0]                       (signal_irq_tty[0]);
    tty->p_irq[1]                	    (signal_irq_tty[1]);
    tty->p_irq[2]                	    (signal_irq_tty[2]);
    tty->p_irq[3]                	    (signal_irq_tty[3]);

    std::cout << " - tty connected" << std::endl;

    tim->p_clk                 	        (signal_clk);
    tim->p_resetn              	        (signal_resetn);
    tim->p_vci                          (signal_vci_tgt_tim);
    tim->p_irq[0]              	        TO BE COMPLETED        
    tim->p_irq[1]              	        TO BE COMPLETED        
    tim->p_irq[2]              	        TO BE COMPLETED        
    tim->p_irq[3]              	        TO BE COMPLETED        

    std::cout << " - tim connected" << std::endl;

    icu->p_clk                   	    (signal_clk);
    icu->p_resetn                	    (signal_resetn);
    icu->p_vci                   	    (signal_vci_tgt_icu);
    icu->p_irq_out[0]                   TO BE COMPLETED        
    icu->p_irq_out[1]                   TO BE COMPLETED        
    icu->p_irq_out[2]                   TO BE COMPLETED        
    icu->p_irq_out[3]                   TO BE COMPLETED        
    icu->p_irq_in[0]             	    TO BE COMPLETED        
    icu->p_irq_in[1]             	    TO BE COMPLETED        
    icu->p_irq_in[2]             	    TO BE COMPLETED        
    icu->p_irq_in[3]             	    TO BE COMPLETED        
    icu->p_irq_in[4]             	    TO BE COMPLETED        
    icu->p_irq_in[5]             	    TO BE COMPLETED        
    icu->p_irq_in[6]             	    TO BE COMPLETED        
    icu->p_irq_in[7]             	    TO BE COMPLETED        
    icu->p_irq_in[8]             	    TO BE COMPLETED        
    icu->p_irq_in[9]             	    TO BE COMPLETED        
    icu->p_irq_in[10]             	    TO BE COMPLETED      
    icu->p_irq_in[11]             	    TO BE COMPLETED      
    icu->p_irq_in[12]             	    TO BE COMPLETED      
    
    std::cout << " - icu connected" << std::endl;

    rom->p_clk                          (signal_clk);
    rom->p_resetn                       (signal_resetn);
    rom->p_vci                          (signal_vci_tgt_rom);

    std::cout << " - rom connected" << std::endl;

    fbf->p_clk                          (signal_clk);
    fbf->p_resetn                       (signal_resetn);
    fbf->p_vci                          (signal_vci_tgt_fbf);

    std::cout << " - fbf connected" << std::endl;

    ioc->p_clk                          (signal_clk);
    ioc->p_resetn                       (signal_resetn);
    ioc->p_vci_initiator                (signal_vci_ini_ioc);
    ioc->p_vci_target                   (signal_vci_tgt_ioc);
    ioc->p_irq                          TO BE COMPLETED      

    std::cout << " - ioc connected" << std::endl;

    dma->p_clk                          (signal_clk);
    dma->p_resetn                       (signal_resetn);
    dma->p_vci_initiator                (signal_vci_ini_dma);
    dma->p_vci_target                   (signal_vci_tgt_dma);
    dma->p_irq[0]                       TO BE COMPLETED      
    dma->p_irq[1]                       TO BE COMPLETED      
    dma->p_irq[2]                       TO BE COMPLETED      
    dma->p_irq[3]                       TO BE COMPLETED      

    std::cout << " - dma connected" << std::endl;

    //////////////////////////////////////////////////////////////////////////
    // simulation
    //////////////////////////////////////////////////////////////////////////

    signal_false = false;
    signal_resetn = false;
    sc_start( sc_time( 1, SC_NS ) );
    signal_resetn = true;
    for ( int n=1 ; n<ncycles ; n++ )
    {
        if( debug && (n > from_cycle) )
        {
            std::cout << "***************** cycle " << std::dec << n << std::endl;

//            proc[0]->print_trace();
//            signal_vci_proc[0].print_trace("[SIG] PROC_0");

//            proc[1]->print_trace();
//            signal_vci_proc[1].print_trace("[SIG] PROC_1");

            proc[2]->print_trace();
            signal_vci_proc[2].print_trace("[SIG] PROC_2 ");

//            proc[3]->print_trace();
//            signal_vci_proc[3].print_trace("[SIG] PROC_3");

//            icu->print_trace();
//            signal_vci_tgt_icu.print_trace("[SIG] ICU ");

            dma->print_trace();
            signal_vci_tgt_dma.print_trace("[SIG] DMA ");
            signal_vci_ini_dma.print_trace("[SIG] DMA ");

            if( signal_irq_dma[0].read() )  std::cout << " - IRQ_DMA0" << std::endl;
            if( signal_irq_dma[1].read() )  std::cout << " - IRQ_DMA1" << std::endl;
            if( signal_irq_dma[2].read() )  std::cout << " - IRQ_DMA2" << std::endl;
            if( signal_irq_dma[3].read() )  std::cout << " - IRQ_DMA3" << std::endl;

            if( signal_irq_proc[0].read() ) std::cout << " - IRQ_PROC0" << std::endl;
            if( signal_irq_proc[1].read() ) std::cout << " - IRQ_PROC1" << std::endl;
            if( signal_irq_proc[2].read() ) std::cout << " - IRQ_PROC2" << std::endl;
            if( signal_irq_proc[3].read() ) std::cout << " - IRQ_PROC3" << std::endl;
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

