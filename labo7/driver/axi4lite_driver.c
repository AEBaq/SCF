/*****************************************************************************************
 * HEIG-VD
 * Haute Ecole d'Ingenerie et de Gestion du Canton de Vaud
 * School of Business and Engineering in Canton de Vaud
 *****************************************************************************************
 * REDS Institute
 * Reconfigurable Embedded Digital Systems
 *****************************************************************************************
 *
 * File                 : axi4lite_driver.c
 * Author               : Me
 * Date                 : Today
 *
 * Context              : SCF tutorial lab
 *
 *****************************************************************************************
 * Brief: J'ai tellement faim
 *
 *****************************************************************************************
 * Modifications :
 * Ver    Date        Student      Comments
 * 1.0    21.04.2026  AEB          Lab07
 *
*****************************************************************************************/

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "../include/axi4lite_ioctl.h"

#define DEVICE_NAME "axi4lite"

struct axi4lite_dev {
    void __iomem   *base;
    dev_t           dev_num;
    struct cdev     cdev;
    struct class   *class;
};

/* Pointeur global minimal requis pour exit() */
static struct axi4lite_dev *g_adev;

/**
 * @brief IOCTL handler for the device
 * @param file Pointer to the file structure
 * @param cmd IOCTL command
 * @param arg IOCTL argument (register offset and value)
 * @return 0 on success, negative error code on failure
 */
static long axi4lite_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct axi4lite_dev *adev = container_of(file->f_inode->i_cdev, struct axi4lite_dev, cdev);
    uint32_t offset, value;

    if (_IOC_TYPE(cmd) != AXI4LITE_NB)
        return -ENOTTY;

    offset = _IOC_NR(cmd) << 2;
    if (offset + sizeof(uint32_t) > AXI_PAGE_SIZE)
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

/**
 * @brief Module initialization: Maps the device memory, checks the constant, registers the character device and creates the device node
 * @return 0 on success, negative error code on failure
 */
static int __init axi4lite_init(void) {
    struct axi4lite_dev *adev;
    uint32_t constant;
    int ret;

    adev = kzalloc(sizeof(*adev), GFP_KERNEL);
    if (!adev) {
        pr_err("[axi4lite] Failed to allocate device structure\n");
        return -ENOMEM;
    }

    // Map the device memory
    adev->base = ioremap(AXI_HPS_BASE_ADDR, AXI_PAGE_SIZE);
    if (!adev->base) {
        pr_err("[axi4lite] Failed to map device memory\n");
        ret = -ENOMEM;
        goto err_ioremap;
    }

    // Check the constant register to verify that the design is present and correct
    constant = ioread32(adev->base + AXI4LITE_REG_ID);
    if (constant != AXI4LITE_CONSTANT) {
        pr_err("[axi4lite] Design not detected (read=0x%08X, expected=0x%08X)\n",
               constant, AXI4LITE_CONSTANT);
        ret = -ENODEV;
        goto err_ioread;
    }

    // Allocate a character device number
    ret = alloc_chrdev_region(&adev->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("[axi4lite] alloc_chrdev_region() failed (%d)\n", ret);
        goto err_alloc_chrdev;
    }

    // Initialize and add the character device
    cdev_init(&adev->cdev, &axi4lite_fops);
    ret = cdev_add(&adev->cdev, adev->dev_num, 1);
    if (ret < 0) {
        pr_err("[axi4lite] cdev_add() failed (%d)\n", ret);
        goto err_cdev_add;
    }

    // Create the device class and node
    adev->class = class_create(DEVICE_NAME);
    if (IS_ERR(adev->class)) {
        pr_err("[axi4lite] class_create() failed\n");
        ret = PTR_ERR(adev->class);
        goto err_class_create;
    }

    device_create(adev->class, NULL, adev->dev_num, NULL, DEVICE_NAME);

    g_adev = adev;
    pr_info("[axi4lite] Driver loaded, /dev/%s ready\n", DEVICE_NAME);
    return 0;

err_class_create:
    cdev_del(&adev->cdev);
err_cdev_add:
    unregister_chrdev_region(adev->dev_num, 1);
err_alloc_chrdev:
err_ioread:
    iounmap(adev->base);
err_ioremap:
    kfree(adev);
    return ret;
}

/**
 * @brief Module exit: Unmaps the device memory, unregisters the character device and destroys the device node
 * @return void
 */
static void __exit axi4lite_exit(void)
{
    struct axi4lite_dev *adev = g_adev;

    device_destroy(adev->class, adev->dev_num);
    class_destroy(adev->class);
    cdev_del(&adev->cdev);
    unregister_chrdev_region(adev->dev_num, 1);
    iounmap(adev->base);
    kfree(adev);
    pr_info("[axi4lite] Driver unloaded\n");
}

module_init(axi4lite_init);
module_exit(axi4lite_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Me");
MODULE_DESCRIPTION("Driver for AXI4-Lite peripheral");
MODULE_VERSION("1.0");