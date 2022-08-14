#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

void usage(char *name)
{
	printf("Usage:\n\t%s <-f (filename)>  <-t (timer value)>\n", name);
}

int main(int argc, char *argv[]) 
{
	FILE *fp, *ft;
	char *path = "/sys/module/infotecs/parameters/path";
	char *timer = "/sys/module/infotecs/parameters/timer";
	char *filename = NULL;
	int timer_value = 0;
	int opt = 0;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	while ((opt = getopt(argc, argv, "f:t:")) != -1)
	{
		switch(opt)
		{
			case 'f':
				filename = optarg;
				printf("Setting filename for writing: %s\n", optarg);
				
				fp = fopen(path, "w+");
				
				if (!fp) {
					fprintf(stderr, "Error while opening file '%s' (error: %d)\n", filename, errno);
					return errno;
				}
				
				fprintf(fp, "%s", filename);
				fclose(fp);
				break;
			case 't':
				timer_value = atoi(optarg);
				printf("Setting timer value to: %s\n", optarg);
				
				ft = fopen(timer, "w+");
				
				if (!ft) {
					fprintf(stderr, "Error while opening file '%d' (error: %d)\n", timer_value, errno);
					return errno;
				}
				
				fprintf(ft, "%d", timer_value);
				fclose(ft);
				break;
			default:
				usage(argv[0]);
		}
	}

	return errno;
}
