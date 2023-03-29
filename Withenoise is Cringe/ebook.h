#pragma once
#include "imgui.h"
#include <d3d11.h>

#include <fstream>
#include <iostream>
#include <vector>

class Book {
protected:
	std::string bookName;

public:
	virtual void RenderBook() {}

	virtual void Close() {}

	virtual void ZoomIn() {}
	virtual void ZoomOut() {}
	virtual void Scroll(float MouseWheel) {}

	virtual void NextPage() {}
	virtual void PrevPage() {}

	virtual std::string GetWordAtPos (ImVec2 pos, ImVec2& pMin, ImVec2& pMax, std::string & sentence) {
		return "";
	}

	std::string getName() {
		return bookName;
	}
};