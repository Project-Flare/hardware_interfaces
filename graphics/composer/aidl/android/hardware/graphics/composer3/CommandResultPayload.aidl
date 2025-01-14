/**
 * Copyright (c) 2021, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.hardware.graphics.composer3;

import android.hardware.graphics.composer3.ChangedCompositionTypes;
import android.hardware.graphics.composer3.ClientTargetPropertyWithBrightness;
import android.hardware.graphics.composer3.CommandError;
import android.hardware.graphics.composer3.DisplayLuts;
import android.hardware.graphics.composer3.DisplayRequest;
import android.hardware.graphics.composer3.PresentFence;
import android.hardware.graphics.composer3.PresentOrValidate;
import android.hardware.graphics.composer3.ReleaseFences;

@VintfStability
union CommandResultPayload {
    /**
     * Indicates an error generated by a command.
     *
     * If there is an error from a command, the composer should only respond
     * with the CommandError, and not with other results
     * (e.g. ChangedCompositionTypes).
     */
    CommandError error;

    /**
     * Sets the layers for which the device requires a different composition
     * type than had been set prior to the last call to VALIDATE_DISPLAY. The
     * client must either update its state with these types and call
     * ACCEPT_DISPLAY_CHANGES, or must set new types and attempt to validate
     * the display again.
     */
    ChangedCompositionTypes changedCompositionTypes;

    /**
     * Sets the display requests and the layer requests required for the last
     * validated configuration.
     *
     * Display requests provide information about how the client must handle
     * the client target. Layer requests provide information about how the
     * client must handle an individual layer.
     */
    DisplayRequest displayRequest;

    /**
     * Sets the present fence as a result of PRESENT_DISPLAY. For physical
     * displays, this fence must be signaled at the vsync when the result
     * of composition of this frame starts to appear (for video-mode panels)
     * or starts to transfer to panel memory (for command-mode panels). For
     * virtual displays, this fence must be signaled when writes to the output
     * buffer have completed and it is safe to read from it.
     */
    PresentFence presentFence;

    /**
     * Sets the release fences for device layers on this display which will
     * receive new buffer contents this frame.
     *
     * A release fence is a file descriptor referring to a sync fence object
     * which must be signaled after the device has finished reading from the
     * buffer presented in the prior frame. This indicates that it is safe to
     * start writing to the buffer again. If a given layer's fence is not
     * returned from this function, it must be assumed that the buffer
     * presented on the previous frame is ready to be written.
     *
     * The fences returned by this function must be unique for each layer
     * (even if they point to the same underlying sync object).
     *
     */
    ReleaseFences releaseFences;

    /**
     * Sets the state of PRESENT_OR_VALIDATE_DISPLAY command.
     */
    PresentOrValidate presentOrValidateResult;

    /**
     * The brightness parameter describes the intended brightness space of the client target buffer.
     * The brightness is in the range [0, 1], where 1 is the current brightness of the display.
     * When client composition blends both HDR and SDR content, the client must composite to the
     * brightness space as specified by the hardware composer. This is so that adjusting the real
     * display brightness may be applied atomically with compensating the client target output. For
     * instance, client-compositing a list of SDR layers requires dimming the brightness space of
     * the SDR buffers when an HDR layer is simultaneously device-composited.
     */
    ClientTargetPropertyWithBrightness clientTargetProperty;

    /**
     * Sets the Lut(s) for the layers.
     *
     * HWC should only request Lut(s) if SurfaceFlinger does not send the Lut(s) to the HWC.
     * The main use-case is like HDR10+ or Dolby Vision where there is no Lut to send from
     * SurfaceFlinger.
     */
    DisplayLuts displayLuts;
}
