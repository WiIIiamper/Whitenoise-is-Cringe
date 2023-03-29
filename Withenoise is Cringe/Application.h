#pragma once

#include "imgui_template.h"
#include "file_dialog.h"
#include "ebook.h"
#include "dict_popup.h"
#include "Dictionary.h"
#include "Anki.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class Application : public ImGuiApp {
private:
    Book* book;
    std::vector< DictPopup > popups;
public:
	void StartUp() {
        show_demo_window = false;
        book = NULL;
        Anki::init();
	}

	void Update() {
        // Show the ImGui Demo Window
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open")) {
                    openFileDialog(hwnd, book);
                }
                if (ImGui::MenuItem("Close")) {
                    if (book != NULL) {
                        book->Close();
                        delete book;
                        book = NULL;
                    }
                }

                ImGui::EndMenu();
            }

            /*if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }*/
            ImGui::EndMainMenuBar();
        }

        // Set fullscreen window
        static bool use_work_area = true;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

        // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
        // Based on your use case you may want one of the other.
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

        bool p_open = true;
        if (ImGui::Begin("Reader", &p_open, flags))
        {
            CaptureInput();

            if (book != NULL) {
                book->RenderBook();
            }
            if (popups.size() != 0) {
                bool open = popups[0].Render();
                if (!open)
                    popups.clear();
            }

            ImGui::End();
        }
	}

    void ShutDown() {
        if (book != NULL) {
            book->Close();
            delete book;
            book = NULL;
        }
    }

private:
    void CaptureInput() {
        ImGuiIO& io = ImGui::GetIO();
        bool hoveringPopup = HoveringPopup();

        // Scrolling
        if (io.MouseWheel != 0 && !hoveringPopup) {
            // Zoom
            if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                if (book != NULL) {
                    if (io.MouseWheel > 0)
                        book->ZoomIn();
                    else
                        book->ZoomOut();
                }
            }
            else {
                if (book != NULL) {
                    book->Scroll(io.MouseWheel);
                }
            }
            popups.clear();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_LeftShift)) {
            if (book != NULL) {
                ImVec2 pMin, pMax;
                std::string sentence;
                std::string word = book->GetWordAtPos(io.MousePos, pMin, pMax, sentence);
                popups.clear();
                if (word.size() != 0) {
                    std::string source = book->getName();
                    popups.push_back(DictPopup(word, pMin, pMax, sentence, source));
                }
            }
        }

        if (ImGui::IsKeyReleased(ImGuiKey_RightArrow)) {
            if (book != NULL) {
                book->NextPage();
            }
            popups.clear();
        }
        if (ImGui::IsKeyReleased(ImGuiKey_LeftArrow)) {
            if (book != NULL) {
                book->PrevPage();
            }
            popups.clear();
        }
    }

    bool HoveringPopup() {
        for (int i = 0; i < popups.size(); ++i) {
            if (popups[i].Hovered())
                return true;
        }
        return false;
    }
};
