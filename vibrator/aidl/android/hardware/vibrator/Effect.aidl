/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.hardware.vibrator;

@VintfStability
@Backing(type="int")
enum Effect {
    /**
     * A single click effect.
     *
     * This effect should produce a sharp, crisp click sensation.
     */
    CLICK = 0,
    /**
     * A double click effect.
     *
     * This effect should produce two sequential sharp, crisp click sensations with a minimal
     * amount of time between them.
     */
    DOUBLE_CLICK = 1,
    /**
     * A tick effect.
     *
     * This effect should produce a soft, short sensation, like the tick of a clock.
     */
    TICK = 2,
    /**
     * A thud effect.
     *
     * This effect should solid feeling bump, like the depression of a heavy mechanical button.
     */
    THUD = 3,
    /**
     * A pop effect.
     *
     * A short, quick burst effect.
     */
    POP = 4,
    /**
     * A heavy click effect.
     *
     * This should produce a sharp striking sensation, like a click but stronger.
     */
    HEAVY_CLICK = 5,
    /**
     * Ringtone patterns. They may correspond with the device's ringtone audio, or may just be a
     * pattern that can be played as a ringtone with any audio, depending on the device.
     */
    RINGTONE_1 = 6,
    RINGTONE_2 = 7,
    RINGTONE_3 = 8,
    RINGTONE_4 = 9,
    RINGTONE_5 = 10,
    RINGTONE_6 = 11,
    RINGTONE_7 = 12,
    RINGTONE_8 = 13,
    RINGTONE_9 = 14,
    RINGTONE_10 = 15,
    RINGTONE_11 = 16,
    RINGTONE_12 = 17,
    RINGTONE_13 = 18,
    RINGTONE_14 = 19,
    RINGTONE_15 = 20,
    /**
     * A soft tick effect meant to be played as a texture.
     *
     * A soft, short sensation like the tick of a clock. Unlike regular effects, texture effects
     * are expected to be played multiple times in quick succession, replicating a specific
     * texture to the user as a form of haptic feedback.
     */
    TEXTURE_TICK = 21,
}
