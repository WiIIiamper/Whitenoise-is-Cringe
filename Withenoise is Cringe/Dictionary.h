#pragma once
#include "simdjson.h"
#include "string_utils.h"
#include "WorkingDirectory.h"

#include <unordered_map>
#include <fstream>

class DictEntry {
public:
	std::string dict_name;
	std::string definition;
	DictEntry() {
		dict_name = "";
		definition = "";
	}

	DictEntry(std::string a, std::string b) {
		dict_name = a;
		definition = b;
	}
};

using namespace simdjson;

bool compare(std::pair < std::string, std::vector<DictEntry> > a,
			std::pair < std::string, std::vector<DictEntry> > b ) {
	if (str::compare(a.first, b.first) == 1)
		return true;
	return false;
}

class DictChunks {
public:
	bool loaded;
	std::string start;
	std::string end;
	
	DictChunks() {
		loaded = false;
	}
};

#include <stdio.h>
#include <stdlib.h>

class Dictionary {
	// Insted of loading the whole dictionaries at once,
	// we break them up into chunks and load them as needed.
	std::vector<DictChunks> chunks;

	std::unordered_map < std::string, std::vector <DictEntry> > dict;
	std::unordered_map < std::string, std::vector< std::string > > deconjugation;

	ondemand::parser parser;

public:
	Dictionary() {
		GetChunks();
		LoadDeconjucationDictionary();
	}

	std::vector<DictEntry> search(std::string term) {
		std::vector<DictEntry> entries;
		term = getWordRoot(term);

		auto idx = dict.find(term);
		if (idx != dict.end()) 
			entries = idx->second;

		return entries;
	}

	std::string getWordRoot(std::string word) {
		str::toLowercase(word);
		LoadDictionaryChunkIfNotLoaded(word);
		auto idx = dict.find(word);
		if (idx == dict.end()) {
			auto deconjugations = getPossibleDeconjugations(word);
			for (auto deconj : deconjugations) {
				auto idx = dict.find(deconj);
				if (idx != dict.end())
					return deconj;
			}
		}
		return word;
	}

private:
	void AddDictEntry(std::string dict_name, std::string& word, std::string& def) {
		DictEntry dictEntry(dict_name, def);

		auto idx = this->dict.find(word);
		if (idx == this->dict.end()) {
			std::vector <DictEntry> entries;
			entries.push_back(dictEntry);

			this->dict.insert(std::make_pair(word, entries));
		}
		else {
			idx->second.push_back(dictEntry);
		}
	}

	void LoadWordsetDictionary( int chunk ) {
		std::string dictPath = Wic::workingDir.Get()+ "assets\\dictionaries\\Wordset\\";
		dictPath += str::intToString(chunk);
		dictPath += ".json";

		padded_string json;
		auto error = padded_string::load(dictPath).get(json);
		if (error) {
			std::cerr << error << std::endl;
			return;
		}
		ondemand::document dict = parser.iterate(json);

		auto entries = dict.get_array();
		for (auto json_entry : entries) {
			auto entry = json_entry.get_object();

			std::string_view word = entry.find_field("t").get_string();
			std::string_view definition = entry.find_field("d").get_string();

			std::string wordStr(word);
			std::string defStr(definition);
			
			AddDictEntry("Wordset", wordStr, defStr);
		}
		chunks[chunk].loaded = true;
	}

	void LoadDeconjucationDictionary() {
		std::string path = Wic::workingDir.Get() + "assets\\dictionaries\\conjugation.json";
		padded_string json = padded_string::load(path);
		ondemand::document dict = parser.iterate(json);

		auto entries = dict.get_array();
		for (auto json_entry : entries) {
			auto entry = json_entry.get_object();
			
			auto dictForms = entry.find_field("dict").get_array();
			std::vector<std::string> dictionaryForms;
			for (auto form : dictForms) {
				std::string_view formStrView = form.get_string();
				std::string formString( formStrView );

				dictionaryForms.push_back(formString);
			}
			
			std::string_view inflection = entry.find_field("inflected").get_string();
			std::string inflected(inflection);

			auto idx = deconjugation.find(inflected);
			if (idx == deconjugation.end()) {
				deconjugation.insert(std::make_pair(inflected, dictionaryForms));
			}
			else {
				for (int i = 0; i < dictionaryForms.size(); i++) {
					idx->second.push_back(dictionaryForms[i]);
				}
			}
		}
	}

	std::vector<std::string> getPossibleDeconjugations(std::string term) {
		std::vector <std::string> deconjugations;

		for (auto idx : deconjugation) {
			std::string inflected = idx.first;
			if (str::endswith(term, inflected)) {
				std::string stem = term.substr(0, (int)term.size() - (int)inflected.size());

				for (auto dictForms : idx.second) 
					deconjugations.push_back(stem + dictForms);
			}
		}

		return deconjugations;
	}

	void LoadDictionaryChunkIfNotLoaded(std::string term) {
		int chunkidx = getChunk(term);
		if (chunkidx != -1 && chunks[chunkidx].loaded == false)
			LoadWordsetDictionary(chunkidx);
	}

	void GetChunks() {
		std::string path = Wic::workingDir.Get() + "assets\\dictionaries\\Table of Contents.txt";
		std::ifstream toc(path);
		char line[25];

		for (int i = 0; i <= 54; i++) {
			DictChunks chunk;
			toc.getline(line, 24);
			chunk.start = line;
			toc.getline(line, 24);
			chunk.end = line;

			chunks.push_back(chunk);
		}
	}

	int getChunk(std::string word) {
		for (int i = 0; i <= 54; i++) {
			if (str::compare(word, chunks[i].start) <= 0 &&
				str::compare(word, chunks[i].end) >= 0)
				return i;
		}
		return -1;
	}
};

namespace Wic {
	Dictionary dictionary;
}