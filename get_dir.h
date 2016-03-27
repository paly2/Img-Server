#define DATA_MAX_SIZE 50000
#define IMG_FOLDER_NAME "images"
#define IMG_DIRECTORY "/home/administrateur/public-imgserver/" IMG_FOLDER_NAME
int make_response(const char *data, unsigned long *size, char *response, const char *content_type);
int get_dir(const char* dir, char** html, unsigned long *size, char* content_type);
int get_file(char* path, char** data, unsigned long *size, char* content_type);
void get_unauthorized(char **data, unsigned long *size, char *content_type, const char *directory);
void get_404(char **data, unsigned long *size, char *content_type, const char *directory);
