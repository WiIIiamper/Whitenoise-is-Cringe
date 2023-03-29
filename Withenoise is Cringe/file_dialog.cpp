#include "file_dialog.h"

#include <iostream>

enum BOOK_TYPE {TXT, PDF, EPUB, CBZ, UNKNOWN};
BOOK_TYPE getBookType(LPWSTR lpstrFile);

bool openFileDialog( HWND owner_window, Book* &book ) {
	OPENFILENAME ofn;
	wchar_t szFile[260];

	const wchar_t* filter = L"Book\0*.TXT;*.epub;*.pdf;*.cbz\0";

	// Intialize ofn
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = owner_window;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		// in case a book is already loaded
		if (book != NULL) {
			book->Close();
			delete book;
			book = NULL;
		}

		BOOK_TYPE type = getBookType(ofn.lpstrFile);
		switch (type) {
		case TXT:
			book = new TxtBook(ofn.lpstrFile);
			break;
		case PDF:
			book = new MuPDFBook(ofn.lpstrFile);
			break;
		case EPUB:
			book = new MuPDFBook(ofn.lpstrFile);
			break;
		case CBZ:
			book = new MuPDFBook(ofn.lpstrFile);
			break;
		default:
			return false;
		}
		return true;
	}

	return false;
}

bool strMatch(const char* a, const char* b) {
	int i;
	for (i = 0; a[i] && b[i]; ++i) {
		if (a[i] != b[i])
			return false;
	}

	if (a[i] || b[i])
		return false;
	return true;
}

BOOK_TYPE getBookType(LPWSTR lpstrFile) {
	int dot = 0;
	for (int i = 0; lpstrFile[i]; ++i)
		if (lpstrFile[i] == '.')
			dot = i;

	char ending[5]; int i;
	for (i = dot + 1; lpstrFile[i] && i < dot + 5; ++i) 
		ending[i - dot - 1] = lpstrFile[i];
	ending[i-dot-1] = '\0';

	if (strMatch(ending, "txt"))
		return TXT;
	else if (strMatch(ending, "pdf"))
		return PDF;
	else if (strMatch(ending, "epub"))
		return EPUB;
	else if (strMatch(ending, "cbz"))
		return CBZ;

	// should never reach this
	return UNKNOWN;
}

