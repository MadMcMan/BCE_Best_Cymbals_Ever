#ifndef CYMBALS_SLOT_ORDER_POLICY_HPP
#define CYMBALS_SLOT_ORDER_POLICY_HPP

START_NAMESPACE_DISTRHO

namespace SlotOrderPolicy {

static constexpr int kSlotCount = 6;
static constexpr int kMissingSlot = -1;

// Canonical slot-order reconciliation policy:
// keep the first valid occurrence of each slot in the supplied order,
// then append unseen slots in ascending slot index order.
inline void normalizeSlotOrder(const int* candidateOrder, int* normalizedOrder) noexcept {
    bool seen[kSlotCount] = {false, false, false, false, false, false};
    int out = 0;

    for (int i = 0; i < kSlotCount && out < kSlotCount; ++i) {
        const int slot = candidateOrder[i];
        if (slot < 0 || slot >= kSlotCount) continue;
        if (seen[slot]) continue;
        seen[slot] = true;
        normalizedOrder[out++] = slot;
    }

    for (int slot = 0; slot < kSlotCount && out < kSlotCount; ++slot) {
        if (seen[slot]) continue;
        normalizedOrder[out++] = slot;
    }
}

inline void normalizeSlotOrder(int (&order)[kSlotCount]) noexcept {
    int normalized[kSlotCount] = {0, 1, 2, 3, 4, 5};
    normalizeSlotOrder(order, normalized);
    for (int i = 0; i < kSlotCount; ++i) {
        order[i] = normalized[i];
    }
}

} // namespace SlotOrderPolicy

END_NAMESPACE_DISTRHO

#endif // CYMBALS_SLOT_ORDER_POLICY_HPP