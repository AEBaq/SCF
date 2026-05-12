# SCF, Laboratoire n°7 : Driver Linux pour IP AXI4-Lite
**Student :** Emily Baquerizo

## Driver

Le driver expose un character device `/dev/axi4lite` depuis lequel le user-space peut accéder aux registres de notre IP AXI4-Lite. L'interface repose sur `ioctl` avec deux commandes, `AXI4LITE_READ` et `AXI4LITE_WRITE` (défini dans le header).

À l'initialisation du module, le driver effectue un `ioremap` de l'adresse physique `0xFF200000` puis lit la constante `0xBADB100D` pour vérifier que le bitstream est bien chargé sur le FPGA et continuer après validation.

## Changement avec le laboratoire 06

Au laboratoire 06, l'accès aux registres se faisait directement via `/dev/mem` et `mmap`. Un registre était simplement un pointeur déréférencé sur l'adresse physique mappée en user-space. Ici, cet accès physique est déplacé dans le kernel et l'application ne voit plus que le fichier `/dev/axi4lite`. Cette fois-ci, nous avons deux helpers `reg_read` et `reg_write` qui encapsulent les appels ioctl.

En terme des tests, les principes restent un peu les mêmes à la différence que les tests ont été récrit pour améliorer la lisibilité et propreté du code ainsi que pour compléter légèrement les tests.