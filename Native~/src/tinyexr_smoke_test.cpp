#include <cassert>
#include <cstdlib>

extern "C" {
float *tinyexr_load_rgba_from_memory(const unsigned char *memory, int size,
                                     int *width, int *height,
                                     const char **error_message);
int tinyexr_is_exr_from_memory(const unsigned char *memory, int size);
void tinyexr_free_error(const char *message);
}

int main() {
  const unsigned char invalid_data[] = {'N', 'O', 'T', 'E', 'X', 'R'};
  assert(tinyexr_is_exr_from_memory(invalid_data,
                                    sizeof(invalid_data)) == 0);

  int width = -1;
  int height = -1;
  const char *error_message = nullptr;
  float *pixels = tinyexr_load_rgba_from_memory(
      invalid_data, sizeof(invalid_data), &width, &height, &error_message);

  assert(pixels == nullptr);
  assert(width == 0);
  assert(height == 0);
  assert(error_message != nullptr);
  tinyexr_free_error(error_message);
  return EXIT_SUCCESS;
}
