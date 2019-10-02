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
 * Copyright (c) UPMC, Lip6
 *         Alain Greiner <alain.greiner@lip6.fr>, 2009
 *
 * Maintainers: alain
 */


#include <iostream>

#include "gcd.h"
#include "vci_gcd_coprocessor.h"
#include "assert.h"

using namespace sc_core;
using namespace soclib::caba;

namespace soclib {
namespace caba {

template<typename vci_param> 
VciGcdCoprocessor<vci_param>::VciGcdCoprocessor(sc_module_name insname,
                                                const soclib::common::IntTab &index,
                                                const soclib::common::MappingTable &mt)
: sc_module(insname),
    r_vci_fsm("r_vci_fsm"),
    r_exe_fsm("r_exe_fsm"),
    r_srcid("r_srcid"),
    r_trdid("r_trdid"),
    r_pktid("r_pktid"),
    r_opa("r_opa"),
    r_opb("r_opb"),
    m_segment(mt.getSegment(index)),
    p_resetn("resetn"),
    p_clk("clk"),
    p_vci("vci")
    {
        SC_METHOD(transition);
        dont_initialize();
        sensitive << p_clk.pos(); // sensibilite sur front montant

        SC_METHOD(genMoore);
        dont_initialize();
        sensitive << p_clk.neg(); // sensibilite sur front descendant
    }

////////////////////////////
template<typename vci_param>
VciGcdCoprocessor<vci_param>::~VciGcdCoprocessor()
{
}

////////////////////////////
template<typename vci_param>
void VciGcdCoprocessor<vci_param>::transition()
{
    if ( !p_resetn.read() )
    {
        r_vci_fsm = VCI_GET_CMD;
        r_exe_fsm = EXE_IDLE;
        return;
    }

#ifdef SOCLIB_MODULE_DEBUG
    std::cout << name() << "  vci_fsm = " << r_vci_fsm.read() << std::endl;
    std::cout << name() << "  exe_fsm = " << r_exe_fsm.read() << std::endl;
    std::cout << name() << "  opa_reg = " << r_opa.read() << std::endl;
    std::cout << name() << "  opb_reg = " << r_opb.read() << std::endl;
#endif
    /////////////////////////////
    switch ( r_exe_fsm.read() ) {
    case EXE_IDLE:
        if( r_vci_fsm.read() == VCI_RSP_START ) r_exe_fsm = EXE_COMPARE;
            break;
    case EXE_COMPARE:
        if      ( r_opa.read() < r_opb.read() )	r_exe_fsm = EXE_DECB; // a< b
        else if ( r_opb.read() < r_opa.read() )	r_exe_fsm = EXE_DECA; // a> b
        else r_exe_fsm = EXE_IDLE; // case equal
            break;
    case EXE_DECA:
        // if( r_opa > r_opb ) opa = opa - opb
            r_opa     = r_opa.read() - r_opb.read();
            r_exe_fsm = EXE_COMPARE;
            break;
    case EXE_DECB:
            r_opb     = r_opb.read() - r_opa.read();
            r_exe_fsm = EXE_COMPARE;
            break;
    } // end switch exe-fsm

    /////////////////////////////
    switch ( r_vci_fsm.read() ) {
    case VCI_GET_CMD:
        if ( p_vci.cmdval.read() ) {
            //  adress : expl : cell == GCD_OPA   c'est l'etat
            typename vci_param::addr_t address = p_vci.address.read();
            // une cell c'est quoi?
            uint32_t cell  = (address - m_segment.baseAddress()) / vci_param::B;
            // only accepts single word requests & checks for segmentation violations
            assert ( ( p_vci.eop.read() ) &&
                     ( p_vci.plen.read() == 4 ) &&
                     ( p_vci.cmd.read() != vci_param::CMD_LOCKED_READ ) &&
                     ( p_vci.cmd.read() != vci_param::CMD_STORE_COND ) &&
                     ( m_segment.contains(address) ) &&
                     "illegal command received by the GCD coprocessor");
            // store the VCI command in registers
            r_srcid	= p_vci.srcid.read(); // numero de l'initiateur defini par S
            r_trdid	= p_vci.trdid.read(); // etiqueter une commande vci par un numero de thread ou de transaction efini par T
            // etiqueter une commande vci....
            //peut etre utiliser pour un initiateur pour envoyer la commande n+1 sans attendre la reponse a la commande n
            r_pktid	= p_vci.pktid.read();

            // test the command
            if ( ( p_vci.cmd.read() == vci_param::CMD_READ )
                 && ( cell == GCD_OPA ) ) {
                    r_vci_fsm = VCI_RSP_RESULT;

            } else if ( ( p_vci.cmd.read() == vci_param::CMD_READ )
                        && ( cell == GCD_STATUS ) ) {
                    // passage adns l'etat status, renvoie 0 si idle et != 0 si
                    // pas fini le calcule
                    r_vci_fsm = VCI_RSP_STATUS;

            } else if ( ( p_vci.cmd.read() == vci_param::CMD_WRITE )
                        && ( cell == GCD_OPA ) ) {
                    // recuperation de l'oprande dans wdata
                    r_opa     = p_vci.wdata.read();
                    r_vci_fsm = VCI_RSP_OPA;

            } else if ( ( p_vci.cmd.read() == vci_param::CMD_WRITE )
                        && ( cell == GCD_OPB ) ) {
                    // recuperation de l'oprande dans wdata
                    r_opb     = p_vci.wdata.read();
                    r_vci_fsm = VCI_RSP_OPB;

            } else if ( ( p_vci.cmd.read() == vci_param::CMD_WRITE )
                        && ( cell == GCD_START ) ) {
                    r_vci_fsm = VCI_RSP_RESULT; // calcule du pgcd avec les commandes
                    // sur opa et opb

            } else {
                std::cout << "illegal command to the GCD coprocessor" << std::endl;
                exit(0);
            }
        }
        break;
    case VCI_RSP_OPA:
    case VCI_RSP_OPB:
    case VCI_RSP_START:
    case VCI_RSP_STATUS:
    case VCI_RSP_RESULT:
        if ( p_vci.rspack.read())	r_vci_fsm = VCI_GET_CMD; // etat de base
            break;
    } // end switch vci_fsm
} // end transition()

////////////////////////////////////////
template<typename vci_param>
void VciGcdCoprocessor<vci_param>::genMoore()
{
    /* les parametres de la classe :
     * r_vci_fsm("r_vci_fsm"), -> etat de la fsm VCI
    r_exe_fsm("r_exe_fsm"), -> etat de la fsm EXE
    r_srcid("r_srcid"), -> aiguiller le paquet REPONSE vers INIT
    r_trdid("r_trdid"), -> etiquette de paquet CMD (num thread ou num transac)
    r_pktid("r_pktid"), -> etiquette de paquet CMD (num thread ou num transac)
    r_opa("r_opa"), -> TODO
    r_opb("r_opb"),-> TODO
    m_segment(mt.getSegment(index)),-> TODO
    p_resetn("resetn"),-> TODO
    p_clk("clk"),-> TODO
    p_vci("vci")-> TODO
    */

    // sorties ind�pendantes de l'�tat de l'automate de MOORE donc on recopie
    // juste les registres (ne dpd pas de l'entree)
    p_vci.rsrcid = r_srcid;
    p_vci.rtrdid = r_trdid;
    p_vci.rpktid = r_pktid; // Comportement specialise, pas besoin de threads
    p_vci.rerror = 0;
    p_vci.reop   = true; // end of packet

    switch (r_vci_fsm) {
    case VCI_GET_CMD:
        // je veux une commande
        p_vci.cmdack = true; // fifo non vide = true
        // j'ai pas de reponse
        p_vci.rspval = false; // fifo non pleine = flase
        p_vci.rdata  = 0;
        break;
    case VCI_RSP_OPA:
    case VCI_RSP_OPB:
    case VCI_RSP_START:
        // je veux pas de commande
        p_vci.cmdack = false;
        //j'ai une reponse
        p_vci.rspval = true;
        p_vci.rdata  = 0;
        break;
    case VCI_RSP_STATUS:
        p_vci.cmdack = false;
        p_vci.rspval = true;
        p_vci.rdata  = r_exe_fsm.read();// valeur si j'ai fini ou pas le status c'est l'etat
        // r_exe_fsm?
        // r_vci_fsm = VCI_RSP_STATUS; nan ici on traite l'automate exe donc
        break;
    case VCI_RSP_RESULT:
        // je veux pas une commande
        p_vci.cmdack = false;
        // j'ai une reponse
        p_vci.rspval = true;
        // blabla, return ( opa ); pour le resultat
        p_vci.rdata  = r_opa;
    }
} // end genMoore()

//template class VciGcdCoprocessor<soclib::caba::VciParams<>>;
template class VciGcdCoprocessor<soclib::caba::VciParams<4, 8, 32, 1, 1, 1, 12, 1, 1, 1>>;

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

