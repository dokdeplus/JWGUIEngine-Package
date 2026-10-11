#pragma once

#include "Export.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace jwgui {

class UIRenderView;

using TimerId = uint64_t;

// One-shot and repeating callbacks on the UI thread, kept in one list so the
// host needs a single OS timer: Application arms it for NextDueMs() and calls
// RunDue when it fires. No platform code, so it is tested with a fake clock.
class JWGUI_API TimerQueue {
public:
    // Milliseconds of the wall clock (time of day); aligned timers use it so
    // clocks change on the second. Replaceable for tests.
    using WallClock = std::function<double()>;

    TimerQueue();

    // dueMs and the times below are on the caller's monotonic clock.
    // intervalMs 0 = once. alignToWall: each run is moved to the next
    // multiple of intervalMs on the wall clock (12:00:01.000, 12:00:02.000, ...).
    // A timer with an owner is dropped (without running) once the owner is gone.
    TimerId Add(double nowMs, unsigned int delayMs, unsigned int intervalMs, std::function<void()> fn,
                const std::shared_ptr<UIRenderView>& owner = nullptr, bool alignToWall = false);
    bool Remove(TimerId id);
    void Clear();

    bool Empty() const { return m_timers.empty(); }
    size_t Count() const { return m_timers.size(); }
    // The earliest due time, or a negative value when there is nothing to run.
    double NextDueMs() const;

    // Runs every timer due at nowMs (each at most once per call). Callbacks
    // may add or remove timers, including their own.
    void RunDue(double nowMs);

    void SetWallClock(WallClock clock) { m_wallClock = std::move(clock); }
    // Delay from now to the next multiple of intervalMs on the wall clock.
    double AlignedDelay(unsigned int intervalMs) const;

private:
    struct Timer {
        TimerId id = 0;
        double dueMs = 0.0;
        unsigned int intervalMs = 0;
        bool alignToWall = false;
        bool hasOwner = false;
        std::weak_ptr<UIRenderView> owner;
        std::function<void()> fn;
    };

    std::vector<Timer> m_timers;
    TimerId m_nextId = 1;
    WallClock m_wallClock;
};

} // namespace jwgui
