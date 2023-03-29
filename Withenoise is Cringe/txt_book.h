#pragma once
#include "imgui.h"
#include "ebook.h"
#include "imgui_fonts.h"

#include <fstream>
#include <iostream>
#include <vector>

class TxtBook : public Book {
	std::vector < char* > text;

public:
	template <class T>
	TxtBook(T filename) {
		std::ifstream in(filename);

		while (!in.eof()) {
			char* line = new char[1025];
			text.push_back(line);
			in.getline(text.back(), 1024);
		}
		in.close();
	}

	void RenderBook() {
		ImGui::PushFont(imFonts["Times New Roman"]);

		ImGui::BeginChild("Text");
		ImGuiListClipper clipper;
		clipper.Begin((int)text.size());
		while (clipper.Step())
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				ImGui::Text(text[i]);

		ImGui::EndChild();

		ImGui::PopFont();
	}

	void Close() {
		for (int i = 0; i < text.size(); ++i) {
			delete text[i];
		}
		text.clear();
	}
};
