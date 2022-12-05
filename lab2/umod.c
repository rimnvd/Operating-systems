#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc > 2) {
        fprintf(stderr, "Too much arguments. Please, try again\n");
        return 0;
    } else if (argc < 2) {
        fprintf(stderr, "Not enough arguments. Please, try again\n");
	return 0;
    }
	char* path = argv[1];
	printf("path: %s\n\n", path);
	FILE* args = fopen("/proc/lab2/args", "w");
	fprintf(args, "%s", path);
	fclose(args);
	FILE* result = fopen("/proc/lab2/result", "r");
	char info;
	while(fscanf(result, "%c", &info) != EOF) {
		printf("%c", info);
	}
	fclose(result);
	return 0;
}
		