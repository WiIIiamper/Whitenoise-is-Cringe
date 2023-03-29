#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <imgui.h>

#define CURL_STATICLIB
#include <curl/curl.h>

struct AnkiNote {
	std::string term;
	std::string glossary;
	std::string sentence;
	std::string source;
	std::string audioURL;
};

struct addNoteInfoStruct {
	std::string deckName;
	std::string modelName;
	std::string termField;
	std::string glossaryField;
	std::string sentenceField;
	std::string sourceField;
	std::string audioURLField;
} addNoteInfo;

class Anki {
public:
	static void init() {
		initAddNoteInfo();
	}

	static bool isOpen() {
		std::string res = invoke("");
		return !(res.size() == 0);
	}

	static void AddNote(AnkiNote note) {
		if (!isOpen())
			return;

		std::string httpsRequest = createAddNoteRequest(note);
		std::cout << httpsRequest << "\n";
		std::cout << invoke(httpsRequest.c_str()) << "\n";
	}

private:
	static void initAddNoteInfo() {
		addNoteInfo.deckName = jsonCompatible("English");
		addNoteInfo.modelName = jsonCompatible("anime-card-en");
		addNoteInfo.termField = jsonCompatible("Word");
		addNoteInfo.glossaryField = jsonCompatible("Glossary");
		addNoteInfo.sentenceField = jsonCompatible("Sentence");
		addNoteInfo.sourceField = jsonCompatible("Source");
		addNoteInfo.audioURLField = jsonCompatible("Audio");
	}

	//This was coppied from 'The Quantum Physicist' from Stack Overflow
	static size_t CurlWrite_CallbackFunc_StdString(void* contents, size_t size, size_t nmemb, std::string* s)
	{
		size_t newLength = size * nmemb;
		try
		{
			s->append((char*)contents, newLength);
		}
		catch (std::bad_alloc& e)
		{
			//handle memory problem
			return 0;
		}
		return newLength;
	}

	static std::string invoke(const char* command) {
		CURL* curl;
		CURLcode res;

		std::string result;
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, "localhost:8765");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, command);

			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				return "";
			}

			curl_easy_cleanup(curl);
		}

		return result;
	}

	static std::string createAddNoteRequest(AnkiNote note) {
		std::string req = "{\"action\":\"addNote\",\"version\":6,\"params\":{\"note\":{";
		req += "\"deckName\":\""; req += addNoteInfo.deckName; req += "\",";
		req += "\"modelName\":\""; req += addNoteInfo.modelName; req += "\",";
		req += "\"fields\": {";
		addFieldToRequest(req, addNoteInfo.termField, note.term); req += ',';
		addFieldToRequest(req, addNoteInfo.glossaryField, note.glossary); req += ',';
		addFieldToRequest(req, addNoteInfo.sentenceField, note.sentence); req += ',';
		addFieldToRequest(req, addNoteInfo.sourceField, note.source); 
		req += "},\"options\":{\"allowDuplicate\":true}}}}";
		return req;
	}

	static void addFieldToRequest(std::string& req, std::string fieldName, std::string field) {
		req += "\"";
		req += fieldName;
		req += "\": \"";
		req += jsonCompatible(field);
		req += "\"";
	}

	static std::string jsonCompatible(std::string str) {
		std::string res;
		for (size_t i = 0; i < str.size(); i++) {
			if (!str::isPrintable(str[i]))
				continue;

			if (str[i] == '\"' || str[i] == '\\') {
				res += '\\';
				res += str[i];
			}
			else if (str[i] == '\n') {
				res += "\\n";
			}
			else
				res += str[i];
		}
		return res;
	}
};
