# CR TP8_ARCHI4



## Questions sur le preloader

### À quoi sert un preloader ?
- Son rôle est de charger en mémoire RAM le code du bootloader spécifique au système d'exploitation qu'on veut charger.

### Où se trouve stocké le code du preloader au démarrage de la plateforme TSAR? Comment les coeurs accèdent_ils au code du preloader ?

- Lieux exacte du stockage dans le disque (par convention) :
" the system specific boot-loader stored on disk at RESET_LOADER_LBA" (Dont la valeur est 2, je suppose que c'est le numero de secteur du disque.

``` asm

    /*
     * Bootstrap Processor jumps to the reset_elf_loader routine passing as argument
     * the block number in which is loaded the .elf file
     */

    li      a0,     RESET_LOADER_LBA
    jal     reset_elf_loader
    nop
	```



- This preloader uses a stack segment allocated in cluster 0 for processor 0.
- The stack allocation is not performed for other processors as they do not
need it during the preloader execution. Therefore, this allocation should be
done by the loaded Operating System.
(tsar/softs/tsar_boot/src/reset.S)



### Comment fait-on pour qu'un code ne soit exécuté que par un seul core ?
- Only processor 0 executes the reset_load_elf function to load into memory
  the system specific boot-loader stored on disk at RESET_LOADER_LBA
- All other processors wait in a low power consumption mode that the
  processor 0 wakes them using an IPI (Inter Processor Interruption)

- Fonction de reveil de core 0,0:

``` c

/*********************************************************************************
 * This function is called by CP0 in cluster(0,0) to activate all other CP0s.
 * It returns the number of CP0s actually activated.
 ********************************************************************************/
static uint32_t boot_wake_all_cp0s( void )
{
    archinfo_header_t*  header;         // Pointer on ARCHINFO header
    archinfo_cluster_t* cluster_base;   // Pointer on ARCHINFO clusters base
    archinfo_cluster_t* cluster;        // Iterator for loop on clusters
    archinfo_device_t*  device_base;    // Pointer on ARCHINFO devices base
    archinfo_device_t*  device;         // Iterator for loop on devices
    uint32_t            cp0_nb = 0;     // CP0s counter

    header       = (archinfo_header_t*)ARCHINFO_BASE;
    cluster_base = archinfo_get_cluster_base(header);
    device_base  = archinfo_get_device_base (header); 

    // loop on all clusters 
    for (cluster = cluster_base;
         cluster < &cluster_base[header->x_size * header->y_size];
         cluster++)
    {
        // Skip boot cluster.
        if (cluster->cxy == BOOT_CORE_CXY)
            continue;
            
        // Skip clusters without core (thus without CP0).
        if (cluster->cores == 0)
            continue;

        // Skip clusters without device (thus without XICU).
        if (cluster->devices == 0)
            continue;

        // search XICU device associated to CP0, and send a WTI to activate it 
        for (device = &device_base[cluster->device_offset];
             device < &device_base[cluster->device_offset + cluster->devices];
             device++)
        {
            if (device->type == DEV_TYPE_ICU_XCU)
            {

#if DEBUG_BOOT_WAKUP
boot_printf("\n[BOOT] core[%x,0] activated at cycle %d\n",
cluster->cxy , boot_get_proctime );
#endif

                boot_remote_sw((xptr_t)device->base, (uint32_t)boot_entry);
                cp0_nb++;
            }
        }
    }
    return cp0_nb;

} // boot_wake_cp0()

```

### A quel moment les cores sortent-ils du preloader ?
- All other processors, when exiting wait mode, read from XICU the address
to jump.
- This address is the boot-loader entry address that has been written in
the mailbox by the IPI sent by processor 0





## Questions sur le bootloader (/almos-mkh/boot/tsar_mips32/boot.c)

### À quoi sert le bootloader ?
- La première tâche du bootloader est de charger en mémoire le code du
système d'exploitation, initialement stocké sur le disque externe.

- La seconde tâche du bootloader est de charger en mémoire, à partie du disque externe de la machine, le fichier /arch_info.bin situé dans le répertoire racine. Ce fichier définit les caractéristiques de l'architecture matérielle

### Quels périphériques doit il pouvoir accéder ?
- The 'boot.elf' file (containing the boot-loader binary code) is stored 
on disk (not in the FAT file system), and must be loaded into memory by 
the preloader running on the core[0][0] (cxy = 0 / lid = 0).

### Quels formats de fichier doit-il pouvoir analyser ?
- The "kernel.elf" and "arch_info.bin" files are supposed to be
stored on disk in a FAT32 file system.


### Dans quel(s) fichier(s) est rangée la description de la plateforme matérielle ? 
- La seconde tâche du bootloader est de charger en mémoire, à partie du disque externe de la machine, le fichier /arch_info.bin situé dans le répertoire racine. Ce fichier définit les caractéristiques de l'architecture matérielle

- https://www-soc.lip6.fr/trac/almos-mkh/wiki/arch_info

### Pourquoi le code du bootloader est-il recopié dans tous les clusters ?
Pour charger le code les segments kdata et kcode dans chaque clusters, initialiser donc
1 OS par cluster (pour eviter la congestion d'acces a la memoire pour acceder au code
du noyau (+ typique des archis CC-NUMA)


### Pourquoi chaque core, a-t-il besoin d'une pile ? où sont-elles placées ?
- Pourquoi :
	Parce que chaque processeur va avoir ses propres fonctions a executer et l'acces
	a la pile doit etre "locale" a chaque processeur. (TODO reexpliquer)

- Placement (boot_entry.S) :
	``` asm
    /* Initialize stack pointer from lid value  */
    
    la      t0,     BOOT_STACK_BASE                     /* t0 <= BOOT_STACK_BASE            */
    li      k1,     BOOT_STACK_SIZE                     /* k1 <= BOOT_STACK_SIZE            */
    multu   k1,     t1
    mflo    k0                                          /* k0 <= BOOT_STACK_SIZE * lid      */
    subu    sp,     t0,     k0                          /* P[cxy,lid] sp initialized        */ 
	```



### Qu'est-ce qu'une IPI ? Comment sont-elles utilisées par le bootloader ? 
- L'accès au disque externe et le chargement du bootloader ne sont exécutés que par le coeur[0[0] (coeur 0 du cluster 0). Les autres coeurs de la plate-forme commencent à exécuter le code du preloader, mais se bloquent immédiatement en mode basse consommation, et ne seront réveillés que plus tard par une IPI (Inter Processor Interrupt). 

``` c

/*********************************************************************************
 * This function is called by all CP0s to activate the other CPi cores. 
 * @ boot_info  : pointer to local 'boot_info_t' structure.
 *********************************************************************************/
static void boot_wake_local_cores(boot_info_t * boot_info)
{
    unsigned int     core_id;        

    // get pointer on XCU device descriptor in boot_info
    boot_device_t *  xcu = &boot_info->int_dev[0];
 
    // loop on cores
    for (core_id = 1; core_id < boot_info->cores_nr; core_id++)
    {

#if DEBUG_BOOT_WAKUP
boot_printf("\n[BOOT] core[%x,%d] activated at cycle %d\n",
boot_info->cxy , core_id , boot_get_proctime() );
#endif
        // send an IPI 
        boot_remote_sw( (xptr_t)(xcu->base + (core_id << 2)) , (uint32_t)boot_entry ); 
    }
} // boot_wake_local_cores()

```





## Questions sur kernel_init

### Comment dans chaque cluster, le noyau a-t-il accès à la description de la plate-forme matérielle ?
- the boot_info structure is built by the bootloader, and used by kernel_init.
it must be the first object in the kdata segment.

### Comment almos_mkh identifie-t-il les coeurs de la plate-forme matérielle ? 
- This file contains the ALMOS-MKH. boot-loader for the TSAR architecture. 
that is a clusterised, shared memory, multi-processor architecture,      
where each processor core is identified by a composite index [cxy,lid]   
with one physical memory bank per cluster.                               

``` c

 core[0][0] (cxy = 0 / lid = 0)


 ```

### Les threads kernel IDLE exécutent, sur chaque coeur la fonction kernel_init(). Le thread IDLE est aussi le thread choisi par le scheduler d'un coeur quand tous les autres threads placés sur ce coeur sont bloqués. Où sont rangés les descripteurs de threads IDLE ? Combien y a-t-il de threads idle ?
- the array of idle threads descriptors must be placed on the first page boundary after
the boot_info structure in the kdata segment.

```c

// This variable defines the "idle" threads descriptors array
__attribute__((section(".kidle"))) char idle_threads[CONFIG_THREAD_DESC_SIZE x CONFIG_MAX_LOCAL_CORES]   CONFIG_PPM_PAGE_ALIGNED;

```


### Pour quelle raison la fonction kernel_init() utilise-t-elle des barrières de synchronisation ? Illustrez votre réponse avec un exemple.


### Pour quelle raison, utilise-t-on à la fois des barrières locales et des barrières globales ?


### Dans le cas d'une barrière globale, expliquez précisément la valeur du premier argument passé à la fonction xbarrier_wait(). L'API permettant l'accès à une barrière globale est définie dans les fichiers almos-mkh/kernel/libk/xbarrier.h et almos-mkh/kernel/libk/xbarrier.c.

volatile boot_remote_barrier_t  global_barrier;   // synchronize CP0 cores
volatile boot_remote_barrier_t  local_barrier;    // synchronize cores in one cluster


### Quel est le rôle de la fonction cluster_init() définie dans le fichier almos-mkh/kernel/kern/cluster.c ?


### Quelle est l'utilité de la structure de donnée distribuée DQDT ? L'API permettant l'accès à la DQDT est définie dans les fichiers almos-mkh/kernel/kern/dqdt.h et almos-mkh/kernel/kern/dqdt.c.

-  DQDT, qui permet d'enregistrer le taux d'utilisation de la mémoire dans chaque cluster, ou la charge de chacun des coeurs de la plateforme.

- " 4.3 DQDT (Distributed Quaternary Decision Tree)La décentralisation de la gestion des ressources, grâce à la mise en place d’un  cluster-manager par nœud cc-NUMA, a comme conséquence un manque de connaissance globaleconcernant la disponibilité des ressources. Cette connaissance est nécessaire au noyau pourpouvoir décider sur quel nœud une requête d’allocation mémoire distante peut être satisfaite,sur quel core une nouvelle tâche doit être placée ou encore, vers quel core une tâche doit êtretransférée lors d’une opération d'équilibrage de charge. Dans cette section, nous décrivonsnotre solution à ce problème qui consiste à doter le noyau d’ALMOS d’un mécanismedécentralisé de représentation distribuée des ressources globalement disponibles. Cettereprésentation prend en compte le caractère NUMA de l’architecture. Ce mécanismedécentralisé et ses politiques de prise de décision constituent la DQDT (DistributedQuaternary Desion Tree). ",  from  https://www-soc.lip6.fr/trac/almos/chrome/site/phd_thesis_ghassan_almaless_2014.pdf

## Quel est le rôle de la fonction lapic_init() ?  
