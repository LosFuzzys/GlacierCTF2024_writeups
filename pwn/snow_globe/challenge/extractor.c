#include "alloca.h"
#include <magick/MagickCore.h>

#include <magick/profile.h>
#include <magick/property.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "snow_globe_meta.h"


typedef enum {
  EXIT_OKAY   = 0,
  EXIT_ARG    = 1,
  EXIT_ALLOC  = 2,
  EXIT_LOAD   = 3,
  EXIT_OP     = 4,
} ExitCode;

static unsigned long IMAGE_SIZE;


void copy_escape_string(char* dest, const char* src, size_t size) {
  while (size--) {
    if (*src == '\n') {
      *(dest++) = '\\';
      *(dest++) = 'n';
    } else {
      *(dest++) = *(src++);
    }
  }
  *dest = '\0';
}

#define IIM_MAX_RECORD 9
#define IIM_MAX_DATASET 255
void parse_iptc_iim_data(const unsigned char* iptc, size_t size, FILE* meta) {
  // Goal, have record_dataset_envvars not being null-initialized, have user
  // controlled data pointer in record_dataset_envvars

  char* record_dataset_envvars[IIM_MAX_RECORD+1][IIM_MAX_DATASET+1];
  for (size_t i = 0; i < IIM_MAX_RECORD; i++)
    memset(record_dataset_envvars[i], '\0', sizeof(record_dataset_envvars[i]));

  // Fill up the environment vars
  #define X(env, icmp_rec, icmp_dataset, _, __) record_dataset_envvars[icmp_rec][icmp_dataset] = env;
  USE_GLOBE_METADATA
  #undef X

  const unsigned char *pos, *end;

  pos = iptc;
  end = pos + size;

  while (pos < end) {
    uint8_t tag_marker = *(pos++);
    uint8_t record = *(pos++);
    uint8_t dataset = *(pos++);
    uint16_t len = *(pos++) << 8;
    len |= *(pos++);

    if (tag_marker != '\x1c') {
      printf("ERROR: Unexpected misalignment from IIM records - cannot proceed!\n");
      return;
    }

    char terminated_data[2*len+1];
    copy_escape_string(terminated_data, (const char*)pos, len);

    const char* env_name = record_dataset_envvars[record][dataset];

    printf("Got dataset envvar: %d:%02d = %p\n", record, dataset, env_name);

    if (env_name != NULL) {
      printf("%s=%s\n", env_name, terminated_data);
      fprintf(meta, "%s=%s\n", env_name, terminated_data);
    } else {
      printf("WARN: Ignoring unknown dataset entry %hhu:%02hhu: %s\n", record, dataset, terminated_data);
    }

    pos += len;
  }
}

struct range{
  unsigned char* current_pos;
  unsigned char* next_pos;
  unsigned char* end;
};

bool validate_iptc(unsigned char* iptc, size_t iptc_size, const unsigned char** iptc_pos, size_t* iptc_len) {
  struct range c;

  #define ADVANCE_CHECK_RETURN(cnt) \
    c.current_pos = c.next_pos; \
    c.next_pos += cnt; \
    if (c.next_pos > c.end) { \
      printf("  failed!\n"); \
      return false; \
    }

  printf("Validating IPTC profile...\n");


  c.current_pos = c.next_pos = iptc;
  c.end = iptc + iptc_size;

  // For some reason, GIMP/ImageMagick/... adds a Image Resource Block:
  // https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_pgfId-1037685
  // At first, check for the 8BIM header
  // It has a size of at least 4 (signature) + 2 (ID) + 2 (empty string) + 4 (size)
  if (c.current_pos[0] != '\x1c') {
    printf(" - Checking IRB...\n");
    const char irb_iptc_signature[] = "8BIM\x04\x04";
    ADVANCE_CHECK_RETURN(sizeof(irb_iptc_signature) - 1);
    if (memcmp(irb_iptc_signature, c.current_pos, sizeof(irb_iptc_signature) - 1))
      return false;

    // Read the size of the Pascal-style string. It is one byte large and the str+len are padded by NULL.
    ADVANCE_CHECK_RETURN(1);
    uint8_t str_size = *c.current_pos;
    if (str_size % 2 == 0)
      str_size++;
    ADVANCE_CHECK_RETURN(str_size);

    // The payload size is contained in the next 4 bits
    ADVANCE_CHECK_RETURN(4);
    int64_t data_cnt = (c.current_pos[0] << 24UL) | (c.current_pos[1] << 16UL) | (c.current_pos[2] << 8UL) | c.current_pos[3];

    // Check the size of the payload
    if (c.end - c.next_pos != data_cnt)
      return false;

    *iptc_pos = c.next_pos;
    *iptc_len = data_cnt;
  } else {
    *iptc_pos = c.next_pos;
    *iptc_len = iptc_size;
  }

  // Validate size and record
  printf(" - Checking IPTC entries...\n");
  size_t current_max_record = 0;

  while (c.next_pos != c.end) {
    ADVANCE_CHECK_RETURN(5);
    // Check for the correct IPTC tag marker
    if (c.current_pos[0] != 0x1c) {
      printf("  Tag marker is not 0x1c (%hhx)", c.current_pos[0]);
      return false;
    }
    // Check if the records are in order. Datasets are not required to be in order
    if (c.current_pos[1] < current_max_record) {
      printf("  Record numbers not in increasing order!\n");
      return false;
    }
    if (c.current_pos[1] > IIM_MAX_RECORD) {
      printf("  Record number too large: %hhu\n", c.current_pos[1]);
      return false;
    }
    current_max_record = c.current_pos[1];

    uint16_t size = (c.current_pos[3] << 8) | c.current_pos[4];
    fprintf(stdout, "  - entry: marker=%hhx, dataset=%hhu:%02hhu, size=%hu, data=%.*s\n", c.current_pos[0], c.current_pos[1], c.current_pos[2], size, size, c.next_pos);
    ADVANCE_CHECK_RETURN(size);
  }

  printf("done!\n");
  return true;
}

bool extract_metadata(Image* img, FILE* meta) {
  const StringInfo* iptc = GetImageProfile(img, "iptc");

  if (!iptc || !iptc->datum) {
    printf("No IPTC data in file - nothing to do...\n");
    return true;
  }

  const unsigned char* iptc_pos;
  size_t iptc_len;
  if (!validate_iptc(iptc->datum, iptc->length, &iptc_pos, &iptc_len))
    return false;

  parse_iptc_iim_data(iptc_pos, iptc_len, meta);

  return true;
}

int main(int argc, char** argv) {
  ExitCode code;
  ImageInfo *info, *info_rgb;
  Image *img, *resized, *extended;
  FILE *in, *out, *meta;
  ExceptionInfo* err;

  code = EXIT_OKAY;

  if (argc != 5)
    return EXIT_ARG;

  char* endptr = NULL;
  IMAGE_SIZE = strtoul(argv[4], &endptr, 10);
  if (endptr == argv[3] || IMAGE_SIZE < 32 || IMAGE_SIZE > 16*1024) {
    printf("ERROR: Invalid image size: %s\n", argv[3]);
    return EXIT_ARG;
  }

  MagickCoreGenesis(argv[0], MagickFalse);

  // Open the required files
  in = fopen(argv[1], "rb");
  out = fopen(argv[2], "wb");
  meta = fopen(argv[3], "w");

  if (in == NULL || out == NULL || meta == NULL) {
    code = EXIT_LOAD;
    goto err_out;
  }

  // Create an image info for loading the image
  info = CloneImageInfo(NULL);
  if (!info) {
    code = EXIT_ALLOC;
    goto out_close_files;
  }
  // Create an image info for the animation output
  info_rgb = CloneImageInfo(NULL);
  if (!info_rgb) {
    code = EXIT_ALLOC;
    goto out_free_image_info;
  }

  err = AcquireExceptionInfo();
  if (!err) {
    code = EXIT_ALLOC;
    goto out_free_rgb_info;
  }

  // Load the image...
  printf("Reading file \"%s\"...\n", argv[1]);
  SetImageInfoFile(info, in);
  img = ReadImage(info, err);

  if (err->severity != UndefinedException) {
    CatchException(err);
    code = EXIT_LOAD;
    goto out_free_exception;
  }

  if (!extract_metadata(img, meta)) {
    printf("ERROR: Could not extract metadata!\n");
    code = EXIT_OP;
    goto out_free_exception;
  }

  // Resize the image to a maximum of 512x512 (keeping the aspect ratio)
  if (img->rows > IMAGE_SIZE || img->columns > IMAGE_SIZE) {
    size_t new_rows = img->rows;
    size_t new_cols = img->columns;

    if (img->rows == img->columns) {
      new_rows = new_cols = IMAGE_SIZE;
    } else if (img->rows > img->columns) {
      new_cols *= (((double)IMAGE_SIZE) / img->rows);
      new_rows = IMAGE_SIZE;
    } else {
      new_rows *= (((double)IMAGE_SIZE) / img->columns);
      new_cols = IMAGE_SIZE;
    }

    resized = ResizeImage(img, new_cols, new_rows, UndefinedFilter, 1.0, err);
    if (resized == NULL) {
      printf("ERROR: Failed to resize image!\n");
      goto out_free_image;
    }
  } else {
    resized = img;
    img = NULL;
    memset(resized->filename, '\0', sizeof(resized->filename));
  }

  if (resized->rows != resized->columns) {
    RectangleInfo rect = {
      .x = 0,
      .y = 0,
      .width = IMAGE_SIZE,
      .height = IMAGE_SIZE,
    };

    // https://github.com/ImageMagick/ImageMagick6/blob/6.9.13-16/magick/geometry.c#L1382
    GravityAdjustGeometry(resized->columns, resized->rows, CenterGravity, &rect);
    extended = ExtentImage(resized, &rect, err);
    if (extended == NULL) {
      printf("ERROR: Failed to extend image!\n");
      goto out_free_resized;
    }
  } else {
    extended = resized;
    resized = NULL;
    memset(extended->filename, '\0', sizeof(extended->filename));
  }

  fprintf(meta, "IMG_SIZE=%zu\n", extended->rows);

  // Write out the RGB file...
  printf("writing image file...\n");

  // Unfortunately, we have to use `SetImageInfo` to initialize the format of output file
  // There seems to be no other way to specify the output format we want if we use a FILE*
  // https://github.com/ImageMagick/ImageMagick6/blob/main/magick/image.c#L2719
  strncpy(info_rgb->filename, "test.rgb", sizeof(info_rgb->filename));
  SetImageInfo(info_rgb, 0, err);
  memset(info_rgb->filename, '\0', sizeof(info_rgb->filename));
  info_rgb->depth = 8;
  SetImageInfoFile(info_rgb, out);

  if (WriteImage(info_rgb, extended) == MagickFalse) {
    printf("ERROR: Failed to write image!\n");
    code = EXIT_OP;
  }

  if (extended)
    extended = DestroyImage(extended);
out_free_resized:
  if (resized)
    resized = DestroyImage(resized);
out_free_image:
  if (img)
    img = DestroyImage(img);
out_free_exception:
  err = DestroyExceptionInfo(err);
out_free_rgb_info:
  info_rgb = DestroyImageInfo(info_rgb);
out_free_image_info:
  info = DestroyImageInfo(info);
out_close_files:
  fclose(in);
  fclose(out);
  fclose(meta);

err_out:
  MagickCoreTerminus();
  return code;
}
