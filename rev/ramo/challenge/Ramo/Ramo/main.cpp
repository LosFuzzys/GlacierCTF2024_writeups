#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CBC 1
#define CTR 1
#define ECB 1

#define AES256 1

#include "aes.hpp"

uint32_t seed = 0;

uint32_t custom_rand()
{
    uint64_t n = seed * 0x343fd + 0x269ec3;
    seed = n & 0xffffffff;
    return (n >> 16) & 0x7fff;
}


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

uint32_t hash(uint32_t initial_hash, unsigned char* str)
{
    uint32_t hash = 5381 + initial_hash;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int main(int argc, char** argv)
{
    unsigned int t = time(NULL);

    if (argc < 2)
    {
        return 1;
    }

    const char* input_file_name = argv[1];

    char* output_file_name = (char*)malloc(strlen(input_file_name) + 5);
    strcpy_s(output_file_name, strlen(input_file_name) + 5, input_file_name);
    strcat_s(output_file_name, strlen(input_file_name) + 5, ".enc");

    FILE* input_file;
    fopen_s(&input_file, input_file_name, "rb");
    FILE* output_file;
    fopen_s(&output_file, output_file_name, "wb");

    if (!input_file || !output_file)
    {
        return 1;
    }

    fseek(input_file, 0, SEEK_END);
    // always needs to be 64 bytes
    long input_data_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    unsigned char* input_data = (unsigned char*)malloc(input_data_size);
    fread(input_data, 1, input_data_size, input_file);

    t ^= hash(input_data_size, input_data);
#if DEBUG
    printf("Time: %ld\n", t);
#endif
    seed = t;

    // AES256 CBC
    char* key = getRandomString(32);
    char* iv = getRandomString(16);
#if DEBUG
    printf("Key: %s\n", key);
    printf("IV: %s\n", iv);
#endif

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, (const uint8_t*)key, (const uint8_t*)iv);

    AES_CBC_encrypt_buffer(&ctx, input_data, input_data_size);

    // 64 / 16 = 4
    for (int i = 0; i < input_data_size / 16; i++)
    {
        // t is 8 bits, so shift up to 2 * 4 = 8 bits
        unsigned char* time_value_ptr = (unsigned char*)&t;

        uint32_t hash_value = hash(time_value_ptr[i], (unsigned char*)input_file_name);
#if DEBUG
        printf("Using k: %d\n", time_value_ptr[i]);
        printf("Round hash: %u\n", hash_value);
#endif
        unsigned char* hash_value_ptr = (unsigned char*)&hash_value;
        for (int k = 0; k < 4; k++)
        {
#if DEBUG
            printf("%d - %02x\n", k, hash_value_ptr[k]);
#endif
            fputc(hash_value_ptr[k], output_file);
        }

        for (int j = 0; j < 16; j++)
        {
            fputc(input_data[i * 16 + j], output_file);
#if DEBUG
            printf("%02x ", input_data[i * 16 + j]);
#endif
        }
#if DEBUG
        printf("\n");
#endif
    }

    fclose(input_file);
    fclose(output_file);

    return 0;
}