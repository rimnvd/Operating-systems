#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/mutex.h>

#include "kmod_header.h"

MODULE_AUTHOR("rimnvd");
MODULE_DESCRIPTION("os-lab2");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static struct proc_dir_entry* proc_entry;
struct path current_path; 
struct result* result;
unsigned int vendor, device;

static DEFINE_MUTEX(kmod_mutex);

static int kmod_open(struct inode* inode, struct file* file) {
    if (!mutex_trylock(&kmod_mutex)) {
        return -EBUSY;
    }
    return 0;
}

static int kmod_release(struct inode* inode, struct file* file) {
    mutex_unlock(&kmod_mutex);
    return 0;
}

static ssize_t kmod_args_write (struct file* file, const char __user* ubuf, size_t count, loff_t* offset) {
	char kbuf[BUF_SIZE];
    char path_name[BUF_SIZE];
    pr_info("kmod: starts writing the args");
    if (*offset > 0 || count > BUF_SIZE) {
        return -EFAULT;
    }
    if (copy_from_user(kbuf, ubuf, count)) {
        return -EFAULT;
    }
    sscanf(kbuf, "%s %u %u", path_name, &vendor, &device);
    if (strlen(path_name) == 0) {
		pr_err("kmod: path is NULL!");
        return count;
    }
    pr_info("kmod: path: %s", path_name);
    if (kern_path(path_name, LOOKUP_FOLLOW, &current_path)) {
        pr_err("kmod: path not found!");
        return count;
    }
    pr_info("kmod: vendor %u", vendor);
    pr_info("kmod: device %u", device);
    *offset = strlen(kbuf);
    return strlen(kbuf);
}

int get_structures(void) {
	struct dentry* current_dentry = current_path.dentry;
    struct my_dentry* new_md = vmalloc(sizeof(struct my_dentry));
    struct my_pci_dev* new_mpd = vmalloc(sizeof(struct my_pci_dev));
    struct pci_dev* pci = NULL;
    //struct device dev;
    pr_info("kmod: starts getting information about dentry");
	if (current_dentry == NULL) {
		pr_err("kmod: dentry is null");
		return -1;
	}
	result = vmalloc(sizeof(struct result));
	new_md->d_flags = current_dentry->d_flags;
	new_md->i_uid = (unsigned int)current_dentry->d_inode->i_uid.val;
    new_md->i_gid = (unsigned int)current_dentry->d_inode->i_gid.val;
    strcpy(new_md->name, current_dentry->d_name.name);
    strcpy(new_md->parent, current_dentry->d_parent->d_name.name);
	result->md = *new_md;
	vfree(new_md);
    pr_info("kmod: starts getting information about pci_dev");
    pci = pci_get_device(vendor, device, pci);
    //struct pci_dev* test = NULL;
    // for_each_pci_dev(test) {
    //     pr_info("kmod: pci device %u", test->device);
    //     pr_info("kmod: device id %u", dev_test.id); 
    //     pr_info("kmod: device name %s", dev_test.init_name);
    //     pr_info("kmod: device type: %s", dev_test.type->name);
    //     pr_info("");
    // }
    new_mpd->vendor = pci->vendor;
    new_mpd->device = pci->device;
    new_mpd->class = pci->class;
    strcpy(new_mpd->name, dev_name(&pci->dev));
    result->mpd = *new_mpd;         
    vfree(new_mpd);
	return 0;
}

static ssize_t kmod_result_read(struct file* file, char __user* ubuf, size_t count, loff_t* offset) {
	char buf[BUF_SIZE];
    int len = 0;
    int status;
	char* c;
	int i;
  pr_info("kmod: starts read the result");
    if (*offset > 0 || count > BUF_SIZE) {
        return 0;
    }
   status = get_structures();
   if (status != 0) {
	   pr_info("ploha");
       return -EFAULT;
   }
    c = (char*) result;
    for (i = 0; i < sizeof(struct result); i++) {
        buf[i] = *c++;
    }
    len += i;
    if (copy_to_user(ubuf, buf, len)) {
        return -EFAULT;
    }
    vfree(result);
    *offset = len;
    return len;
}

static struct file_operations fops  = {
    .owner = THIS_MODULE,
    .read = kmod_result_read,
    .write = kmod_args_write,
    .open = kmod_open,
    .release = kmod_release
};

static int __init kmod_init(void) {
    pr_info("kmod: module successfully initialized");
    proc_entry = proc_mkdir("lab2", NULL);
    if (proc_entry == NULL) {
        pr_err("kmod: could not create lab2 directory!");
        return -1;
    }
    proc_create("args", 0666, proc_entry, &fops);
    proc_create("info", 0666, proc_entry, &fops);
    return 0;
}

static void __exit kmod_exit(void) {
    proc_remove(proc_entry);
    pr_info("kmod: module successfully removed");
}

module_init(kmod_init);
module_exit(kmod_exit);
