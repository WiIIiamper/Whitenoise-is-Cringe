#pragma once
#include "ebook.h"
#include "txt_book.h"
#include "MuPDF Book.h"

#include <Windows.h>
#include <fstream>

extern bool openFileDialog(HWND owner_window, Book* &book);