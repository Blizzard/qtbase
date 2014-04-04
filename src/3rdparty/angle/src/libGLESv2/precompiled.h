//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// precompiled.h: Precompiled header file for libGLESv2.

#define GL_APICALL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define EGLAPI
#include <EGL/egl.h>

#include <assert.h>
#include <cstddef>
#include <float.h>
#include <intrin.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <algorithm> // for std::min and std::max
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY==WINAPI_FAMILY_APP || WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP)
#define ANGLE_OS_WINRT
#if WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP
#define ANGLE_OS_WINPHONE
#endif
#endif

#ifndef ANGLE_ENABLE_D3D11
#include <d3d9.h>
#else
#if !defined(ANGLE_OS_WINRT)
#include <D3D11.h>
#else
#include <d3d11_1.h>
#define Sleep(x) WaitForSingleObjectEx(GetCurrentThread(), x, FALSE)
#define GetVersion() WINVER
#endif
#include <dxgi.h>
#endif
#ifndef ANGLE_OS_WINPHONE
#include <D3Dcompiler.h>
#endif

#ifdef _MSC_VER
#include <hash_map>
#endif
