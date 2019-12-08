# CR TP8_ARCHI4



## Questions sur le preloader

### À quoi sert un preloader ?
- Son rôle est de charger en mémoire RAM le code du bootloader spécifique au système d'exploitation qu'on veut charger.

### Où se trouve stocké le code du preloader au démarrage de la plateforme TSAR? Comment les coeurs accèdent_ils au code du preloader ?
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

### A quel moment les cores sortent-ils du preloader ?
- All other processors, when exiting wait mode, read from XICU the address
to jump.
- This address is the boot-loader entry address that has been written in
the mailbox by the IPI sent by processor 0






## Questions sur le bootloader

### À quoi sert le bootloader ?


### Quels périphériques doit il pouvoir accéder ?


### Quels formats de fichier doit-il pouvoir analyser ?


### Dans quel(s) fichier(s) est rangée la description de la plateforme matérielle ? 


### Pourquoi le code du bootloader est-il recopié dans tous les clusters ?


### Pourquoi chaque core, a-t-il besoin d'une pile ? où sont-elles placées ?


### Qu'est-ce qu'une IPI ? Comment sont-elles utilisées par le bootloader ? 




## Questions sur kernel_init

### Comment dans chaque cluster, le noyau a-t-il accès à la description de la plate-forme matérielle ?


### Comment almos_mkh identifie-t-il les coeurs de la plate-forme matérielle ? 


### Les threads kernel IDLE exécutent, sur chaque coeur la fonction kernel_init(). Le thread IDLE est aussi le thread choisi par le scheduler d'un coeur quand tous les autres threads placés sur ce coeur sont bloqués. Où sont rangés les descripteurs de threads IDLE ? Combien y a-t-il de threads idle ?


### Pour quelle raison la fonction kernel_init() utilise-t-elle des barrières de synchronisation ? Illustrez votre réponse avec un exemple.


### Pour quelle raison, utilise-t-on à la fois des barrières locales et des barrières globales ?


### Dans le cas d'une barrière globale, expliquez précisément la valeur du premier argument passé à la fonction xbarrier_wait(). L'API permettant l'accès à une barrière globale est définie dans les fichiers ​almos-mkh/kernel/libk/xbarrier.h et ​almos-mkh/kernel/libk/xbarrier.c.


### Quel est le rôle de la fonction cluster_init() définie dans le fichier ​almos-mkh/kernel/kern/cluster.c ?


### Quelle est l'utilité de la structure de donnée distribuée DQDT ? L'API permettant l'accès à la DQDT est définie dans les fichiers ​almos-mkh/kernel/kern/dqdt.h et ​almos-mkh/kernel/kern/dqdt.c.


### Quel est le rôle de la fonction lapic_init() ?  
