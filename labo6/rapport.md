# SCF, Laboratoire n°6
**Student :** Emily Baquerizo

## Flash SD
```sh
nox$ picocom -b 115200 /dev/ttyUSB0
picocom v2024-07

port is        : /dev/ttyUSB0
flowcontrol    : none
baudrate is    : 115200
parity is      : none
databits are   : 8
stopbits are   : 1
txdelay is     : 0 ns
escape is      : C-a
local echo is  : no
noinit is      : no
noreset is     : no
hangup is      : no
nolock is      : no
send_cmd is    : sz -vv
receive_cmd is : rz -vv -E
imap is        : 
omap is        : 
emap is        : crcrlf,delbs,
logfile is     : none
initstring     : none
exit_after is  : not set
exit is        : no
minimal cmds is: no

Type [C-a] [C-h] to see available commands
Terminal ready

evl-de1 ~ #
```

Le repository du kernel Linux a été clone avec juste un depth de 1 et la branche v6.18 comme demandé dans la consigne.

Ensuite, j'ai crée un nouveau fichier se basant sur le device tree de la DE0 `linux/linux/arch/arm/boot/dts/intel/socfpga/socfpga_cyclone5_de0_nano_soc.dts` afin d'avoir un device tree pour notre DE1. Le Makefile a également été modifié en ajoutant notre nouveau fichier dedans.

La toolchain a été téléchargé entre temps sur ma machine et la compilation a pu être faite en s'assurant d'indiquer la bonne toolchain. Puis, j'ai crée l'itb en complétant au préalable le .its fourni avec les chemins de la zImage et du .dtb créés avant.

Le .itb a été placé dans la bonne partition de la carte SD avant de pouvoir être booté sur la DE1-SoC.

Pour tester tout cela, on peut directement lire nos status avec `/dev/mem` et les offsets correspondants mais pour compléter cela, le code test du labo précédent a été repris et modifié pour ce laboratoire. Ã noter que le test ne couvre pas les cas extrêmes, il permet juste de voir que nous arrivons bel et bien à accéder à nos LED, key, switch, et 7-segment.