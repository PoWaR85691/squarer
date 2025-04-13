#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define EOK 0

static int dev_open = 0;

static int dev_major = 0;
#define DEV_NAME "squarer"

static struct class *dev_class;
static struct device *dev_device;
static struct cdev squarer_dev;

static int squarer_open(struct inode *inode, struct file *file);
static int squarer_release(struct inode *inode, struct file *file);
static ssize_t squarer_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
static ssize_t squarer_write(struct file *file, const char __user *buf, size_t count, loff_t *pos);

static const struct file_operations squarer_fops = {
    .owner = THIS_MODULE,
    .open = squarer_open,
    .release = squarer_release,
    .read = squarer_read,
    .write = squarer_write
};

#define BUF_SIZE 64
static char input[BUF_SIZE] = "";
static char output[BUF_SIZE] = "0\n";
static int input_pos = 0;
static int output_pos = 0;

static int __init squarer_init(void) {
    dev_t dev = 0;
    if (alloc_chrdev_region(&dev, 0, 1, DEV_NAME) < 0) {
        goto err_alloc_chrdev_region;
    }
    dev_major = MAJOR(dev);

    dev_class = class_create(DEV_NAME);
    if (IS_ERR(dev_class)) {
        goto err_class_create;
    }

    dev_device = device_create(dev_class, NULL, MKDEV(dev_major, 0), NULL, DEV_NAME);
    if (IS_ERR(dev_device)) {
        goto err_device_create;
    }

    cdev_init(&squarer_dev, &squarer_fops);
    squarer_dev.owner = THIS_MODULE;

    if (cdev_add(&squarer_dev, MKDEV(dev_major, 0), 1) < 0) {
        goto err_cdev_add;
    }

    printk(KERN_INFO "%s: loaded\n", DEV_NAME);
    return 0;

err_cdev_add:
    cdev_del(&squarer_dev);
    device_destroy(dev_class, MKDEV(dev_major, 0));
err_device_create:
    class_destroy(dev_class);
err_class_create:
    unregister_chrdev_region(MKDEV(dev_major, 0), 1);
err_alloc_chrdev_region:
    printk(KERN_CRIT "%s: module didn't loaded\n", DEV_NAME);
    return 1;
}

static void __exit squarer_exit(void) {
    cdev_del(&squarer_dev);
    device_destroy(dev_class, MKDEV(dev_major, 0));
    class_destroy(dev_class);
    unregister_chrdev_region(MKDEV(dev_major, 0), 1);
    printk(KERN_INFO "%s: unloaded\n", DEV_NAME);
}

static int squarer_open(struct inode *inode, struct file *file) {
    if (dev_open) return -EBUSY;
    ++dev_open;

    input_pos = 0;
    output_pos = 0;

    return EOK;
}

static int squarer_release(struct inode *inode, struct file *file) {
    --dev_open;

    if (input_pos > 0) {
        input[input_pos] = '\0';

        u64 val, res;
        if (sscanf(input, "%llu", &val) == 1) {
            res = val * val;
        } else {
            res = 0;
        }
        snprintf(output, BUF_SIZE, "%llu\n", res); 

        printk(KERN_INFO "%s in: %s", DEV_NAME, input);
        printk(KERN_INFO "%s out: %s", DEV_NAME, output);
    }

    return EOK;
}

static ssize_t squarer_read(struct file *file, char __user *buf, size_t count, loff_t *pos) {
    size_t len = strlen(output);
    if (*pos >= len) {
        return 0;
    }
    if (*pos + count > len) {
        count = len - *pos;
    }
    if (copy_to_user(buf, output + *pos, count) != EOK) {
        return -EINVAL;
    }

    *pos += count; output_pos = *pos;
    return count;
}

static ssize_t squarer_write(struct file *file, const char __user *buf, size_t count, loff_t *pos) {
    if (*pos >= BUF_SIZE - 1) {
        return -EINVAL;
    }
    if (*pos + count > BUF_SIZE - 1) {
        count = BUF_SIZE - 1 - *pos;
    }
    if (copy_from_user(input + *pos, buf, count) != EOK) {
        return -EINVAL;
    }

    *pos += count; input_pos = *pos;
    return count;
}

module_init(squarer_init);
module_exit(squarer_exit);

MODULE_LICENSE("GPL");