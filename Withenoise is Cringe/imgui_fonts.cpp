#include "imgui_fonts.h"

std::unordered_map<const char*, ImFont*> imFonts;

void LoadFonts(ImGuiIO& io) {
	imFonts["Open Sans"] = io.Fonts->AddFontFromFileTTF("assets\\fonts\\OpenSans\\OpenSans-Regular.ttf", 18.0f);
	imFonts["Times New Roman"] = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\times.ttf", 18.0f);

	io.FontDefault = imFonts["Open Sans"];
}