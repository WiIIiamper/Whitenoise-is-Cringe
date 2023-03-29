#pragma once

#include <mupdf/fitz.h>
#include <vector>
#include <string>
#include "string_utils.h"
#include "DX11 Helper.h"

struct MuPdfWord {
	std::string text;
	float x0, y0, x1, y1;
	int sentenceIdx;
};

struct MuPdfPage {
	int w, h;
	ID3D11ShaderResourceView* texture;
};

class MuPDFWrapper {
	fz_context* context;
	fz_document* doc;

	int w, h;
	MuPdfPage page;

	int numPages;
	std::vector< std::string > sentences;

public:
	MuPDFWrapper(const char* filename) {
		LoadDocument(filename);
	}

	MuPdfPage getPage(int pageIdx, double zoom) {
		fz_matrix transformMatrix = GetTransformationMatrix(zoom);
		fz_pixmap* pixmap = GetPixmapFromPage(context, doc, pageIdx, transformMatrix);
		AddStraightAlphaToPixmap(pixmap);

		DX11Helper::mupdfPixmapToShaderResourceView(pixmap, &page.texture);
		page.w = pixmap->w;
		page.h = pixmap->h;
		fz_drop_pixmap(context, pixmap);
		return page;
	}

	int getNumPages() {
		return numPages;
	}

	std::vector<MuPdfWord> ExtractWords(int pageIdx) {
		fz_stext_page* StructTextPage = GetStructuredTextFromPage(pageIdx);
		std::vector <MuPdfWord> words;
		ExtractWordsFromStextPage(StructTextPage, words);
		fz_drop_stext_page(context, StructTextPage);
		return words;
	}

	std::string getSentence(int sentenceIdx) {
		return sentences[sentenceIdx];
	}

	void CleanUp() {
		fz_drop_document(context, doc);
		fz_drop_context(context);
	}

private:
	void LoadDocument(const char* path) {
		context = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
		if (!context) {
			std::cout << "Cannot create mudpf context.\n";
			return;
		}

		fz_var(doc);

		fz_try(context) {
			RegisterDocumentHandlers(context);
			doc = OpenDocument(context, path);
			numPages = CountPages(context, doc);
		}
		fz_catch(context) {
			CleanUp();
		}
	}

	void RegisterDocumentHandlers(fz_context* context) {
		fz_try(context) {
			fz_register_document_handlers(context);
		}
		fz_catch(context)
		{
			fprintf(stderr, "cannot register document handlers: %s\n", fz_caught_message(context));
		}
	}

	fz_document* OpenDocument(fz_context* context, const char* path) {
		fz_document* doc = NULL;
		fz_try(context)
			doc = fz_open_document(context, path);
		fz_catch(context)
		{
			fprintf(stderr, "cannot open document: %s\n", fz_caught_message(context));
		}
		return doc;
	}

	int CountPages(fz_context* context, fz_document* doc) {
		int numPages = 0;
		fz_try(context)
			numPages = fz_count_pages(context, doc);
		fz_catch(context)
		{
			fprintf(stderr, "cannot count number of pages: %s\n", fz_caught_message(context));
		}

		return numPages;
	}

	fz_pixmap* GetPixmapFromPage(fz_context* context, fz_document* document, int pageNumber, fz_matrix transform) {
		fz_pixmap* pixmap = NULL;

		fz_try(context)
			pixmap = fz_new_pixmap_from_page_number(context, doc, pageNumber, transform,fz_device_rgb(context), 0);
		fz_catch(context)
		{
			fprintf(stderr, "cannot render page: %s\n", fz_caught_message(context));
		}

		return pixmap;
	}

	void AddStraightAlphaToPixmap(fz_pixmap* pixmap) {
		int numPixels = pixmap->w * pixmap->h;
		unsigned char* newSamples = new unsigned char[numPixels * 4];
		int R = 0, G = 1, B = 2;

		int iter = 0;
		unsigned char* p = pixmap->samples;
		for (int pix = 0; pix < numPixels; ++pix)
		{
			newSamples[iter++] = p[R];
			newSamples[iter++] = p[G];
			newSamples[iter++] = p[B];
			newSamples[iter++] = 255; // alpha channel
			p += pixmap->n;
		}
		pixmap->n = 4;
		delete pixmap->samples;
		pixmap->samples = newSamples;
		pixmap->stride = pixmap->w * pixmap->n;
	}

	fz_page* LoadPage(int pageIdx) {
		fz_page* page = NULL;
		fz_try(context) {
			page = fz_load_page(context, doc, pageIdx);
		} 
		fz_catch(context) {
			std::cout << "Cannot load page " << pageIdx << "\n" << fz_caught_message(context);
		}
		return page;
	}

	fz_stext_page* GetStructuredTextFromPage(int pageIdx) {
		fz_stext_page* StructTextPage = NULL;
		fz_page* page = LoadPage(pageIdx);

		fz_try(context) {
			StructTextPage = fz_new_stext_page_from_page(context, page, NULL);
		}
		fz_catch(context) {
			std::cout << "Cannot get structured text from page " << pageIdx << "\n" << fz_caught_message(context);
		}

		fz_drop_page(context, page);
		return StructTextPage;
	}

	void ExtractWordsFromStextPage(fz_stext_page* StructTextPage, std::vector<MuPdfWord>&words) {
		sentences.clear();
		sentences.push_back("");
		auto TextBlock = StructTextPage->first_block;
		while (TextBlock != NULL) {
			ExtractWordsFromTextBlock(TextBlock, words);
			TextBlock = TextBlock->next;
		}
	}

	void ExtractWordsFromTextBlock(fz_stext_block* TextBlock, std::vector<MuPdfWord>& words) {
		auto TextLine = TextBlock->u.t.first_line;
		while (TextLine != NULL) {
			ExtractWordsFromTextLine(TextLine, words);
			TextLine = TextLine->next;
		}
	}

	void ExtractWordsFromTextLine(fz_stext_line* TextLine, std::vector<MuPdfWord>& words) {
		MuPdfWord currWord;
		currWord.text = "";
		currWord.sentenceIdx = sentences.size() - 1;
		for (auto ch = TextLine->first_char; ch != NULL; ch = ch->next) {
			if (str::isLetter(ch->c)) {
				AppendCharToWord(currWord, ch);
			}
			else if (!currWord.text.empty()) {
				words.push_back(currWord);
				currWord.text = "";
			}

			AppencCharToCurrSentence(currWord, ch);
		}
		if (currWord.text.size()) {
			words.push_back(currWord);
		}
	}

	void AppendCharToWord(MuPdfWord& word, fz_stext_char* ch) {
		if (word.text.empty()) {
			word.x0 = ch->quad.ul.x;
			word.y0 = ch->quad.ul.y;
		}
		word.x1 = ch->quad.lr.x;
		word.y1 = ch->quad.lr.y;
		word.text += ch->c;
	}

	void AppencCharToCurrSentence(MuPdfWord& currWord, fz_stext_char* ch) {
		if (!(sentences.back().size() == 0 && ch->c == ' '))
			sentences.back() += ch->c;
		if (ch->c == '.') {
			sentences.push_back("");
			currWord.sentenceIdx = sentences.size() - 1;
		}
	}

	fz_matrix GetTransformationMatrix(double zoom) {
		fz_matrix transform = fz_scale(zoom, zoom);
		return transform;
	}
};
