#include <systemc>
#include <limits>

#include "vci_gcd_master.h"
#include "vci_gcd_coprocessor.h"
#include "vci_signals.h"
#include "vci_param.h"
#include "mapping_table.h"

#define GCD_BASE	TO BE COMPLETED...
#define GCD_SIZE	TO BE COMPLETED...
	
int sc_main(int argc, char *argv[])
{
        using namespace sc_core;
	using namespace soclib::caba;
	using namespace soclib::common;

	// VCI fields width definition
	//	cell_size	= 4;
	// 	plen_size	= 8;
	// 	addr_size	= 32;
	// 	rerror_size	= 1;
	// 	clen_size	= 1;
	// 	rflag_size	= 1;
	// 	srcid_size	= 12;
	// 	trdid_size	= 1;
	// 	pktid_size	= 1;
	// 	wrplen_size	= 1;

	typedef VciParams< TO BE COMPLETED > vci_param;

	///////////////////////////////////////////////////////////////////////////
	// simulation arguments : number of cycles & seed for the random generation
	///////////////////////////////////////////////////////////////////////////
        int ncycles = std::numeric_limits<int>::max();
        int seed    = 123456789;
        if (argc > 1) ncycles = atoi(argv[1]) ;
        if (argc > 2) seed = atoi(argv[2]) ;

	//////////////////////////////////////////////////////////////////////////
	// Mapping Table
	//////////////////////////////////////////////////////////////////////////
	MappingTable maptab(32, IntTab(8), IntTab(8), 0x03000000);
	maptab.add(soclib::common::Segment("GCD", GCD_BASE, GCD_SIZE, IntTab(0), true));
	std::cout << std::endl << maptab << std::endl;

	//////////////////////////////////////////////////////////////////////////
        // Signals
	//////////////////////////////////////////////////////////////////////////
        sc_clock               		signal_clk("signal_clk", sc_time( 1, SC_NS ), 0.5 );
        sc_signal<bool> 		signal_resetn("signal_resetn");
        VciSignals<vci_param> 		signal_vci("signal_vci");

	//////////////////////////////////////////////////////////////////////////
	// Components
	//////////////////////////////////////////////////////////////////////////
        VciGcdMaster<vci_param> 	master(TO BE COMPLETED);
	VciGcdCoprocessor<vci_param>	coproc(TO BE COMPLETED);

	//////////////////////////////////////////////////////////////////////////
	// Net-List
	//////////////////////////////////////////////////////////////////////////
	master.p_clk(signal_clk);
	master.p_resetn(signal_resetn);
	master.p_vci(signal_vci);

	coproc.p_clk(signal_clk);
	coproc.p_resetn(signal_resetn);
	coproc.p_vci(signal_vci);

	//////////////////////////////////////////////////////////////////////////
	// simulation
	//////////////////////////////////////////////////////////////////////////
        signal_resetn = false;
        sc_start( sc_time( 1, SC_NS ) ) ;

        signal_resetn = true;
        for ( size_t n=1 ; n<ncycles ; n++) sc_start( sc_time( 1, SC_NS ) ) ;

        return(0);

} // end sc_main

