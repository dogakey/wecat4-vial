// Copyright 2024 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define RENAME_WITH_LINE(A, B) COMBINE(A, B)
#define COMBINE(A, B) A##B
#define raw_hid_send(a, b) RENAME_WITH_LINE(_temp_rhs_, __LINE__)(a, b)
#define _temp_rhs_29 replaced_hid_send  // raw_hid.h
#define _temp_rhs_442 replaced_hid_send // via.c (vial-qmk dd43959a 기준 줄번호. 제조사 원본 값은 461)
#define _temp_rhs_466 replaced_hid_send // via.c (vial-qmk dd43959a 기준)
