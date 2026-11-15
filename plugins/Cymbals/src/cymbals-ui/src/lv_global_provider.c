#include "lvgl.h"
#if LV_ENABLE_GLOBAL_CUSTOM
#include "src/core/lv_global.h"

// Weak fallback for library consumers that don't provide their own
// lv_global_default (e.g. standalone cymbals-ui users). When linked against
// dpf-widgets/DGL which provides a strong thread_local implementation, that
// strong symbol wins over this weak one. The headless test links this file
// directly with -DCYMBALS_UI_STRONG_GLOBAL so it provides a strong symbol
// that satisfies liblvgl.a regardless of static link order.
#if !defined(CYMBALS_UI_STRONG_GLOBAL) && (defined(__GNUC__) || defined(__clang__))
__attribute__((weak))
#endif
lv_global_t* lv_global_default(void) {
    static lv_global_t g;
    return &g;
}
#endif
