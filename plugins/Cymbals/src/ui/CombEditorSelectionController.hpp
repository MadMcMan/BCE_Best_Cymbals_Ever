#ifndef COMB_EDITOR_SELECTION_CONTROLLER_HPP
#define COMB_EDITOR_SELECTION_CONTROLLER_HPP

#include "UIXYPad.hpp"
#include <cstdio>

START_NAMESPACE_DISTRHO

struct CombEditorSelectionController {
    static int maskFromXy(lv_obj_t* xyPad) {
        return xyPad ? UIXYPad::getSelectedMask(xyPad) : 0;
    }

    static int firstSelectedLocal(int mask) {
        for (int i = 0; i < 4; ++i)
            if (mask & (1 << i)) return i;
        return -1;
    }

    static void setTitleFromMask(lv_obj_t* title, int mask, int baseOneBased) {
        if (!title) return;
        if (mask == 0) {
            lv_label_set_text(title, "FILTER");
            return;
        }

        int count = 0;
        for (int i = 0; i < 4; ++i)
            if (mask & (1 << i)) ++count;

        char buf[64];
        if (count == 1) {
            const int local = firstSelectedLocal(mask);
            snprintf(buf, sizeof(buf), "FILTER %d", baseOneBased + local);
        } else {
            int off = snprintf(buf, sizeof(buf), "FILTERS ");
            bool first = true;
            for (int i = 0; i < 4 && off < (int)sizeof(buf) - 1; ++i) {
                if ((mask & (1 << i)) == 0) continue;
                off += snprintf(buf + off, sizeof(buf) - off, "%s%d", first ? "" : "+", baseOneBased + i);
                first = false;
            }
        }
        lv_label_set_text(title, buf);
    }

    static int syncPanelVisibilityAndTitle(lv_obj_t* panel, lv_obj_t* title, lv_obj_t* xyPad, int baseOneBased) {
        const int mask = maskFromXy(xyPad);
        if (panel) {
            if (mask == 0) lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
        }
        setTitleFromMask(title, mask, baseOneBased);
        return mask;
    }
};

END_NAMESPACE_DISTRHO

#endif // COMB_EDITOR_SELECTION_CONTROLLER_HPP
