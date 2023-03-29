#pragma once
#include "imgui.h"
#include "Dictionary.h"
#include "Anki.h"

#include <iostream>

class DictPopup{
	ImVec2 pos;
	ImGuiWindowFlags flags;

	std::pair<ImVec2, ImVec2> wordBbox;
	std::string wordRaw;
	std::string wordRoot;
	std::string sentence;
	std::string source;
	bool p_open;

	std::vector<DictEntry> entries;

public:
	DictPopup(std::string w, ImVec2 pMin, ImVec2 pMax, std::string sentence, std::string source) {
		wordRaw = w;
		wordBbox = std::make_pair(pMin, pMax);
		this->sentence = sentence;
		this->source = source;

		p_open = true;
		flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_HorizontalScrollbar;
		setAdequatePos();

		wordRoot = Wic::dictionary.getWordRoot(w);
		entries = Wic::dictionary.search(wordRoot);
	}

	bool Render() {
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(ImVec2(400, 250));

        if (ImGui::Begin( wordRoot.c_str(), &p_open, flags))
        {
			RenderDictionaryEntries();
			if (ImGui::Button("Add to Anki")) {
				CreateAnkiNote();
			}
			RenderSearchOnlineButtons();
        }
		ImGui::End();
		
		return p_open;
	}

	bool Hovered() {
		auto io = ImGui::GetIO();
		if (io.MousePos.y > pos.y && io.MousePos.y < pos.y + 250
			&& io.MousePos.x > pos.x && io.MousePos.x < pos.x + 400)
			return true;
		return false;
	}

private:
	void CreateAnkiNote() {
		AnkiNote anki_note;
		anki_note.term = wordRoot;
		AddGlossaryToNote(anki_note);
		anki_note.sentence = sentence;
		anki_note.source = source;
		Anki::AddNote(anki_note);
	}

	void AddGlossaryToNote(AnkiNote& anki_note) {
		anki_note.glossary = "";
		for (auto dictEntry : entries) {
			anki_note.glossary += dictEntry.definition;
			anki_note.glossary += '\n';
		}
	}

	void RenderDictionaryEntries() {
		for (int i = 0; i < entries.size(); i++) {
			std::string dict_name = entries[i].dict_name;
			std::string definition = entries[i].definition;

			ImGui::Text(dict_name.c_str());
			ImGui::Separator();
			ImGui::Text(definition.c_str());
			ImGui::Separator();
		}
	}

	void RenderSearchOnlineButtons() {
		std::string str = "Search \"" + wordRaw + "\" on Google.";
		if (ImGui::Button(str.c_str())) {
			std::string url = "start https://www.google.com/search?q=" + wordRaw;
			system(url.c_str());
		}

		str = "Search \"" + wordRaw + " meaning\" on Google.";
		if (ImGui::Button(str.c_str())) {
			std::string url = "start https://www.google.com/search?q=" + wordRaw + "+meaning";
			system(url.c_str());
		}

		str = "Search \"" + wordRaw + "\" on Urban Dictionary.";
		if (ImGui::Button(str.c_str())) {
			std::string url = "start https://www.urbandictionary.com/define.php?term=" + wordRaw;
			system(url.c_str());
		}
	}

	void setAdequatePos() {
		ImVec2 windowPos = ImGui::GetCursorScreenPos();
		ImVec2 avail_size = ImGui::GetContentRegionAvail();

		if (wordBbox.second.y + 250 <= windowPos.y + avail_size.y) {
			pos.y = wordBbox.second.y; // place the popup under the word
		}
		else if (wordBbox.first.y - 250 >= windowPos.y) {
			pos.y = wordBbox.first.y - 250; // place the popup above the word
		}
		else {
			pos.y = wordBbox.second.y; // place the popup under the word
		}

		if (wordBbox.first.x + 400 <= windowPos.x + avail_size.x) {
			pos.x = wordBbox.first.x; // place the popup on the leftside of the word
		}
		else if (wordBbox.second.x - 400 >= windowPos.x) {
			pos.x = wordBbox.second.x - 400; // place the popup above the word
		}
		else {
			pos.x = wordBbox.first.x; // place the popup on the leftside of the word
		}
	}
};