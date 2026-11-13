#include <aurora_app.h>

#include "Lunix.h"

int main(void) {
  uint32_t Entry = 0;

  mmu_enter();

  if (LunixLoadProgram("/hello.elf", &Entry) != LUNIX_OK) {
    gfx_print(SCREEN_TOP, 12, 10, FONT_TITLE, COLOR_WHITE,
              "ELF loading failed");
  }

  while (app_loop()) {
    gfx_print(SCREEN_TOP, 12, 10, FONT_TITLE, COLOR_WHITE,
              "ELF loaded successfully");

    if (hid_keys_down() & KEY_START) {
      break;
    }
  }

  mmu_leave();
  return 0;
}
