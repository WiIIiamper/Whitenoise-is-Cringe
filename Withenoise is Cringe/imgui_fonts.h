#pragma once
#include "imgui.h"
#include <unordered_map>

extern std::unordered_map<const char*, ImFont*> imFonts;

extern void LoadFonts(ImGuiIO& io);