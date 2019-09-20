/* -*- c++ -*-
 *
 * SOCLIB_LGPL_HEADER_BEGIN
 * 
 * This file is part of SoCLib, GNU LGPLv2.1.
 * 
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 * 
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARr_opaNTY; without even the implied warranty of
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
 * Copyright (c) UPMC, Lip6
 *         Alain Greiner <alain.greiner@lip6.fr>, 2009
 *
 * Maintainers: alain
 */

#include "fifo_gcd_coprocessor.h"

using namespace sc_core;

namespace soclib {
namespace caba {

////////////////////////////////////////////////////////////////
FifoGcdCoprocessor::FifoGcdCoprocessor( sc_module_name insname )
	: sc_module(insname),
      	r_fsm("r_fsm"),
      	r_opa("r_opa"),
      	r_opb("r_opb"),
        p_resetn("resetn"),
        p_clk("clk"),
		p_in("p_in"),
		p_out("p_out")
{
	SC_METHOD(transition);
	dont_initialize();
	sensitive << p_clk.pos();
	
	SC_METHOD(genMoore);
	dont_initialize();
	sensitive << p_clk.neg();
}

FifoGcdCoprocessor::~FifoGcdCoprocessor( )
{
}

/////////////////////////////////////
void FifoGcdCoprocessor::transition()
{
	if ( !p_resetn.read() ) {
		r_fsm = A COMPLETER
		return;
	}

	switch ( r_fsm.read() ) {
	case READ_OPA :
		if ( p_in.rok.read() ) {
			r_opa = A COMPETER
			r_fsm = A COMPLETER
		}
		break;
	case READ_OPB :
		if ( p_in.rok.read() ) {
			r_opb = A COMPLETER
			r_fsm = A COMPLETER
		}
		break;
	case COMPARE:
		if      ( r_opa.read() < r_opb.read() )  r_fsm = A COMPLETER
		else if ( r_opa.read() > r_opb.read() )  r_fsm = A COMPLETER
		else                                     r_fsm = A COMPLETER
		break;
	case DECR_A :
	    r_opa = A COMPLETER
	    r_fsm = A COMPLETER
	break;
	case DECR_B :
	    r_opb = A COMPLETER
	    r_fsm = A COMPLETER
	break;
	case WRITE_RES :
		if ( p_out.wok.read() ) {
			r_fsm = A COMPLETER
		}
		break;
        } // end switch
} // end transition()

///////////////////////////////////
void FifoGcdCoprocessor::genMoore()
{
	switch ( r_fsm.read() ) {
	case READ_OPA : 
        case READ_OPB :
		p_in.r 		= A COMPLETER
		p_out.w		= A COMPLETER
                p_out.data	= A COMPLETER
	break; 
	case COMPARE :
	case DECR_A :
	case DECR_B :
		p_in.r 		= A COMPLETER
		p_out.w		= A COMPLETER
                p_out.data	= A COMPLETER
	break; 
	case WRITE_RES :
		p_in.r 		= A COMPLETER
		p_out.w		= A COMPLETER
                p_out.data	= A COMPLETER
	break; 
     } // end switch
} // end genMoore()

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

