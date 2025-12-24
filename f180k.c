// gcc -Wall -O2 -o f180k f180k.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 512
#define DISK_SIZE_180K (180 * 1024)

static const unsigned char boot_sector[] = {
  0xeb,0x3c,0x90,'m','k','f','s','.','f','a','t',0x00,
  0x02,0x01,0x01,0x00,0x02,0x00,0x02,0x60,0x01,0xf8,
  0x01,0x00,0x10,0x00,0x02,0x00,0,0,0,0,0,0,
  0x80,0x00,0x29,0xa4,0xf3,0xb3,0xaa,
  'N','O',' ','N','A','M','E',' ',' ',' ',' ',
  'F','A','T','1','2',' ',' ',' '
};

static const unsigned char boot_sig[] = { 0x55, 0xAA };
static const unsigned char fat_init[] = { 0xF8, 0xFF, 0xFF };

int main(int argc, char *argv[]) {
    FILE *f;
    unsigned char zero[SECTOR_SIZE];
    int i;

    if (argc != 2) {
        printf("uso: %s imagem.img\n", argv[0]);
        return 1;
    }

    memset(zero, 0, sizeof(zero));

    /* criar imagem vazia */
    f = fopen(argv[1], "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    for (i = 0; i < DISK_SIZE_180K / SECTOR_SIZE; i++)
        fwrite(zero, SECTOR_SIZE, 1, f);

    fclose(f);

    /* escrever boot + FAT */
    f = fopen(argv[1], "rb+");
    if (!f) {
        perror("fopen");
        return 1;
    }

    /* boot sector */
    fwrite(boot_sector, sizeof(boot_sector), 1, f);
    fseek(f, 510, SEEK_SET);
    fwrite(boot_sig, 2, 1, f);

    /* FAT 1 */
    fseek(f, SECTOR_SIZE, SEEK_SET);
    fwrite(fat_init, sizeof(fat_init), 1, f);

    fclose(f);

    puts("Imagem FAT12 180K criada com sucesso.");
    return 0;
}

