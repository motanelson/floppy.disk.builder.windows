// gcc -Wall -O2 -o f288 f288.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SECTOR_SIZE      512
#define DISK_SIZE_2880K  (2880 * 1024)

/* ===== BOOT SECTOR FIELDS ===== */

/* Jump + OEM */
uint8_t  BS_jmpBoot[3] = { 0xEB, 0x3C, 0x90 };
char     BS_OEMName[8] = "mkfsfat";

/* BPB */
uint16_t BPB_BytsPerSec = 512;
uint8_t  BPB_SecPerClus = 2;
uint16_t BPB_RsvdSecCnt = 1;
uint8_t  BPB_NumFATs    = 2;
uint16_t BPB_RootEntCnt = 224;
uint16_t BPB_TotSec16   = 5760;
uint8_t  BPB_Media      = 0xF0;
uint16_t BPB_FATSz16    = 9;
uint16_t BPB_SecPerTrk  = 36;
uint16_t BPB_NumHeads   = 2;
uint32_t BPB_HiddSec    = 0;
uint32_t BPB_TotSec32   = 0;

/* Extended BPB */
uint8_t  BS_DrvNum      = 0;
uint8_t  BS_Reserved1  = 0;
uint8_t  BS_BootSig    = 0x29;
uint32_t BS_VolID      = 0x575076C6;
char     BS_VolLab[11] = "NO NAME    ";
char     BS_FilSysType[8] = "FAT12   ";

/* FAT initial bytes */
uint8_t FAT_init[3] = { 0xF0, 0xFF, 0xFF };

/* ===== Write boot sector ===== */
static void write_boot_sector(FILE *f) {
    uint8_t sector[SECTOR_SIZE];
    memset(sector, 0, sizeof(sector));

    memcpy(sector + 0x00, BS_jmpBoot, 3);
    memcpy(sector + 0x03, BS_OEMName, 8);

    memcpy(sector + 0x0B, &BPB_BytsPerSec, 2);
    memcpy(sector + 0x0D, &BPB_SecPerClus, 1);
    memcpy(sector + 0x0E, &BPB_RsvdSecCnt, 2);
    memcpy(sector + 0x10, &BPB_NumFATs, 1);
    memcpy(sector + 0x11, &BPB_RootEntCnt, 2);
    memcpy(sector + 0x13, &BPB_TotSec16, 2);
    memcpy(sector + 0x15, &BPB_Media, 1);
    memcpy(sector + 0x16, &BPB_FATSz16, 2);
    memcpy(sector + 0x18, &BPB_SecPerTrk, 2);
    memcpy(sector + 0x1A, &BPB_NumHeads, 2);
    memcpy(sector + 0x1C, &BPB_HiddSec, 4);
    memcpy(sector + 0x20, &BPB_TotSec32, 4);

    memcpy(sector + 0x24, &BS_DrvNum, 1);
    memcpy(sector + 0x25, &BS_Reserved1, 1);
    memcpy(sector + 0x26, &BS_BootSig, 1);
    memcpy(sector + 0x27, &BS_VolID, 4);
    memcpy(sector + 0x2B, BS_VolLab, 11);
    memcpy(sector + 0x36, BS_FilSysType, 8);

    sector[510] = 0x55;
    sector[511] = 0xAA;

    fwrite(sector, SECTOR_SIZE, 1, f);
}

int main(int argc, char *argv[]) {
    FILE *f;
    uint8_t zero[SECTOR_SIZE];
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

    for (i = 0; i < DISK_SIZE_2880K / SECTOR_SIZE; i++)
        fwrite(zero, SECTOR_SIZE, 1, f);

    fclose(f);

    /* escrever boot + FATs */
    f = fopen(argv[1], "rb+");
    if (!f) {
        perror("fopen");
        return 1;
    }

    write_boot_sector(f);

    /* FAT 1 */
    fseek(f, SECTOR_SIZE, SEEK_SET);
    fwrite(FAT_init, sizeof(FAT_init), 1, f);

    /* FAT 2 */
    fseek(f, SECTOR_SIZE * (1 + BPB_FATSz16), SEEK_SET);
    fwrite(FAT_init, sizeof(FAT_init), 1, f);

    fclose(f);

    puts("Imagem FAT12 2.88MB criada com sucesso.");
    return 0;
}
