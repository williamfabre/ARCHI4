# CR TP7_ARCHI4



## Quelle est la taille de l'espace d'adressage physique et quelle est la taille de l'espace virtuelle ?
L'architecture TSAR utilise des processeurs 32bits pour minimiser la consommation énergétique. Chaque application possède donc un espace virtuel limité à 4 Goctets (adresses virtuelles sur 32 bits).

## Quelles sont les tailles des tables de 1e et et 2e niveau d'une table de page ?

| type of page inside | name  | layer  | size  | nb bits  | taille (en nombre d'entrees) |
|---|---|---|---|---|---|
| 2M bytes page descriptor | (called PTE1)  | 1st level  | 2MO=2^21  | PPN1 (19 bits)  | 2^19 pages possibles |
| 4K bytes page descriptor | (called PTD1)  | 1st level  | 4KO=2^12  | PPN2 (28 bits)  | 2^28 pages possibles |

| 4K bytes page descriptor | (called PTE1)  | 2nd level  | 4KO=2^12  | soft (8 bits)   | 512 pages possibles  |
| 4K bytes page descriptor | (called PTD1)  | 2nd level  | 4KO=2^12  | PPN2 (19 bits)  | 2^19 pages possibles |

## A quoi servent les bits V et D dans les tables de 1e et de 2e niveau de la table des pages ?
* V = "One additional bit is generally attached to each entry in the page table:
 a valid–invalid bit. When this bit is set to valid, the associated page is in
 the process’s logical address space and is thus a legal (or valid) page.
 When the bit is set to invalid, the page is not in the process’s logical address space."

 The valid bits of the TLB should all be set to 0 when context switch. The page table entries in the TLB
 corresponded to the old page table, so none of them are valid once the page table address register points to a different page table.

* D = The page has been modified when 1 (set by the hardware)


## Quelle est le nombre d'entrée de chaque TLB ?
 Each MMU contains a 64 entries TLB (Translation Look-aside Buffer). These TLBs are implemented as
 set-associative caches (16 sets of 4 ways). Each entry in these TLBs can contain either a 4 Kbytes
 page descriptor, or a 2 Mbytes page descriptor. The figure below illustrate the general structure of the TSAR L1 caches.


## Description importantes de DHCCP
* DHCCP :  protocol (Distributed Hybrid Cache Coherence Protocol)

* This Global Directory stores the status of each cache line replicated in
  at least one L1 cache of the TSAR architecture. 

* NO exclusive ownership state for a cache line. 

* The L2 cache is inclusive for all L1 caches.

* MULTICAST_UPDATE : When the number of copies is smaller than the DHCCP
 threshold, the L2 cache controller registers the locations of all the copies,
 and can send a dedicated update(L) request to each relevant L1 cache in case of modification of L.
	
* BROADCAST_INVAL : When the number of copies is larger than the DHCCP threshold,
 the memory cache controller registers only the number of copies (without localization)
 and broadcast an inval request to all L1 caches in case of modification of L. 

 * VCI TRDID and PKTID values : issue several simultaneous direct transactions, that are distinguished by the . 

* READ : transaction can have four sub-types: It can be instruction or data,
 and it can be cacheable or uncacheable. In case of a burst transaction the burst
 must be included in a 16 words cache line. This constraint applies for both
 the L1 cache controllers and the I/O controllers with a DMA capability.
 For all READ transaction, the VCI command packet contains one single VCI flit,
 and the VCI response packet contains at most 16 flits. 

* WRITE transaction can be a single word request or a variable length burst request.
In case of burst, all words must belong to the same cache line. and the BE
field can have different values for each flit (including the zero value).
The VCI command packet contains at most 16 flits and the VCI response packet
contains one VCI flit. A WRITE burst transaction initiated by a DMA
controller must respect the same constraint. 

* CAS (Compare & Swap) transaction can target any single word contained in
a memory cache. The command must transport both the old data value and the
new data value. The response returns only a Boolean indicating failure/success
for the CAS transaction. This means that the VCI command packet contains
two flits and the VCI response packet contains one flit. 


* A CLEANUP transaction is initiated by the L1 cache when it must evict a line L
for replacement, to signal to the owner L2 cache that it does not contains
anymore a copy of L. This transaction requires two packet types:
The L1 cache send a cleanup(L) packet to the owner L2 cache.
The L2 cache returns a clack(L) packet to signal that its list of copies for L has been updated. 

* A MULTI_UPDATE transaction is a multi-cast transaction initiated by the L2 cache
when it receives a WRITE request to a replicated cache line, and the number
of copies does not exceeds the DHCCP threshold. This transaction requires two packet types:
The L2 send as many update(L,DATA) packets as the number of registered copies (but the writer).
Each L1 cache returns an update_ack(L) packet to the L2 cache to signal that the local copy has been updated. 


* A MULTI_INVAL transaction is a multi-cast transaction, initiated by the L2
cache, when it must evict a given line L, and the number of copies does
not exceeds the DHCCP threshold. To keep the inclusion property, all copies
in L1 caches must be invalidated. This transaction requires three types of packets:
The L2 cache send as many inval(L) packets as the number of registered copies to all registered L1 caches.
Each L1 cache send a cleanup(L) packet to the L2 cache to signal that the local copy has been invalidated.
The L2 cache returns to each L1 cache a clack(L) packet to signal that its list of copies for L has been updated. 

* A BROADCAST_INVAL transaction is a broadcast transaction initiated by a L2
cache when a line L has been modified by a WRITE, or when the line L must be
evicted for replacement, and the number of copies exceeds the DHCCP threshold.
This transaction request three types of packets:
The L2 cache send to all L1 caches controller a bc_inval(L) broadcast packet.
Each L1 cache that contains a copy of L send a cleanup(L) packet to the L2 cache to signal that the local copy has been invalidated.
The L2 cache returns to each L1 cache that made a cleanup, a clack(L) packet to signal that its list of copies for L has been updated. 

## Que signifie LL/SC ? A quoi servent ces instructions ?
 * LL (Linked Load) transaction can target any single word contained in a
 memory cache. The response returns two 32 bits values that are the addressed
 data value, and a signature that has been allocated by the memory cache
 to this LL reservation. This means that the VCI command packet contains
 one flit and the VCI response packet contains two flits. 

* SC (Store Conditionnal) transaction can target any single word contained in
a memory cache. The command must transport both the new data value and the
signature obtained after the LL transaction. The response returns only a
Boolean indicating failure/success for the SC transaction. This means that the
VCI command packet contains two flits and the VCI response packet contains one flit. 

## Combien de réseaux DSPINs indépendants interconnectent les clusters ? Pourquoi ?
 The TSAR architecture uses the DSPIN network on chip infrastructure to define three independent networks.

* The Direct Network implements the 40 bits TSAR physical address space supporting
software driven transactions. It transports the direct READ, WRITE, LL, SC
and CAS transactions from a VCI initiator (typically a L1 cache controller)
to a VCI target (typically a memory cache controller, or a memory mapped peripheral).
All VCI packets are translated to DSPIN packets by specific VCI/DSPIN wrappers.
There is two physically separated networks for command packets and response packets.
Both networks have a two-level hierarchical structure with a local interconnect
in each cluster (that can be implemented as a local crossbar, or as a local ring),
and a global interconnect (implemented as a 2D mesh). 

* The Coherence Network is used to transport the coherence packets implementing
the DHCCP coherence protocol between L2 cache controllers and L1 cache controllers.
This network is not visible by the software, and does not use wrappers,
as the L1 and L2 cache controllers use directly the DSPIN packet format. Here
again there is three physically separated networks to transport L2-to-L1 packets
(M2P network), to transport L1-to-L2 packets (P2M network) and to transport
CLACK packets (CLACK network). These networks have a two-level hierarchical
structure with a local interconnect in each cluster (that can be implemented
as a local crossbar, or as a local ring), and a global interconnect (implemented as a 2D mesh). 

* The RAM Network supports communications between the L2 caches and the
tiles implementing the L3 cache, in case of MISS or cache line
replacement in the L2 caches. It supports also the direct communication
between the external peripheral that have a DMA capability
(Disk controllers, or network controllers) and the L3 cache.
It has a 3D mesh topology and the DSPIN flit width is 64 bits. 

## Dans quel fichier doit-on indiquer le nombre de core et le nombre de cluster ?
le fichier /arch_info.bin situé dans le répertoire racine. Ce fichier définit
les caractéristiques de l'architecture matérielle: nombre de clusters,
nombre de coeurs dans chaque cluster, taille de la mémoire physique dans
chaque cluster, nombre et types des périphérique.

## Quelle est la différence entre le bootloader et le preloader ?
 Le preloader charge le bootloader. le bootloader charge l'OS.

## Dans quel cluster se trouve le noyau ?
Dans tous les clusters :
*  Pour permettre a chaque instance du noyau de gérer ses propres structures de données
propres, les deux segments seg_kcode et seg_kdata constituant le code kernel sont répliqués dans chaque cluster.
* ce code kernel est dans la mémoire locale de chaque cluster[i]. 

## A quoi sert le fichier ldscript utilisé pour la production d'un exécutable ?


## Pourquoi toutes les applications user ont le même ldscript ?


## Pourquoi doit-on modifier l'image disque utilisé par TSAR pour ajouter une application ?  


