#include <aurora_app.h>

int main(void) {
  int x = 200, y = 120;

  while (app_loop()) {
    uint32_t held = hid_keys_held();
    int cx, cy;

    if (held & KEY_LEFT)
      x -= 2;
    if (held & KEY_RIGHT)
      x += 2;
    if (held & KEY_UP)
      y -= 2;
    if (held & KEY_DOWN)
      y += 2;
    hid_cpad(&cx, &cy);
    x += cx / 32;
    y -= cy / 32;
    if (hid_keys_down() & KEY_START)
      break;

    gfx_clear(SCREEN_TOP, RGB(16, 18, 28));
    gfx_print(SCREEN_TOP, 12, 10, FONT_TITLE, COLOR_WHITE, "Lunix");
    gfx_circle(SCREEN_TOP, x, y, 12, COLOR_AURORA);

    gfx_clear(SCREEN_BOTTOM, RGB(28, 28, 34));
    gfx_print(SCREEN_BOTTOM, 12, 10, FONT_REGULAR, COLOR_WHITE,
              "D-pad or circle pad: move\nTouch: draw a dot\nSTART: quit");
    if (hid_touch_held())
      gfx_circle(SCREEN_BOTTOM, hid_touch_x(), hid_touch_y(), 6, COLOR_ORANGE);
  }

  return 0;
}
