#pragma once

class LinearAllocator;
class GrowingStackAllocator;

static LinearAllocator*         g_GlobalAllocator = nullptr;
static GrowingStackAllocator*   g_GrowingGlobalAllocator = nullptr;


