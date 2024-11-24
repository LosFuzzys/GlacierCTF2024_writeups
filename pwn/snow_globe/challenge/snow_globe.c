#include "snow_globe_meta.h"
#include "stdlib.h"
#include <magick/MagickCore.h>

#include <limits.h>
#include <magick/draw.h>
#include <magick/image.h>
#include <magick/profile.h>
#include <magick/string_.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define NUM_FLAKES 40
#define NUM_FRAMES 60

typedef enum {
  EXIT_OKAY   = 0,
  EXIT_ARG    = 1,
  EXIT_ALLOC  = 2,
  EXIT_LOAD   = 3,
  EXIT_OP     = 4,
} ExitCode;


static unsigned long IMAGE_SIZE;

static ImageInfo* globe_info = NULL;
static Image* globe_overlay = NULL;


struct Snowflake {
  uint16_t base_x;
  float amplitude;
  float freq;

  float y_speed;

  uint16_t diameter;
};

#define randf(max) ((max) * ((float)rand())/((float)RAND_MAX));

void random_flake(struct Snowflake* flake) {
  flake->y_speed = 1.0 + randf(0.05*IMAGE_SIZE);
  flake->diameter = 1.0 + randf(10.0);
  flake->amplitude = randf(IMAGE_SIZE*0.4);
  flake->freq = randf(10.0);
  flake->base_x = (uint16_t)randf((float)IMAGE_SIZE);
}

bool draw_snowflake(Image* frame, struct Snowflake* flake, size_t i) {
  float x, y;
  char buf[256];

  y = i * flake->y_speed;
  if (y >= IMAGE_SIZE)
    return true;

  x = flake->base_x + sin((i * flake->freq)/NUM_FRAMES) * flake->amplitude;
  if (x < 0 || x >= IMAGE_SIZE)
    return true;


  DrawInfo* draw = AcquireDrawInfo();

  snprintf(buf, sizeof(buf), "fill #fffefe\n ellipse %1$u,%2$u %3$hu,%3$hu 0,360",
      (unsigned)x, (unsigned)y, flake->diameter);
  draw->primitive = ConstantString(buf);

  bool okay = DrawImage(frame, draw);

  if (!okay) {
    printf("Draw Failed!\n");
  }

  DestroyDrawInfo(draw);

  return true;
}
bool overlay_globe_frame(Image** frame, ExceptionInfo* err) {
  // Append the globe frame
  AppendImageToList(frame, globe_overlay);

  // Merge them together. That creates a new image:
  // https://github.com/ImageMagick/ImageMagick6/blob/6.9.13-16/magick/layer.c#L2076
  Image* composed = MergeImageLayers(*frame, FlattenLayer, err);

  // Remove globe again from the list
  if (globe_overlay != RemoveLastImageFromList(frame)) {
    printf("ERROR: Expected removed image to be globe overlay!\n");
    abort();
  }

  // Handle a potential merging error
  if (err->severity != UndefinedException) {
    return false;
  }

  // Destroy the existing frame, move composed frame in its place
  DestroyImage(*frame);

  *frame = composed;

  return true;
}
void set_anim_params(Image* frame) {
    frame->delay = 50;
    frame->ticks_per_second = 1000;
}

bool create_animation(Image** anim, const Image* base_img, ExceptionInfo* err) {
  struct Snowflake flakes[NUM_FLAKES];

  for (size_t i = 0; i < NUM_FLAKES; i++)
    random_flake(&flakes[i]);

  for (size_t i = 0; i < NUM_FRAMES; i++) {
    printf("  Frame %zu...\n", i);
    Image* copy = CloneImage(base_img, 0, 0, MagickTrue, err);
    if (!copy || err->severity != UndefinedException) {
      CatchException(err);
      printf("Failed to clone image!\n");
      return false;
    }

    for (size_t flake = 0; flake < NUM_FLAKES; flake++) {
      if (!draw_snowflake(copy, &flakes[flake], i)) {
        printf("Failed to overlay snow!\n");
        return false;
      }
    }


    if (!overlay_globe_frame(&copy, err)) {
      CatchException(err);
      printf("Failed to overlay globe frame!\n");
      return false;
    }

    set_anim_params(copy);
    AppendImageToList(anim, copy);
  }

  return true;
}


bool prepare_overlays(char* binpath, ExceptionInfo* err) {
  char overlay_path[PATH_MAX];

  // Build together globe overlay path relatively
  if (realpath(binpath, overlay_path) == NULL)
    abort();
  char* pos_last_slash = strrchr(overlay_path, '/');
  *(pos_last_slash+1) = '\0';
  strncpy(pos_last_slash+1, "globe.png", sizeof(overlay_path) - (pos_last_slash+1 - overlay_path));

  printf("Reading overlay \"%s\"...\n", overlay_path);


  globe_info = CloneImageInfo(NULL);
  if (!globe_info) {
    return false;
  }

  strncpy(globe_info->filename, overlay_path, sizeof(globe_info->filename));
  Image* img = ReadImage(globe_info, err);

  if (err->severity != UndefinedException) {
    CatchException(err);
    goto err_destroy_info;
  }

  globe_overlay = ResizeImage(img, IMAGE_SIZE, IMAGE_SIZE, UndefinedFilter, 1.0, err);
  if (err->severity != UndefinedException) {
    CatchException(err);
    goto err_destroy_img;
  }

  DestroyImage(img);
  return true;

err_destroy_img:
  DestroyImage(img);
err_destroy_info:
    globe_info = DestroyImageInfo(globe_info);

  return false;
}

__attribute__((format(printf, 4, 5)))
bool asnprintf(char* buf, size_t size, size_t* i, const char* fmt, ...) {
  bool out = false;

  va_list args;
  va_start(args, fmt);

  size_t remaining = size - *i;

  int cnt = vsnprintf(buf + *i, remaining, fmt, args);
  if (cnt >= 0) {
    if (cnt < remaining) {
      *i += cnt;
      out = true;
    }
  }

  va_end(args);

  return out;
}
#define CHK_RET(fmt, ...) \
  if (!asnprintf(buf, XMP_MAX, &i, fmt, ##__VA_ARGS__)) \
    goto out

char* create_xmp_metadata() {
  const char* env;
  bool okay = false;
  const size_t XMP_MAX = 32*1024;
  char* buf = malloc(XMP_MAX);
  memset(buf, '\0', XMP_MAX);

  size_t i = 0;

  // As a reference
  // XMP specs: https://github.com/adobe/XMP-Toolkit-SDK/blob/9f779e8/docs/XMPSpecificationPart1.pdf
  // As a reference from an image with XMP data embedded: `exiftool -b -xmp <file>`
  // Nobody got 173 Francs to spare for a goofy XMP writer.
  // The code does very likely **not** produce compliant XML or XMP, but it's gotta be good enough for this case.
  // Adding another dependency to libxml when ImageMagick is already
  // terribly big wouldn't be nice.
  // Of course, for productive code, this is a bad idea.
  // But after all, if the created XML is wrong, it ought to not affect snow_globe,
  // but rather the user's viewer (which is the attacker), so ¯\_(ツ)_/¯

  CHK_RET("<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.1.2\">");
  CHK_RET("<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">");
  CHK_RET("<rdf:Description ");
  CHK_RET("xmlns:dc=\"http://purl.org/dc/elements/1.1/\" ");
  CHK_RET("xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\" ");
  CHK_RET("xmlns:photoshop=\"http://ns.adobe.com/photoshop/1.0/\" ");
  CHK_RET("xmlns:Iptc4xmpCore=\"http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/\" ");
  CHK_RET("xmlns:Iptc4xmpExt=\"http://iptc.org/std/Iptc4xmpExt/2008-02-29/\" ");

  // Write all env vars that are part of the description
  #define X(var, _, __, part_of_desk, fmt) \
    if (part_of_desk && (env = getenv(var)) != NULL) { \
      CHK_RET(fmt, env); \
    }
  USE_GLOBE_METADATA
  #undef X

  CHK_RET(">");


  #define X(var, _, __, part_of_desk, fmt) \
    if (!part_of_desk && (env = getenv(var)) != NULL) { \
      CHK_RET(fmt, env); \
    }
  USE_GLOBE_METADATA
  #undef X

  CHK_RET("</rdf:Description>");
  CHK_RET("</rdf:RDF>");
  CHK_RET("</x:xmpmeta>");

  okay = true;

out:
  if (!okay) {
    free(buf);
    buf = NULL;
  }

  return buf;
}
#undef CHK_RET


int main(int argc, char** argv) {
  ExitCode code;
  ImageInfo *info, *info_anim;
  Image *img, *anim;
  ExceptionInfo* err;

  code = EXIT_OKAY;

  srand(time(NULL));

  if (argc != 3) {
    printf("Invalid number of argument!\n");
    return EXIT_ARG;
  }

  // Prepare the output FILE*
  FILE* out_file = NULL;
  if (strncmp("fd:", argv[2], 3) == 0) {
    int fd;
    if (sscanf(argv[2], "fd:%d", &fd) == 1)
      out_file = fdopen(fd, "wb");
  } else {
    out_file = fopen(argv[2], "wb");
  }
  if (out_file == NULL) {
    printf("Could not open %s!\n", argv[2]);
    return EXIT_ARG;
  }

  char* endptr = NULL;
  const char* size_env = getenv("IMG_SIZE");
  if (size_env == NULL) {
    printf("ERROR: Environment variable 'IMG_SIZE' is not set!\n");
    return EXIT_ARG;
  }
  IMAGE_SIZE = strtoul(size_env, &endptr, 10);
  if (endptr == size_env || IMAGE_SIZE < 32 || IMAGE_SIZE > 16*1024) {
    printf("ERROR: Invalid image size: %s\n", size_env);
    return EXIT_ARG;
  }

  MagickCoreGenesis(argv[0], MagickFalse);

  // Create an image info for loading the image
  info = CloneImageInfo(NULL);
  if (!info) {
    code = EXIT_ALLOC;
    goto err_out;
  }
  // Create an image info for the animation output
  info_anim = CloneImageInfo(NULL);
  if (!info_anim) {
    code = EXIT_ALLOC;
    goto out_free_image_info;
  }
  // Create an image list that holds the animation frames
  anim = NewImageList();
  // Funnily enough, NewImageList seems to return NULL:
  // https://github.com/ImageMagick/ImageMagick6/blob/6.9.13-16/magick/list.c#L962
  // Thus the NULL check is absolute moot
  /*
  if (!anim) {
    code = EXIT_ALLOC;
    goto out_free_anim_info;
  }
  */

  err = AcquireExceptionInfo();
  if (!err) {
    printf("Could not allocate exception info!\n");
    code = EXIT_ALLOC;
    goto out_free_anim;
  }

  printf("Reading overlay...\n");
  if (!prepare_overlays(argv[0], err)) {
    printf("Could not read overlay!\n");
    code = EXIT_ALLOC;
    goto out_free_exception;
  }

  // Load the image...
  char sizebuf[64];
  printf("Reading file \"%s\"...\n", argv[1]);
  strncpy(info->filename, argv[1], sizeof(info->filename)-1);
  snprintf(sizebuf, sizeof(sizebuf), "%lux%lu", IMAGE_SIZE, IMAGE_SIZE);

  info->filename[sizeof(info->filename)-1] = '\0';
  info->size = ConstantString(sizebuf);
  info->depth = 8;
  img = ReadImage(info, err);

  if (err->severity != UndefinedException) {
    CatchException(err);
    code = EXIT_LOAD;
    goto out_free_overlays;
  }

  printf("Creating animation...\n");
  if (!create_animation(&anim, img, err)) {
    printf("Failed to create animation!\n");
    code = EXIT_OP;
    goto out_free_image;
  }

  // Building XMP metadata
  printf("Building XMP metadata...\n");
  char* xmp_meta = create_xmp_metadata();
  if (xmp_meta) {
    bool success = SetImageProfile(anim, "xmp", StringToStringInfo(xmp_meta));
    // The data is copied:
    // https://github.com/ImageMagick/ImageMagick6/blob/6.9.13-16/magick/string.c#L2485
    free(xmp_meta);

    if (!success) {
      printf("Failed to set XMP profile!\n");
    }
  } else {
    printf("Failed to create XMP metadata!\n");
  }

  // Write out the animation
  printf("writing animation file...\n");

  // Unfortunately, we have to use `SetImageInfo` to initialize the format of output file
  // There seems to be no other way to specify the output format we want if we use a FILE*
  // https://github.com/ImageMagick/ImageMagick6/blob/main/magick/image.c#L2719
  strncpy(info_anim->filename, "test.webp", sizeof(info_anim->filename));
  SetImageInfo(info_anim, 0, err);
  memset(info_anim->filename, '\0', sizeof(info_anim->filename));
  SetImageInfoFile(info_anim, out_file);
  info_anim->adjoin = MagickTrue;

  if (WriteImage(info_anim, anim) == MagickFalse) {
    printf("ERROR: Failed to write image!\n");
  }


out_free_overlays:
  DestroyImage(globe_overlay);
  DestroyImageInfo(globe_info);
out_free_image:
  DestroyImage(img);
out_free_exception:
  err = DestroyExceptionInfo(err);
out_free_anim:
  anim = DestroyImageList(anim);
//out_free_anim_info:
  info_anim = DestroyImageInfo(info_anim);
out_free_image_info:
  info = DestroyImageInfo(info);

err_out:
  MagickCoreTerminus();
  return code;
}
