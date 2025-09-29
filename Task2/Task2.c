#include <stdio.h>
#include <stdlib.h> //distinguish between files and directories
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h> //maximum path length
#include <sys/stat.h>

void traverse(const char *path, int n){
	
	if (access(path, R_OK | X_OK) != 0) {
    		printf("skipping dir %s: no read permission\n", path);
    		return;
	}	
	
	DIR *d = opendir(path);
	if(!d){ //check if dir can be opened if no print the error code and the path of the dir
		int err = errno;
		printf("couldn't open dir, error: %d path: %s\n", err, path);
		return;
	}
	

	struct dirent *entry;
	while((entry = readdir(d)) != NULL){
		
		if(strcmp(entry -> d_name, ".") == 0 || strcmp(entry -> d_name, "..") == 0) 
			continue;
		
		printf("%*s%s\n", n, "", entry -> d_name);
		
		struct stat entryInfo;			
		char fullPath[PATH_MAX];
		snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry -> d_name);//add dir name to initial path to get full path to dir
		if((lstat(fullPath, &entryInfo) == 0) &&  S_ISDIR(entryInfo.st_mode)){	
			traverse(fullPath, (n + 4));
		}
		
	}

	closedir(d);
}


int main(){
	traverse(".", 0);
	return 0;
}
