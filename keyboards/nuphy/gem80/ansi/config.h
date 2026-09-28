// gem80/ansi has one key LED fewer than iso (88 vs 89); the shared parent
// gem80/config.h sets the iso count
#ifdef RGB_MATRIX_LED_COUNT
#    undef RGB_MATRIX_LED_COUNT
#endif
#define RGB_MATRIX_LED_COUNT 88
