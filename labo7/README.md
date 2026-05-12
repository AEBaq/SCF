# Charger le module
insmod axi4lite_driver.ko

# Vérifier les messages kernel
dmesg | tail -5

# Lancer les tests
./axi4lite_test

# Décharger
rmmod axi4lite_driver

User-space app
     │  open("/dev/axi4lite")
     │  ioctl(AXI4LITE_READ / AXI4LITE_WRITE)
     v
Kernel : axi4lite_driver.ko
     │  ioremap(0xFF200000)  →  vérification 0xBADB100D au init
     │  ioread32 / iowrite32
     v
AXI4-Lite IP (FPGA via lwhps2fpga bridge)s