#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "kmod_header.h"

int main(int argc, char* argv[]) {
    if (argc > 4) {
        fprintf(stderr, "Too much arguments. Usage <path>, <vendor>, <device>\n");
        return 0;
    } else if (argc < 4) {
        fprintf(stderr, "Not enough arguments. Usage <path>, <vendor>, <device>\n");
	return 0;
    }
	char inbuf[BUF_SIZE];
	char outbuf[BUF_SIZE];
	char* path = argv[1];
  unsigned int vendor = strtoul(argv[2], NULL, 16);
  unsigned int device = strtoul(argv[3], NULL, 16);
	sprintf(inbuf, "%s %u %u\n", path, vendor, device);
  printf("path: %s\n", path);
  printf("vendor: %u\n", vendor);
  printf("device: %u\n\n", device);
  FILE* kmod_args = fopen("/proc/lab2/args", "w");
  if (kmod_args == NULL) {
	  printf("File /proc/lab2/args not found");
	  return 0;
  }
  fwrite(&inbuf, sizeof(inbuf), 1, kmod_args);
  fclose(kmod_args);
  FILE* kmod_info = fopen("/proc/lab2/info", "r");
  if (kmod_info == NULL) {
	  printf("File /proc/lab2/info not found");
	  return 0;
  }
  struct result* result = (struct result*) malloc(sizeof(struct result));
  fread(&outbuf, sizeof(struct result), 1, kmod_info);
  int i;
  char* c;
  c = (char*) result;
  for (i = 0; i < sizeof(struct result); i++) {
	  *c = outbuf[i];
	  c++;
  }
  fclose(kmod_info);
  printf("DENTRY\n");
  printf("Name: %s\n", result->md.name);
  printf("Parent: %s\n", result->md.parent);
  printf("Flags: %u\n", result->md.d_flags);
  printf("Inode UID: %u\n", result->md.i_uid);
  printf("Inode GID: %u\n", result->md.i_gid);
  printf("\nPCI_DEV\n");
  printf("Name: %s\n", result->mpd.name);
  printf("Vendor: %u\n", result->mpd.vendor);
  printf("Class: %u\n", result->mpd.class);
  printf("Device: %u\n", result->mpd.device);
  free(result);	
  return 0;
}
