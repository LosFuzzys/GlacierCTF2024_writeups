#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define CBC 1
#define CTR 1
#define ECB 1

#define AES256 1

uint32_t seed = 0;

uint32_t custom_rand()
{
    uint32_t n = seed * 0x343fd + 0x269ec3;
    seed = n & 0xffffffff;
    return (n >> 16) & 0x7fff;
}

#include "aes.hpp"

char* getRandomString(int len)
{
    char* str = (char*)malloc(len + 1);
    int i;
    for (i = 0; i < len; i++)
    {
        str[i] = ' ' + custom_rand() % ('~' - ' ');
    }
    str[len] = '\0';
    return str;
}

uint32_t hash(uint32_t initial_hash, char* str)
{
    uint32_t hash = 5381 + initial_hash;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    char* input_file_name = argv[1];

    printf("Input file: %s\n", input_file_name);

    char* output_file_name = (char*)malloc(strlen(input_file_name));
    strncpy(output_file_name, input_file_name, strlen(input_file_name) - 4);
    strcat(output_file_name, ".dec");

    printf("Output file: %s\n", output_file_name);

    char org_input_file_name[] = "flag.txt";//; (char*)malloc(strlen(input_file_name));
    //memset(org_input_file_name,0, strlen(input_file_name));
    //strncpy(org_input_file_name, input_file_name, strlen(input_file_name));

    printf("Original input file: %s\n", org_input_file_name);

    FILE* input_file = fopen(input_file_name, "rb");
    FILE* output_file = fopen(output_file_name, "wb");

    if (!input_file || !output_file)
    {
        printf("Error opening files");
        return 1;
    }

    fseek(input_file, 0, SEEK_END);
    // always needs to be 64 bytes
    long input_data_size = ftell(input_file);
    printf("Input data size: %ld\n", input_data_size);
    fseek(input_file, 0, SEEK_SET);
    char* input_data = (char*)malloc(input_data_size);
    fread(input_data, 1, input_data_size, input_file);

    char data[16 * 4 + 1];

    unsigned int org_t;
    unsigned char org_t_parts[4];

    // we have 4 rounds
    for (int i = 0; i < 4; i++)
    {
        unsigned char* ptr = (unsigned char*)input_data + i * 20;

        uint32_t target_hash = *(uint32_t*)(ptr);
        printf("Round hash: %u\n", target_hash);
        for (int k = 0; k < 255; k++)
        {
            uint32_t test_hash = hash(k, org_input_file_name);
            if (test_hash == target_hash)
            {
                printf("Found valid k: %d\n", k);

                org_t_parts[i] = k;
                break;
            }
        }
        // get the initial value of the hash

        for (int j = 0; j < 16; j++)
        {
            printf("%02x ", ptr[j + 4]);
            data[i * 16 + j] = ptr[j + 4];
        }
        printf("\n");
    }

    org_t = *(unsigned int*)org_t_parts;

    printf("Restored time: %ld\n", org_t);

    seed = org_t;

    // AES256 CBC
    char* key = getRandomString(32);
    printf("Key: %s\n", key);
    char* iv = getRandomString(16);
    printf("IV: %s\n", iv);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, (const uint8_t*)key, (const uint8_t*)iv);

    AES_CBC_decrypt_buffer(&ctx, (uint8_t*)data, 64);

    fwrite(data, 1, 64, output_file);

    fclose(input_file);
    fclose(output_file);

    return 0;
}