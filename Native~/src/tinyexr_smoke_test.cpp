#include <cassert>
#include <cstdint>
#include <cstdlib>

extern "C" {
float *tinyexr_load_rgba_from_memory(const unsigned char *memory, int size,
                                     int *width, int *height,
                                     const char **error_message);
uint32_t *tinyexr_load_rgb9e5_from_memory(
    const unsigned char *memory, int size, int *width, int *height,
    int flip_vertically, const char **error_message);
uint32_t tinyexr_pack_rgb9e5(float red, float green, float blue);
int tinyexr_is_exr_from_memory(const unsigned char *memory, int size);
void tinyexr_free_error(const char *message);
}

int main() {
  assert(tinyexr_pack_rgb9e5(0.0f, 0.0f, 0.0f) == 0u);
  assert(tinyexr_pack_rgb9e5(1.0f, 1.0f, 1.0f) ==
         (256u | (256u << 9) | (256u << 18) | (16u << 27)));
  assert(tinyexr_pack_rgb9e5(65408.0f, 65408.0f, 65408.0f) == 0xffffffffu);

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

  error_message = nullptr;
  uint32_t *packed = tinyexr_load_rgb9e5_from_memory(
      invalid_data, sizeof(invalid_data), &width, &height, 1, &error_message);
  assert(packed == nullptr);
  assert(width == 0);
  assert(height == 0);
  assert(error_message != nullptr);
  tinyexr_free_error(error_message);
  return EXIT_SUCCESS;
}
