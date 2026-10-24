#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <mutex>
#include <type_traits>

namespace lxr {

/**
 * VoiceContext - gives every plugin instance its own private copy of the
 * LXR voice state.
 *
 * Why this exists
 * ---------------
 * The LXR firmware keeps each voice's complete state (oscillators,
 * envelopes, filter, transient generator, parameters, trigger state ...)
 * in ONE file-scope global: voiceArray[], snareVoice, cymbalVoice or
 * hatVoice. On the hardware that is fine, there is only one of each. In
 * a DAW the plugin DLL is loaded once per process, so every instance of a
 * plugin type would otherwise share that single global: a note sent to
 * one instance triggers all of them, parameters leak between instances,
 * and constructing a new instance re-initialises the voice under the
 * others.
 *
 * How it works
 * ------------
 * Each engine owns a VoiceContext<Voice>. The context holds a private
 * copy of the voice struct ("saved"). Before an engine touches the DSP it
 * takes the context's Lock. Taking the Lock
 *   1. locks a per-plugin-DLL mutex (so two instances never use the
 *      global at the same time, e.g. on different audio threads), and
 *   2. if a different instance last used the global, copies the global
 *      back into that instance's private copy and loads this instance's
 *      private copy into the global.
 * Consecutive calls from the same instance cost nothing extra. The DSP
 * code itself is untouched; it always sees "the" global voice, which
 * simply holds the state of whichever instance is currently running.
 *
 * The voice structs are plain C data that are only ever copied back to
 * the same global address, so a byte copy is safe (pointers inside them
 * point at the global itself or at constant tables).
 *
 * Not covered (deliberately): the random-number generator and the host
 * tempo are still shared. That is harmless: instances simply draw
 * different noise values from one stream, and all instances in a project
 * see the same tempo.
 */
template <typename Voice>
class VoiceContext
{
    static_assert (std::is_trivially_copyable<Voice>::value,
                   "LXR voice state must be plain data to be swapped by memcpy");

public:
    /** @param globalState address of the DSP's global voice (or voice array)
        @param count       number of Voice elements at that address */
    VoiceContext (Voice* globalState, std::size_t count = 1)
        : global (globalState), numElems (count), saved (new Voice[count]())
    {
    }

    ~VoiceContext()
    {
        std::lock_guard<std::recursive_mutex> g (shared().mutex);
        if (shared().owner == this)
            shared().owner = nullptr;   // our state is simply abandoned
    }

    VoiceContext (const VoiceContext&)            = delete;
    VoiceContext& operator= (const VoiceContext&) = delete;

    /** RAII guard: while it lives, the global voice holds THIS instance's state. */
    class Lock
    {
    public:
        explicit Lock (VoiceContext& c) : guard (shared().mutex) { c.makeCurrent(); }

    private:
        std::lock_guard<std::recursive_mutex> guard;
    };

    /** Usage:  auto lk = ctx.lock();   (safe to nest; the mutex is recursive) */
    Lock lock() { return Lock (*this); }

private:
    struct Shared
    {
        std::recursive_mutex mutex;
        VoiceContext*        owner { nullptr };   // whose state is in the global right now
    };

    // One per plugin DLL and Voice type (function-local static: thread-safe init).
    static Shared& shared()
    {
        static Shared s;
        return s;
    }

    void makeCurrent()
    {
        Shared& s = shared();
        if (s.owner == this)
            return;

        if (s.owner != nullptr)
            s.owner->saveFromGlobal();

        std::memcpy (static_cast<void*> (global), saved.get(), sizeof (Voice) * numElems);
        s.owner = this;
    }

    void saveFromGlobal()
    {
        std::memcpy (saved.get(), static_cast<const void*> (global), sizeof (Voice) * numElems);
    }

    Voice*                   global;
    std::size_t              numElems;
    std::unique_ptr<Voice[]> saved;
};

} // namespace lxr
