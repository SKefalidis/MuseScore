//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_MIDI_FLUIDLITESYNTH_H
#define MU_MIDI_FLUIDLITESYNTH_H

#include <memory>
#include <vector>
#include <cstdint>
#include <functional>

#include "../isynthesizer.h"
#include "../miditypes.h"

#include "modularity/ioc.h"

//! NOTE Used for the test, the main synthesizer will not be this one.

namespace mu {
namespace midi {
struct Fluid;
class FluidLiteSynth : public ISynthesizer
{
public:
    FluidLiteSynth();

    std::string name() const override;

    Ret init(float samplerate) override;
    Ret addSoundFont(const io::path& filePath) override;
    Ret setupChannels(const Programs& programs) override;

    bool handleEvent(const Event& e) override;

    void allSoundsOff() override; // all channels
    void flushSound() override;

    void channelSoundsOff(channel_t chan) override;
    bool channelVolume(channel_t chan, float val) override;  // 0. - 1.
    bool channelBalance(channel_t chan, float val) override; // -1. - 1.
    bool channelPitch(channel_t chan, int16_t pitch) override; // -12 - 12

    void writeBuf(float* stream, unsigned int samples) override;

private:

    const Program& program(uint16_t chan) const;

    enum midi_control
    {
        BANK_SELECT_MSB = 0x00,
        VOLUME_MSB      = 0x07,
        BALANCE_MSB     = 0x08,
        PAN_MSB         = 0x0A
    };

    std::shared_ptr<Fluid> m_fluid = nullptr;
    bool m_isLoggingSynthEvents = false;

    std::vector<float> m_preallocated; // used to flush a sound
    float m_sampleRate = 44100.0f;
    Programs m_programs;
};
}
}

#endif //MU_MIDI_FLUIDLITESYNTH_H