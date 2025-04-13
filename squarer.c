#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define DRIVER "squarer"

#define EOK 0

static int dev_open = 0;

static int dev_major = 0;
#define DEV_FIRST 0
#define DEV_COUNT 1
#define DEV_GROUP "squarer"

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
    int res = EOK;

    dev_t dev = 0;
    res = alloc_chrdev_region(&dev, DEV_FIRST, DEV_COUNT, DEV_GROUP);
    dev_major = MAJOR(dev);

    if (res < 0) {
        printk(KERN_CRIT "%s: char device didn't created\n", DRIVER);
        goto err;
    }

    cdev_init(&squarer_dev, &squarer_fops);
    squarer_dev.owner = THIS_MODULE;

    res = cdev_add(&squarer_dev, MKDEV(dev_major, DEV_FIRST), DEV_COUNT);
    if (res < 0) {
        printk(KERN_CRIT "%s: device files didn't created\n", DRIVER);
        goto err;
    }

    printk(KERN_INFO "%s: loaded\n", DRIVER);
    return EOK;

err:
    unregister_chrdev_region(MKDEV(dev_major, DEV_FIRST), DEV_COUNT);
    return res;
}

static void __exit squarer_exit(void) {
    cdev_del(&squarer_dev);
    unregister_chrdev_region(MKDEV(dev_major, DEV_FIRST), DEV_COUNT);
    printk(KERN_INFO "%s: unloaded\n", DRIVER);
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

        printk(KERN_INFO "%s in: %s", DRIVER, input);
        printk(KERN_INFO "%s out: %s", DRIVER, output);
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
    if (*pos >= BUF_SIZE) {
        return -EINVAL;
    }
    if (*pos + count > BUF_SIZE) {
        count = BUF_SIZE - *pos;
    }
    if (copy_from_user(input + *pos, buf, count)) {
        return -EINVAL;
    }

    *pos += count; input_pos = *pos;
    return count;
}

module_init(squarer_init);
module_exit(squarer_exit);

MODULE_LICENSE("GPL");