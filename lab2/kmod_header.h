#define BUF_SIZE 2048
#define NAME_LEN 50

struct my_dentry {
	unsigned int d_flags;
    char name[NAME_LEN];
    char parent[NAME_LEN];
    unsigned int i_uid;
    unsigned int i_gid;
};

struct my_pci_dev {
    unsigned short vendor;
    unsigned short device;
    unsigned int class;
    char name[NAME_LEN];
};

struct result {
    struct my_dentry md;
    struct my_pci_dev mpd;
};
