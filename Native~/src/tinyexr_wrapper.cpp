#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "tinyexr.h"

#if defined(_WIN32) && !defined(TINYEXR_UNITY_STATIC)
#define TINYEXR_UNITY_API __declspec(dllexport)
#else
#define TINYEXR_UNITY_API __attribute__((visibility("default")))
#endif

namespace {

char *DuplicateMessage(const char *message) {
  if (message == NULL) {
    return NULL;
  }

  const size_t length = std::strlen(message) + 1;
  char *copy = static_cast<char *>(std::malloc(length));
  if (copy == NULL) {
    return NULL;
  }

  std::memcpy(copy, message, length);
  return copy;
}

void SetError(const char **error_message, const char *message) {
  if (error_message == NULL) {
    return;
  }

  *error_message = DuplicateMessage(message);
}

uint32_t PackRgb9e5(float red, float green, float blue) {
  constexpr float kMaxValue = 65408.0f;
  constexpr int kMantissaBits = 9;
  constexpr int kExponentBias = 15;

  const auto sanitize = [=](float value) {
    if (std::isnan(value) || value <= 0.0f) {
      return 0.0f;
    }
    return std::min(value, kMaxValue);
  };

  red = sanitize(red);
  green = sanitize(green);
  blue = sanitize(blue);
  const float maximum = std::max(red, std::max(green, blue));
  if (maximum == 0.0f) {
    return 0;
  }

  int shared_exponent =
      std::max(-kExponentBias,
               std::min(kExponentBias + 1,
                        static_cast<int>(std::floor(std::log2(maximum))) + 1));
  float scale = std::ldexp(1.0f, shared_exponent - kMantissaBits);
  if (static_cast<int>(std::floor(maximum / scale + 0.5f)) ==
          (1 << kMantissaBits) &&
      shared_exponent < kExponentBias + 1) {
    ++shared_exponent;
    scale *= 2.0f;
  }

  const auto mantissa = [scale](float value) {
    return static_cast<uint32_t>(std::min(
        511, static_cast<int>(std::floor(value / scale + 0.5f))));
  };
  const uint32_t encoded_exponent =
      static_cast<uint32_t>(shared_exponent + kExponentBias);
  return mantissa(red) | (mantissa(green) << 9) | (mantissa(blue) << 18) |
         (encoded_exponent << 27);
}

}  // namespace

extern "C" {

TINYEXR_UNITY_API uint32_t tinyexr_pack_rgb9e5(float red, float green,
                                                float blue) {
  return PackRgb9e5(red, green, blue);
}

TINYEXR_UNITY_API float *tinyexr_load_rgba_from_memory(
    const unsigned char *memory, int size, int *width, int *height,
    const char **error_message) {
  if (error_message != NULL) {
    *error_message = NULL;
  }
  if (width != NULL) {
    *width = 0;
  }
  if (height != NULL) {
    *height = 0;
  }

  if (memory == NULL || size <= 0 || width == NULL || height == NULL) {
    SetError(error_message, "Invalid EXR input.");
    return NULL;
  }

  float *rgba = NULL;
  const char *tinyexr_error = NULL;
  const int result =
      LoadEXRFromMemory(&rgba, width, height, memory, static_cast<size_t>(size),
                        &tinyexr_error);

  if (result != TINYEXR_SUCCESS) {
    if (error_message != NULL) {
      if (tinyexr_error != NULL) {
        *error_message = tinyexr_error;
      } else {
        SetError(error_message, "TinyEXR failed without an error message.");
      }
    } else if (tinyexr_error != NULL) {
      FreeEXRErrorMessage(tinyexr_error);
    }

    if (rgba != NULL) {
      std::free(rgba);
    }

    if (width != NULL) {
      *width = 0;
    }
    if (height != NULL) {
      *height = 0;
    }
    return NULL;
  }

  if (tinyexr_error != NULL) {
    FreeEXRErrorMessage(tinyexr_error);
  }

  return rgba;
}

TINYEXR_UNITY_API uint32_t *tinyexr_load_rgb9e5_from_memory(
    const unsigned char *memory, int size, int *width, int *height,
    int flip_vertically, const char **error_message) {
  float *rgba = tinyexr_load_rgba_from_memory(memory, size, width, height,
                                               error_message);
  if (rgba == NULL) {
    return NULL;
  }

  const size_t pixel_count =
      static_cast<size_t>(*width) * static_cast<size_t>(*height);
  if (*width <= 0 || *height <= 0 ||
      pixel_count > SIZE_MAX / sizeof(uint32_t)) {
    std::free(rgba);
    SetError(error_message, "EXR dimensions are too large.");
    return NULL;
  }

  uint32_t *packed =
      static_cast<uint32_t *>(std::malloc(pixel_count * sizeof(uint32_t)));
  if (packed == NULL) {
    std::free(rgba);
    SetError(error_message, "Could not allocate the packed EXR image.");
    return NULL;
  }

  for (int destination_y = 0; destination_y < *height; ++destination_y) {
    const int source_y =
        flip_vertically ? *height - 1 - destination_y : destination_y;
    for (int x = 0; x < *width; ++x) {
      const size_t source =
          (static_cast<size_t>(source_y) * *width + x) * 4;
      const size_t destination =
          static_cast<size_t>(destination_y) * *width + x;
      packed[destination] =
          PackRgb9e5(rgba[source], rgba[source + 1], rgba[source + 2]);
    }
  }

  std::free(rgba);
  return packed;
}

TINYEXR_UNITY_API int tinyexr_is_exr_from_memory(const unsigned char *memory,
                                                 int size) {
  if (memory == NULL || size <= 0) {
    return 0;
  }

  return IsEXRFromMemory(memory, static_cast<size_t>(size)) == TINYEXR_SUCCESS
             ? 1
             : 0;
}

TINYEXR_UNITY_API void tinyexr_free(void *ptr) {
  if (ptr != NULL) {
    std::free(ptr);
  }
}

TINYEXR_UNITY_API void tinyexr_free_error(const char *message) {
  FreeEXRErrorMessage(message);
}

}  // extern "C"
