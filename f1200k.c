
// gcc -Wall -O2 -o f12M f12M.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SECTOR_SIZE 512
#define DISK_SIZE   (1200 * 1024)   // 1.2MB = 1,228,800 bytes
#define TOTAL_SECTORS (DISK_SIZE / SECTOR_SIZE)

/* ---------------- FAT BPB ---------------- */

#pragma pack(push,1)
typedef struct {
    uint8_t  jmp[3];
    char     oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fats;
    uint16_t root_entries;
    uint16_t total_sectors16;
    uint8_t  media;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors32;

    /* Extended BPB */
    uint8_t  drive;
    uint8_t  reserved;
    uint8_t  boot_sig;
    uint32_t volume_id;
    char     label[11];
    char     fs[8];
} BPB;
#pragma pack(pop)

/* ---------------- BOOT CODE (simples) ---------------- */
static const uint8_t boot_code[] = {
    0xFA, 0xEB, 0xFE  // cli ; jmp $
};

int main(int argc, char *argv[]) {
    FILE *f;
    uint8_t zero[SECTOR_SIZE];
    BPB bpb;

    if (argc != 2) {
        printf("uso: %s imagem.img\n", argv[0]);
        return 1;
    }

    memset(zero, 0, sizeof(zero));

    /* -------- criar imagem vazia -------- */
    f = fopen(argv[1], "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    for (int i = 0; i < TOTAL_SECTORS; i++)
        fwrite(zero, SECTOR_SIZE, 1, f);

    fclose(f);

    /* -------- preencher BPB -------- */
    memset(&bpb, 0, sizeof(bpb));

    bpb.jmp[0] = 0xEB;
    bpb.jmp[1] = 0x3C;
    bpb.jmp[2] = 0x90;

    memcpy(bpb.oem, "mkfsFAT", 7);

    bpb.bytes_per_sector    = 512;
    bpb.sectors_per_cluster = 1;
    bpb.reserved_sectors    = 1;
    bpb.fats                = 2;
    bpb.root_entries        = 224;
    bpb.total_sectors16     = TOTAL_SECTORS;
    bpb.media               = 0xF9;      // 1.2MB
    bpb.sectors_per_fat     = 9;
    bpb.sectors_per_track   = 15;
    bpb.heads               = 2;
    bpb.hidden_sectors      = 0;

    bpb.drive    = 0x00;
    bpb.boot_sig = 0x29;
    bpb.volume_id = 0x12345678;

    memcpy(bpb.label, "NO NAME    ", 11);
    memcpy(bpb.fs, "FAT12   ", 8);

    /* -------- escrever boot sector -------- */
    f = fopen(argv[1], "rb+");
    if (!f) {
        perror("fopen");
        return 1;
    }

    fwrite(&bpb, sizeof(bpb), 1, f);
    fwrite(boot_code, sizeof(boot_code), 1, f);

    fseek(f, 510, SEEK_SET);
    fputc(0x55, f);
    fputc(0xAA, f);

    /* -------- FAT inicial -------- */
    fseek(f, SECTOR_SIZE, SEEK_SET);
    fputc(0xF9, f);
    fputc(0xFF, f);
    fputc(0xFF, f);

    fclose(f);

    puts("Imagem FAT12 1.2MB criada com sucesso.");
    return 0;
}
