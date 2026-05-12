#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>

#include "../include/axi4lite_ioctl.h"

#define DRIVER_NAME     "axi4lite"
#define BASE_ADDR       0xFF200000
#define MAP_SIZE        0x1000
#define EXPECTED_CONST  0xBADB100D

/* ------------------------------------------------------------------ */
/*  Structure d'état du driver — remplace toutes les globales           */
/* ------------------------------------------------------------------ */
struct axi4lite_dev {
    void __iomem   *base;
    dev_t           dev_num;
    struct cdev     cdev;
    struct class   *class;
};

/* ------------------------------------------------------------------ */
/*  Opérations fichier                                                  */
/* ------------------------------------------------------------------ */
static long axi4lite_ioctl(struct file *file,
                           unsigned int cmd,
                           unsigned long arg)
{
    struct axi4lite_dev *adev = container_of(file->f_inode->i_cdev,
                                             struct axi4lite_dev, cdev);
    uint32_t offset, value;

    /* Vérification du magic number */
    if (_IOC_TYPE(cmd) != AXI4LITE_MAGIC)
        return -ENOTTY;

    /* Reconstruction de l'offset depuis le numéro de commande */
    offset = _IOC_NR(cmd) << 2;

    if (offset + sizeof(uint32_t) > AXI_MAP_SIZE)
        return -EINVAL;

    if (_IOC_DIR(cmd) & _IOC_READ) {
        value = ioread32(adev->base + offset);
        if (put_user(value, (uint32_t __user *)arg))
            return -EFAULT;

    } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
        if (get_user(value, (uint32_t __user *)arg))
            return -EFAULT;
        iowrite32(value, adev->base + offset);

    } else {
        return -ENOTTY;
    }

    return 0;
}

static const struct file_operations axi4lite_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = axi4lite_ioctl,
};

/* ------------------------------------------------------------------ */
/*  Fonctions d'initialisation (découpées pour la lisibilité)           */
/* ------------------------------------------------------------------ */
static int axi4lite_map_and_check(struct axi4lite_dev *adev)
{
    uint32_t constant;

    adev->base = ioremap(BASE_ADDR, MAP_SIZE);
    if (!adev->base) {
        pr_err("[axi4lite] ioremap() a échoué\n");
        return -ENOMEM;
    }

    constant = ioread32(adev->base + REG_CONST);
    if (constant != EXPECTED_CONST) {
        pr_err("[axi4lite] Design absent (lu=0x%08X, attendu=0x%08X)\n",
               constant, EXPECTED_CONST);
        iounmap(adev->base);
        return -ENODEV;
    }

    pr_info("[axi4lite] Design détecté (constante=0x%08X)\n", constant);
    return 0;
}

static int axi4lite_register_cdev(struct axi4lite_dev *adev)
{
    int ret;

    ret = alloc_chrdev_region(&adev->dev_num, 0, 1, DRIVER_NAME);
    if (ret < 0) {
        pr_err("[axi4lite] alloc_chrdev_region() a échoué (%d)\n", ret);
        return ret;
    }

    cdev_init(&adev->cdev, &axi4lite_fops);
    ret = cdev_add(&adev->cdev, adev->dev_num, 1);
    if (ret < 0) {
        pr_err("[axi4lite] cdev_add() a échoué (%d)\n", ret);
        unregister_chrdev_region(adev->dev_num, 1);
        return ret;
    }

    adev->class = class_create(DRIVER_NAME);
    if (IS_ERR(adev->class)) {
        ret = PTR_ERR(adev->class);
        pr_err("[axi4lite] class_create() a échoué (%d)\n", ret);
        cdev_del(&adev->cdev);
        unregister_chrdev_region(adev->dev_num, 1);
        return ret;
    }

    device_create(adev->class, NULL, adev->dev_num, NULL, DRIVER_NAME);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Init / Exit                                                         */
/* ------------------------------------------------------------------ */

/* Pointeur module-level minimal : nécessaire pour exit() */
static struct axi4lite_dev *g_adev;

static int __init axi4lite_init(void)
{
    struct axi4lite_dev *adev;
    int ret;

    adev = kzalloc(sizeof(*adev), GFP_KERNEL);
    if (!adev)
        return -ENOMEM;

    ret = axi4lite_map_and_check(adev);
    if (ret)
        goto err_free;

    ret = axi4lite_register_cdev(adev);
    if (ret)
        goto err_unmap;

    g_adev = adev;
    pr_info("[axi4lite] Driver chargé — /dev/%s disponible\n", DRIVER_NAME);
    return 0;

err_unmap:
    iounmap(adev->base);
err_free:
    kfree(adev);
    return ret;
}

static void __exit axi4lite_exit(void)
{
    struct axi4lite_dev *adev = g_adev;

    device_destroy(adev->class, adev->dev_num);
    class_destroy(adev->class);
    cdev_del(&adev->cdev);
    unregister_chrdev_region(adev->dev_num, 1);
    iounmap(adev->base);
    kfree(adev);
    pr_info("[axi4lite] Driver déchargé\n");
}

module_init(axi4lite_init);
module_exit(axi4lite_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SCF Student");
MODULE_DESCRIPTION("Driver AXI4-Lite pour DE1-SoC (Lab 07)");
MODULE_VERSION("1.0");