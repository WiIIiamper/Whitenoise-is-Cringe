#pragma once

#include <Windows.h>
#include "ebook.h"
#include "MuPDF Wrapper.h"

class MuPDFBook : public Book {
	MuPDFWrapper* mupdf = NULL;
	char DocPath[260];

	int currPageIdx = 0, numPages;
	MuPdfPage currPage;
	std::vector<MuPdfWord> words;
	double zoom, scroll;

public:
	MuPDFBook(LPWSTR filename) {
		StartUp(filename);
	}

	void StartUp(LPWSTR filename) {
		CopyFilenameToPath(DocPath, filename);
		GetBookName(DocPath);
		mupdf = new MuPDFWrapper(DocPath);

		numPages = mupdf->getNumPages();
		zoom = 1.0;
		scroll = 0;
		currPage = mupdf->getPage(0, zoom);
	}

	void RenderBook() {
		EnsureZoomFitsPageWidth();
		EnsureScrollPosIsOnThePage();

		int w = currPage.w, h = currPage.h;
		ImVec2 pMin = PagePointToRenderPoint(0, 0);
		ImVec2 pMax = PagePointToRenderPoint(w/zoom, h/zoom);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddImage((void*)currPage.texture, pMin, pMax);
	}

	void Close() {
		mupdf->CleanUp();
		delete mupdf;
	}

	void NextPage() {
		if (currPageIdx + 1 < numPages) {
			currPage = mupdf->getPage(++currPageIdx, zoom);
		}
		scroll = 0;
	}

	void PrevPage() {
		if (currPageIdx > 0) {
			currPage = mupdf->getPage(--currPageIdx, zoom);
		}
		scroll = 0;
	}

	void Scroll(float MouseWheel) {
		scroll -= MouseWheel * 35 * zoom;
	}

	void ZoomIn() {
		Zoom(0.15f);
	}

	void ZoomOut() {
		Zoom(-0.15f);
	}

	std::string GetWordAtPos(ImVec2 pos, ImVec2& upperLeft, ImVec2& lowerRight, std::string& sentence) {
		words = mupdf->ExtractWords(currPageIdx);
		for (auto word : words) {
			upperLeft = PagePointToRenderPoint(word.x0, word.y0);
			lowerRight = PagePointToRenderPoint(word.x1, word.y1);

			if (pos.x >= upperLeft.x && pos.x <= lowerRight.x 
				&& pos.y >= upperLeft.y && pos.y <= lowerRight.y) {
				sentence = mupdf->getSentence(word.sentenceIdx);
				return word.text;
			}
		}
		return "";
	}

private:
	void CopyFilenameToPath(char* path, LPWSTR filename) {
		int i;
		for (i = 0; filename[i]; i++)
			path[i] = filename[i];
		path[i] = '\0';
	}

	void GetBookName(char* path) {
		for (int i = 0; path[i]; i++) {
			bookName += path[i];
			if (path[i] == '\\')
				bookName = "";
		}
		for (size_t i = bookName.size() - 1; i > 0; i--) {
			if (bookName[i] == '.') {
				bookName = bookName.substr(0, i);
				break;
			}
		}
	}

	ImVec2 PagePointToRenderPoint(float x, float y) {
		ImVec2 res(x, y);
		ImVec2 avail_size = ImGui::GetContentRegionAvail();
		int w = currPage.w, h = currPage.h;

		res.x *= zoom;
		res.y *= zoom;
		res.y -= scroll;
		res.x += (avail_size.x - w) / 2;
		if (h < avail_size.y)
			res.y += (avail_size.y - h) / 2;
		res.x += ImGui::GetCursorScreenPos().x;
		res.y += ImGui::GetCursorScreenPos().y;

		return res;
	}

	void Zoom(double delta) {
		double originalZoom = zoom;
		double maxZoom = GetMaximumZoom();
		zoom += delta;
		if (zoom < 0.25f)
			zoom = 0.25f;
		if (zoom > maxZoom)
			zoom = maxZoom;

		if (zoom != originalZoom) {
			currPage = mupdf->getPage(currPageIdx, zoom);
		}
	}

	void EnsureZoomFitsPageWidth() {
		int pageWidth = currPage.w;
		int windowWidth = ImGui::GetContentRegionAvail().x;
		if (pageWidth > windowWidth) {
			zoom = GetMaximumZoom();
			currPage = mupdf->getPage(currPageIdx, zoom);
		}
	}

	double GetMaximumZoom() {
		int pageWidth = currPage.w;
		int windowWidth = ImGui::GetContentRegionAvail().x;
		int originalPageWidth = pageWidth / zoom;
		return (double)windowWidth / originalPageWidth;
	}

	// Ensures we don't scroll into the void
	void EnsureScrollPosIsOnThePage() {
		int windowHeight = ImGui::GetContentRegionAvail().y;
		int pageHeight = currPage.h;
		if (pageHeight - scroll < windowHeight)
			scroll = pageHeight - windowHeight;

		if (scroll < 0.f)
			scroll = 0.f;
	}

	void RenderWordBboxes() {
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		for (int i = 0; i < words.size(); i++) {
			MuPdfWord word = words[i];
			ImVec2 pMin = PagePointToRenderPoint(word.x0, word.y1);
			ImVec2 pMax = PagePointToRenderPoint(word.x1, word.y0);

			draw_list->AddRect(pMin, pMax, IM_COL32(190, 230, 9, 255));
		}
	}
};