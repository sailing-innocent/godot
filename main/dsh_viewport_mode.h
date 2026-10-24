/**************************************************************************/
/*  dsh_viewport_mode.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include <cstdint>

// Process-wide state for the DSH (DeepSeek Harness) native viewport worker
// mode, enabled by the reserved --dsh-viewport-protocol 2 --broker-pid <pid>
// arguments appended by the DSH desktop broker. Set during Main::setup()
// argument parsing; consumed by the Windows display server and the worker.
//
// Main::iteration polls worker_stop_check (registered by the platform worker)
// instead of including any platform header, keeping main/ free of
// platform-specific includes.
struct DSHViewportMode {
	static bool worker_enabled; // --dsh-viewport-protocol 2 was passed.
	static uint32_t broker_pid; // --broker-pid value (target of the NT handle export).
	typedef bool (*WorkerStopCheck)();
	static WorkerStopCheck worker_stop_check; // Non-null while a worker runs.
};
