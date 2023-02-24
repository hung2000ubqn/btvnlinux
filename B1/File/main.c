#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysmacros.h>

int main(int argc, char const *argv[])
{
	struct stat in4;
	
	if (argc != 2) {
		printf("Usage: ./file-infor /path/to/file");
		exit(EXIT_FAILURE);
	}

	if (lstat(argv[1], &in4) == -1) {
		printf("lstat\n");
		exit(EXIT_FAILURE);
	}

	printf("Loai File:  ");
	switch (in4.st_mode & S_IFMT) 
	{
		case S_IFBLK:  printf("block device\n");            break;
        	case S_IFCHR:  printf("character device\n");        break;
        	case S_IFDIR:  printf("directory\n");               break;
        	case S_IFIFO:  printf("FIFO/pipe\n");               break;
        	case S_IFLNK:  printf("symlink\n");                 break;
        	case S_IFREG:  printf("regular file\n");            break;
        	case S_IFSOCK: printf("socket\n");                  break;
        	default:       printf("unknown?\n");                break;
        }

    	printf("Last time modified: %s\n", ctime(&in4.st_mtim.tv_sec));
	printf("Size of file:  %jd bytes\n", (intmax_t)in4.st_size);
	
	return 0;
}
