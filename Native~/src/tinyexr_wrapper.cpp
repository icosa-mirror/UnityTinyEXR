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

}  // namespace

extern "C" {

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
