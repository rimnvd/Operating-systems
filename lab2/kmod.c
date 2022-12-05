#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>
#include <linux/device.h>
#include <linux/pci.h>

#define BUF_SIZE 1024

MODULE_AUTHOR("rimnvd");
MODULE_DESCRIPTION("os-lab2");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static struct proc_dir_entry* proc_entry;
struct path current_path;  
int dev_count;
static char* dev_buf;

static ssize_t kmod_args_write (struct file* file, const char __user* ubuf, size_t count, loff_t* offset) {
    pr_info("kmod: starts write path");
    char kbuf[BUF_SIZE];
    char path_name[BUF_SIZE];
    if (*offset > 0 || count > BUF_SIZE) {
        return -EFAULT;
    }
    if (copy_from_user(kbuf, ubuf, count)) {
        return -EFAULT;
    }
    int  char_write_count =  sscanf(kbuf, "%s", &path_name);
    if (path_name == NULL) {
        pr_info("kmod: path is NULL!");
        return count;
    }
    pr_info("kmod: path: %s", path_name);
    if (kern_path(path_name, LOOKUP_FOLLOW, &current_path)) {
        pr_err("kmod: path not found!");
        return count;
    }
    *offset = strlen(kbuf);
    return strlen(kbuf);
}

static size_t dentry_struct_write(char __user* ubuf) {
	pr_info("kmod: starts write dentry");
    struct dentry* current_dentry = current_path.dentry;
    struct pci_dev* pci;
    char kbuf[BUF_SIZE];
    int len = 0;
    len += sprintf(kbuf, "\nDENTRY\n");
    len += sprintf(kbuf + len, "Name : %s\n", current_dentry->d_name.name);
    len += sprintf(kbuf + len, "Parent name : %s\n", current_dentry->d_parent->d_name.name);
    len += sprintf(kbuf + len, "Inode UID : %d\n", current_dentry->d_inode->i_uid);
    len += sprintf(kbuf + len, "Inode GID : %d\n", current_dentry->d_inode->i_gid);
    if (copy_to_user(ubuf + dev_count,  kbuf, len)) {
        return -EFAULT;
    }
    return len;
}

static size_t device_struct_write(char __user* ubuf) {
	pr_info("kmod: starts write device");
    struct pci_dev* pci;
	dev_count += sprintf(dev_buf, "DEVICE\n");
    for_each_pci_dev(pci) {
        dev_count += sprintf(dev_buf + dev_count, "\n");
        dev_count += sprintf(dev_buf + dev_count, "Pci: [%d]\n", pci->device);
        dev_count += sprintf(dev_buf + dev_count, "Id: %d\n", pci->dev.id);
        dev_count += sprintf(dev_buf + dev_count, "Init name: %s\n", (pci->dev.init_name));
        dev_count += sprintf(dev_buf + dev_count, "Parent: %s\n", (pci->dev.parent->init_name));
		dev_count += sprintf(dev_buf + dev_count, "Kobject: %s\n", (pci->dev.kobj.name));
		dev_count += sprintf(dev_buf + dev_count, "Type: %s\n", (pci->dev.type->name));
		dev_count += sprintf(dev_buf + dev_count, "Bus: %s\n", (pci->dev.bus->name));
        if (dev_count > 90000) break;
    }
    if (copy_to_user(ubuf, dev_buf, dev_count)) {
        return -EFAULT;
    }
    return dev_count;
}

static ssize_t kmod_result_read(struct file* file, char __user* ubuf, size_t count, loff_t* offset) {
	pr_info("kmod: starts read result");
    char buf[BUF_SIZE];
    int len = 0;
    if (*offset > 0) {
        return 0;
    }
	len += device_struct_write(ubuf);
    len += dentry_struct_write(ubuf);
    *offset = len;
    return len;
}

static struct file_operations fops  = {
    .owner = THIS_MODULE,
    .read = kmod_result_read,
    .write = kmod_args_write
};

static int __init kmod_init(void) {
    pr_info("kmod: module successfully initialized");
    proc_entry = proc_mkdir("lab2", NULL);
    if (proc_entry == NULL) {
        pr_err("kmod: could not create lab2 directory!");
        return -1;
    }
    proc_create("args", 0666, proc_entry, &fops);
    proc_create("result", 0666, proc_entry, &fops);
    dev_buf = kmalloc(100000, GFP_KERNEL);
	if (dev_buf == NULL) {
		pr_err("kmod: could not allocate memory!");
		return -1;
	}
    return 0;
}

static void __exit kmod_exit(void) {
    proc_remove(proc_entry);
	kfree(dev_buf);
    pr_info("kmod: module successfully removed");
}

module_init(kmod_init);
module_exit(kmod_exit);
