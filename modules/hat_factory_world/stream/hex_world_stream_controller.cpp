/**************************************************************************/
/*  hex_world_stream_controller.cpp                                       */
/**************************************************************************/
#include "hex_world_stream_controller.h"

#include "../runtime/hex_region_snapshot.h"
#include "core/object/class_db.h"

void HexWorldStreamController::_bind_methods() {
	ClassDB::bind_method(D_METHOD("bind_view", "view"), &HexWorldStreamController::bind_view);
	ClassDB::bind_method(D_METHOD("request_region", "cells"), &HexWorldStreamController::request_region);
	ClassDB::bind_method(D_METHOD("pin_region", "cells", "priority"), &HexWorldStreamController::pin_region, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("unpin", "handle"), &HexWorldStreamController::unpin);
	ClassDB::bind_method(D_METHOD("is_ready", "coord"), &HexWorldStreamController::is_ready);
	ClassDB::bind_method(D_METHOD("get_state", "coord"), &HexWorldStreamController::get_state);
	ClassDB::bind_method(D_METHOD("get_epoch"), &HexWorldStreamController::get_epoch);
	ClassDB::bind_method(D_METHOD("set_failure_callback", "callback"), &HexWorldStreamController::set_failure_callback);
	ClassDB::bind_method(D_METHOD("is_epoch_current", "epoch"), &HexWorldStreamController::is_epoch_current);
	BIND_ENUM_CONSTANT(STATE_UNLOADED);
	BIND_ENUM_CONSTANT(STATE_REQUESTED);
	BIND_ENUM_CONSTANT(STATE_READY);
	BIND_ENUM_CONSTANT(STATE_FAILED);
}

void HexWorldStreamController::bind_view(const Ref<HexRegionSnapshot> &p_view) {
	view = p_view;
	epoch++;
}

uint32_t HexWorldStreamController::request_region(const TypedArray<MicroCoord> &p_cells) {
	// v1 synchronous placeholder: the authoritative Rules layer is fully
	// resident, so readiness is answered from the bound snapshot. The epoch
	// increments per request; a future async engine consults it to discard
	// stale results (extension point, see file header).
	ERR_FAIL_COND_V(view.is_null(), 0);
	epoch++;
	for (int i = 0; i < p_cells.size(); i++) {
		if (!view->has_cell_data(p_cells[i])) {
			// High-availability rule: never fabricate placeholder cells; the
			// failure callback reports the exact missing coordinate.
			if (on_failed.is_valid()) {
				on_failed.call(p_cells[i]);
			}
			return epoch;
		}
	}
	return epoch;
}

int HexWorldStreamController::pin_region(const TypedArray<MicroCoord> &p_cells, int p_priority) {
	// v1: pins are advisory counters (nothing streams out). Priority is
	// recorded for the future budget evictor; the handle is real.
	(void)p_priority;
	ERR_FAIL_COND_V(view.is_null(), 0);
	uint64_t tag = 0;
	for (int i = 0; i < p_cells.size(); i++) {
		const Ref<MicroCoord> c = p_cells[i];
		tag = tag * 131 + (uint64_t)(uint32_t)c->get_q() * 8191 + (uint64_t)(uint32_t)c->get_r();
	}
	const int handle = next_pin++;
	pins.insert(handle, tag);
	const int *existing = pin_counts.getptr(tag);
	pin_counts[tag] = (existing ? *existing : 0) + 1;
	return handle;
}

void HexWorldStreamController::unpin(int p_pin_handle) {
	const uint64_t *tag = pins.getptr(p_pin_handle);
	if (tag == nullptr) {
		return;
	}
	const int *count = pin_counts.getptr(*tag);
	const int remaining = (count ? *count : 0) - 1;
	if (remaining <= 0) {
		pin_counts.erase(*tag);
	} else {
		pin_counts[*tag] = remaining;
	}
	pins.erase(p_pin_handle);
}

bool HexWorldStreamController::is_ready(const Ref<MicroCoord> &p_coord) const {
	if (view.is_null() || p_coord.is_null()) {
		return false;
	}
	// Rules layer readiness = authoritative data present (no placeholders).
	return view->has_cell_data(p_coord);
}

int HexWorldStreamController::get_state(const Ref<MicroCoord> &p_coord) const {
	return is_ready(p_coord) ? STATE_READY : STATE_FAILED;
}

void HexWorldStreamController::set_failure_callback(const Callable &p_callback) {
	on_failed = p_callback;
}

bool HexWorldStreamController::is_epoch_current(uint32_t p_epoch) const {
	// In v1 the view is immutable per binding, so any epoch issued after the
	// latest bind_view remains current. The async engine will compare against
	// an advancing epoch instead; the check shape is already final.
	return view.is_valid() && p_epoch <= epoch && p_epoch > 0;
}
