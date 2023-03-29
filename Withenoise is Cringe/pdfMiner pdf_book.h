#pragma once
#include "ebook.h"
#include "imgui_fonts.h"

#include <Python.h>
#define NOMINMAX
#include <Windows.h>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>

class LTChar {
public:
	const char* text;
	double x, y;
	const char* fontname;
	double fontSize;
	int width;

	bool validWithinAWord() {
		if ((text[0] >= 'a' && text[0] <= 'z') ||
			(text[0] >= 'A' && text[0] <= 'Z') ||
			text[0] == '-')
			return true;
		return false;
	}
};

class LTWord {
public:
	std::string text;
	int x0, y0, x1, y1;
	LTWord() {
		text = "";
		x0 = y0 = x1 = y1 = 0;
	}
};

class LTPage {
public:
	int x0, y0, x1, y1;
	std::vector<LTChar> characters;
	std::vector<LTWord> words;

	LTPage() {
		x0 = y0 = x1 = y1 = 0;
	}
};

class PdfBook : public Book {
private:
	char path[260];

	PyObject* pyScript, *pyPages, *pyClasses;
	PyObject* pyReturn; // tuple returned from python

	int currPage, totalPages;
	std::vector<LTPage> Pages;
	float zoom;
	float scrollPos;

public:
	template <class T>
	PdfBook(T filename) {
		// For some unknown reason I have to copy the over to another array
		int i;
		for (i = 0; filename[i]; i++)
			path[i] = filename[i];
		path[i] = '\0';

		PyObject* pName, * pFunc, * pArgs;

		pName = PyUnicode_FromString("parse_pdf");
		pyScript = PyImport_Import(pName);
		Py_XDECREF(pName);

		if (pyScript != NULL) {
			pFunc = PyObject_GetAttrString(pyScript, "get_pages");
			pName = PyUnicode_FromString(path);
			pArgs = PyTuple_Pack(1, pName);
			pyReturn = PyObject_CallObject(pFunc, pArgs);

			pyPages = PyTuple_GetItem(pyReturn, 0);
			pyClasses = PyTuple_GetItem(pyReturn, 1);

			Py_XDECREF(pName);
			Py_XDECREF(pFunc);
			Py_XDECREF(pArgs);
		}

		currPage = 0;
		ParseNextPage();

		ImVec2 avail_size = ImGui::GetContentRegionAvail();
		zoom = (1.5 * avail_size.x * Pages[0].y1) /
			(3 * Pages[0].x1 * avail_size.y);

		scrollPos = 0.f;
	}

	void RenderBook() {
		LTPage page = Pages[currPage];

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 avail_size = ImGui::GetContentRegionAvail();
		ImVec2 pos;

		// Maximum Zoom so that the width fits with the page
		float maxZoom = (avail_size.x * page.y1) /
			(page.x1 * avail_size.y);
		zoom = std::min(zoom, maxZoom);

		double x0 = page.x0, x1 = page.x1, y0 = page.y0, y1 = page.y1;
		double scale = avail_size.y / y1;
		scale *= zoom;
		
		y1 *= scale;
		x1 *= scale;

		// Ensure we don't scroll out of the page, into the void
		scrollPos = std::min(y1 - (y1 / zoom), (double)scrollPos);
		scrollPos = std::max(0.f, scrollPos);

		for (int i = 0; i < page.characters.size(); i++) {
			LTChar character = page.characters[i];

			pos = ImGui::GetCursorScreenPos();
			pos.x += (character.x * scale);
			pos.y += (y1 - character.y * scale) - scrollPos;

			pos.x += ((avail_size.x - x1) / 2); // display in the center
			if (zoom < 1.f)
				pos.y += ((avail_size.y - y1) / 2);

			draw_list->AddText(imFonts["Times New Roman"], character.fontSize * scale, pos, IM_COL32(255, 255, 255, 255), character.text);
		}

		// Draw the bounding boxes of words, mostly used for testing
		/*
		for (int i = 0; i < page.words.size(); i++) {
			ImVec2 pMin = ImGui::GetCursorScreenPos();
			ImVec2 pMax = ImGui::GetCursorScreenPos();
			int height = page.words[i].y1 - page.words[i].y0;

			pMin.x += (page.words[i].x0 * scale);
			pMin.y += (y1- (page.words[i].y1 - height) * scale) - scrollPos;

			pMax.x += (page.words[i].x1 * scale);
			pMax.y += (y1- (page.words[i].y0 - height) * scale) - scrollPos;

			pMin.x += ((avail_size.x - x1) / 2);
			pMax.x += ((avail_size.x - x1) / 2);
			if (zoom < 1.f) {
				pMin.y += ((avail_size.y - y1) / 2);
				pMax.y += ((avail_size.y - y1) / 2);
			}
			draw_list->AddRect(pMin, pMax, IM_COL32(190, 230, 9, 255));
		}
;		*/
	}

	void Close() {
		Pages.clear();

		Py_XDECREF(pyScript);
		Py_XDECREF(pyReturn);
	}

	void ZoomIn() {
		zoom = std::min(4.0, zoom + 0.15);
	}

	void ZoomOut() {
		zoom = std::max(0.25, zoom - 0.15);
	}

	void Scroll(float MouseWheel) {
		scrollPos -= MouseWheel * 35 * zoom;
	}

	void NextPage() {
		currPage++;
		if (currPage >= Pages.size()) {
			ParseNextPage();
		}
		scrollPos = 0;
	}

	void PrevPage() {
		if (currPage > 0) {
			scrollPos = 0;
			currPage--;
		}
		//ParsePage();
	}

	std::string GetWordAtPos(ImVec2 pos, ImVec2 &pMin, ImVec2 &pMax) {
		LTPage page = Pages[currPage];
		ImVec2 avail_size = ImGui::GetContentRegionAvail();

		double x0 = page.x0, x1 = page.x1, y0 = page.y0, y1 = page.y1;
		double scale = avail_size.y / y1;
		scale *= zoom;

		y1 *= scale;
		x1 *= scale;

		for (int i = 0; i < page.words.size(); i++) {
			pMin = ImGui::GetCursorScreenPos();
			pMax = ImGui::GetCursorScreenPos();
			LTWord word = page.words[i];

			int height = word.y1 - word.y0;

			pMin.x += (word.x0 * scale);
			pMin.y += (y1 - (word.y1 - height) * scale) - scrollPos;

			pMax.x += (word.x1 * scale);
			pMax.y += (y1 - (word.y0 - height) * scale) - scrollPos;

			pMin.x += ((avail_size.x - x1) / 2);
			pMax.x += ((avail_size.x - x1) / 2);
			if (zoom < 1.f) {
				pMin.y += ((avail_size.y - y1) / 2);
				pMax.y += ((avail_size.y - y1) / 2);
			}

			if (pos.x >= pMin.x && pos.x <= pMax.x && pos.y >= pMin.y && pos.y <= pMax.y) {
				return page.words[i].text;
			}
		}
		return "";
	}

private:
	void ExtractWords(LTPage& page) {
		float line_margin = 0.5f;
		float word_margin = 0.5f;

		LTWord word;
		for (int i = 0; i < page.characters.size(); ++i) {
			LTChar ch = page.characters[i];

			if (word.text.size() == 0) {
				if (ch.validWithinAWord())
				{
					word.text += ch.text;
					word.x0 = ch.x; word.y0 = ch.y;
				}

				continue;
			}

			if (abs(ch.y - page.characters[i - 1].y) < line_margin) {
				if (abs(ch.x - page.characters[i - 1].x) < page.characters[i - 1].fontSize + word_margin) {
					if (ch.validWithinAWord()) {
						word.text += ch.text;
						continue;
					}
				}
			}

			word.x1 = page.characters[i - 1].x + page.characters[i - 1].width;
			word.y1 = page.characters[i - 1].y + page.characters[i - 1].fontSize;
			page.words.push_back(word);

			word.text = "";
			if (ch.validWithinAWord())
			{
				word.text += ch.text;
				word.x0 = ch.x; word.y0 = ch.y;
			}
		}
		if (word.text.size() != 0)
			page.words.push_back(word);
	}

	void ParseNextPage() {
		auto tFuncCall = GetTickCount();

		LTPage newPage;

		PyObject* page_layout = PyIter_Next(pyPages);
		PyObject* pBbox = PyObject_GetAttrString(page_layout, "bbox");
		ParsePageLayout(page_layout, newPage);

		// Get bounding box
		newPage.x0 = _PyLong_AsInt(PyTuple_GetItem(pBbox, 0));
		newPage.y0 = _PyLong_AsInt(PyTuple_GetItem(pBbox, 1));
		newPage.x1 = _PyLong_AsInt(PyTuple_GetItem(pBbox, 2));
		newPage.y1 = _PyLong_AsInt(PyTuple_GetItem(pBbox, 3));

		assert(newPage.x0 == 0 && newPage.y0 == 0);

		Py_XDECREF(page_layout);
		Py_XDECREF(pBbox);

		ExtractWords(newPage);
		Pages.push_back(newPage);
		std::cout << (float)(GetTickCount() - tFuncCall) / 1000 << "s to parse a page.\n";
	}

	void ParseCharcater(PyObject* pyCharacter, LTPage & page) {
		PyObject* pyText = PyObject_GetAttrString(pyCharacter, "_text");
		PyObject* pyFontname = PyObject_GetAttrString(pyCharacter, "fontname");
		PyObject* pyFontSize = PyObject_GetAttrString(pyCharacter, "size");
		PyObject* pyWidth = PyObject_GetAttrString(pyCharacter, "width");
		PyObject* pyMatrix = PyObject_GetAttrString(pyCharacter, "matrix");

		LTChar Character;
		auto text = PyUnicode_AsUTF8(pyText);
		Character.text = text;
		Character.fontname = PyUnicode_AsUTF8(pyFontname);
		Character.x = PyFloat_AsDouble(PyTuple_GetItem(pyMatrix, 4));
		Character.y = PyFloat_AsDouble(PyTuple_GetItem(pyMatrix, 5));
		Character.fontSize = PyFloat_AsDouble(pyFontSize);
		Character.width = (int)PyFloat_AsDouble(pyWidth);

		page.characters.push_back(Character);

		Py_XDECREF(pyText);
		Py_XDECREF(pyFontname);
		Py_XDECREF(pyFontSize);
		Py_XDECREF(pyMatrix);
	}

	void ParseTextLine(PyObject* textLine, LTPage & page) {
		PyObject* Iter = PyObject_GetAttrString(textLine, "__iter__");
		PyObject* pyArgs = NULL;
		PyObject* pyCharacters = PyObject_CallObject(Iter, pyArgs);
		PyObject* character;

		while ((character = PyIter_Next(pyCharacters))) {
			PyObject* pyLTChar = PyTuple_GetItem(pyClasses, 2);

			if (PyObject_IsInstance(character, pyLTChar) == 1) {
				ParseCharcater(character, page);
			}

			Py_XDECREF(character);;
		}

		Py_XDECREF(Iter);
		Py_XDECREF(pyCharacters);
	}

	void ParseTextContainer(PyObject* textContainer, LTPage & page) {
		PyObject* Iter = PyObject_GetAttrString(textContainer, "__iter__");
		PyObject* pyArgs = NULL;
		PyObject* text_lines = PyObject_CallObject(Iter, pyArgs);
		PyObject* text_line;

		while ( (text_line = PyIter_Next(text_lines) ) ) {
			PyObject* LTTextLine = PyTuple_GetItem(pyClasses, 1);

			if (PyObject_IsInstance(text_line, LTTextLine) == 1) {
				ParseTextLine(text_line, page);
			}
			Py_XDECREF(text_line);
		}

		Py_XDECREF(Iter);
		Py_XDECREF(text_lines);
	}

	void ParsePageLayout(PyObject* pageLayout , LTPage & page) {
		PyObject* Iter = PyObject_GetAttrString(pageLayout, "__iter__");
		PyObject* pyArgs = NULL;
		PyObject* elements = PyObject_CallObject(Iter, pyArgs);
		PyObject* element;

		assert(PyIter_Check(elements));

		while ((element = PyIter_Next(elements))) {
			PyObject* LTTextContainer = PyTuple_GetItem(pyClasses, 0);
			if (PyObject_IsInstance(element, LTTextContainer) == 1) {
				ParseTextContainer(element, page);
			}
			Py_XDECREF(element);
		}

		Py_XDECREF(Iter);
		Py_XDECREF(elements);
	}
};