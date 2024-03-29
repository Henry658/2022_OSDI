#ifndef _CPIO_H
#define CPIO_H

typedef struct {//cpio_newc_header
	char	c_magic[6];
	char	c_ino[8];
	char	c_mode[8];
	char	c_uid[8];
	char	c_gid[8];
	char	c_nlink[8];
	char	c_mtime[8];
	char	c_filesize[8];
	char	c_devmajor[8];
	char	c_devminor[8];
	char	c_rdevmajor[8];
	char	c_rdevminor[8];
	char	c_namesize[8];
	char	c_check[8];
} cpio_header_type;

void init_cpio();

unsigned int get_cpio_address();


void ls();


void cat(char * str);

int get_usr_program_address(char * str);

int get_usr_program_size(char * str);

void lab3_basic_1(char *str);

#endif
