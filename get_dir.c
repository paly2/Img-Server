#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "get_dir.h"

int make_response(const char *data, unsigned long *size, char *response, const char *content_type) {	
	printf("%s\n\n", data);
	sprintf(response, "HTTP/1.1 200 OK\r\n"
	"Content-Type: %s\r\n"
	"Content-Length: %u\r\n"
	"Accept-Range: bytes\r\n"
	//"Connexion: close"
	"\r\n", content_type, *size);
	int i;
	for(i = 0 ; response[i] != '\0' ; i++);
	memcpy(response+i, data, *size);
	
	// Now, increase the size with the header size
	*size += i;
	return 0;
}

int filter(const struct dirent *dir) {
	if((dir->d_name)[0] == '.')
		return 0;
	else
		return 1;
}
int get_dir(const char* dir, char** html, unsigned long *size, char *content_type) {
	*html = malloc(DATA_MAX_SIZE*sizeof(char));
	
	char dir_images[strlen(IMG_DIRECTORY)+strlen(dir)+1]; sprintf(dir_images, IMG_DIRECTORY "%s", dir);

	struct dirent **namelist;
	int n;
	n = scandir(dir_images, &namelist, filter, alphasort);
	if(n < 0) {
		perror("scandir");
		return 1;
	}
	
	sprintf(*html, "<!DOCTYPE html><html><head><meta charset=\"utf-8\" /><title>Directory : .%s</title><body>\n"
	"<h1>Directory : .%s</h1>\n"
	"<p><ul>\n", dir, dir);
	
	if(n == 0)
		strcat(*html, "<h3>This directory is empty.</h3>");
	while(n--) {
		char element[strlen(dir)+strlen(namelist[n]->d_name)*2+27];
		sprintf(element, "<a href=\"%s/%s\"><li>%s</li></a>\n", dir, namelist[n]->d_name, namelist[n]->d_name);
		strcat(*html, element);
		free(namelist[n]);
	}
	free(namelist);
	
	strcat(*html, "</ul></p>"
	"<p>Img Server - Writted by <a href=\"mailto:plangrognet@laposte.net\">paly2</a> (Pierre-Adrien Langrognet). This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License.</p>"
	"</body></html>");
	
	strcpy(content_type, "text/html");
	*size = strlen(*html)+1;
	
	return 0;
}

int get_file(char *path, char **data, unsigned long *size, char *content_type) {
	// Find the image format
	char *token = NULL, *format = NULL;
	int binary = 0;
	char path_cpy[strlen(IMG_DIRECTORY)+strlen(path)+1]; sprintf(path_cpy, IMG_DIRECTORY "%s", path);
	
	format = strrchr(path, '.');
	if(format != NULL) {
		format++;
		if(strcmp(format, "png") == 0 || strcmp(format, "gif") == 0 || strcmp(format, "jpg") == 0 || strcmp(format, "jpeg") == 0
		|| strcmp(format, "PNG") == 0 || strcmp(format, "GIF") == 0 || strcmp(format, "JPG") == 0 || strcmp(format, "JPEG") == 0) {
			sprintf(content_type, "image/%s", format);
			binary = 1;
		}
		else {
			if(strcmp(format, "txt") == 0) strcpy(format, "plain");
			sprintf(content_type, "text/%s", format);
		}
	}
	else
		strcpy(content_type, "unknown"); // This should get an error in the browser :>
	
	// Get the file
	FILE *inputfile = NULL;
	int c;
	int n = 0;
	if(binary)
		inputfile = fopen(path_cpy,"rb");
	else
		inputfile = fopen(path_cpy,"r");
	
	
	if(inputfile == NULL) {
		perror("fopen");
		return 1;
	}
	else {
		if(binary) {
			fseek(inputfile, 0, SEEK_END);
			*size = ftell(inputfile);
			fseek(inputfile, 0, SEEK_SET);
			*data = malloc(*size+1);
			fread(*data, *size, 1, inputfile);
		}
		else {
			*data = malloc(DATA_MAX_SIZE*sizeof(char));
			while((c = fgetc(inputfile)) != EOF) {
				*(*data+n) = c;
				n++;
				if(n > DATA_MAX_SIZE) {
					printf("Too much data to send !\n");
					return 1;
				}
			}
			*size = n;
		}
		fclose(inputfile);
	}
	return 0;
}

void get_404(char **data, unsigned long *size, char *content_type, const char* directory) {
	FILE *missingfile = NULL;
	char filename[500] = ""; strcpy(filename, directory);
	*(strrchr(filename, '/')+1) = '\0'; strcat(filename, "404.html");
	printf("Filename: %s\n", filename);
	missingfile = fopen(filename,"r");
	*data = malloc(DATA_MAX_SIZE*sizeof(char));
	
	int c;
	int n = 0;
	while ((c = fgetc(missingfile)) != EOF){
		*(*data+n) = c;
		n++;
	}
	fclose(missingfile);
	strcpy(content_type, "text/html");
	*size = n;
}

void get_unauthorized(char **data, unsigned long *size, char *content_type, const char* directory) {
	FILE *file = NULL;
	char filename[500] = ""; strcpy(filename, directory);
	*(strrchr(filename, '/')+1) = '\0'; strcat(filename, "unauthorized.html");
	file = fopen(filename,"r");
	*data = malloc(DATA_MAX_SIZE*sizeof(char));
	
	int c;
	int n = 0;
	while ((c = fgetc(file)) != EOF){
		*(*data+n) = c;
		n++;
	}
	fclose(file);
	strcpy(content_type, "text/html");
	*size = n;
}
