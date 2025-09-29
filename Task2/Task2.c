#include <stdio.h>
#include <stdlib.h> //distinguish between files and directories
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h> //maximum path length
#include <sys/stat.h>


bool checkExtension(const char *fileName){
	size_t length = strlen(fileName);
	return (length > 2 && strcmp(fileName + length - 2, ".c") == 0);		
}

char* newFileName(const char *src){
	const char *extension = ".orig";
	char *dst = (char *)malloc(strlen(src) + strlen(extension) + 1);
	if(!dst){
		perror("malloc error\n");
		exit(1);
	}
	
	strncpy(dst, src, strlen(src));
	strcat(dst, extension);
	return dst;
}

void copyFile(const char *src){
	char *dst = newFileName(src);
	char * buffer[4096];
	size_t bytes_read;
	FILE *srcFilePtr, *dstFilePtr;
	
	srcFilePtr = fopen(src, "r");
	
	if(srcFilePtr){
		dstFilePtr = fopen(dst, "w");
		
		if(dstFilePtr){
			while((bytes_read = fread(buffer, 1, 4096, srcFilePtr)) > 0){
			
				fwrite(buffer, 1, bytes_read, dstFilePtr);
			}
			fclose(dstFilePtr);
		}
		else
			printf("skipping file %s\n", src);
			
		fclose(srcFilePtr);
	}	
	else 
		printf("skipping file %s\n", src);
		
	
	free(dst);
}

void traverse(const char *path){
	
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
	
	bool isWritable = (access(path, W_OK) == 0);
	
	if(!isWritable)	printf("Skipping files in dir %s: Permission denied\n", path);

	struct dirent *entry;
	while((entry = readdir(d)) != NULL){
		
		if(strcmp(entry -> d_name, ".") == 0 || strcmp(entry -> d_name, "..") == 0) 
			continue;
		
		//construct full pathname
		char fullPath[PATH_MAX];
		snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry -> d_name);//add dir name to initial path to get full path to dir
		
		//printf("%s %s\n", entry -> d_name, fullPath);
		
		struct stat entryInfo;
						
		if(lstat(fullPath, &entryInfo) == 0){
			
			if(S_ISDIR(entryInfo.st_mode)){
				
				traverse(fullPath);
			
			}
			else if(isWritable && S_ISREG(entryInfo.st_mode) && checkExtension(entry -> d_name)){
				if (access(fullPath, R_OK) == 0) {
					copyFile(fullPath);
				}
				else{
					printf("skipping file %s: no read permission\n", fullPath);
				}
			}
		} 
	}
	//fstat(src_fd, &entryInfo); //get file permissions
	closedir(d);
}


int main(){
	traverse(".");
	return 0;
}
