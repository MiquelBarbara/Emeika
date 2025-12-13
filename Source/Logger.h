#pragma once


static char* Strdup(const char* s) { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = ImGui::MemAlloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }

class Logger
{
public:
    Logger()
    {
        Clear();
        InputBuf[0] = '\0';
    }

    void Clear()
    {
        for (int i = 0; i < Items.Size; i++)
            ImGui::MemFree(Items[i]);
        Items.clear();
    }

    void AddLog(const char* msg, ...)  IM_FMTARGS(2)
    {
        char buf[1024];
        va_list args;
        va_start(args, msg);
        vsnprintf(buf, IM_ARRAYSIZE(buf), msg, args);
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
    }

    void Draw(const char* title, bool* p_open = NULL)
    {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // Display log scrolling region
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (int i = 0; i < Items.Size; i++)
            ImGui::TextUnformatted(Items[i]);

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();

        // Input box
        ImGui::Separator();
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (InputBuf[0] != '\0')
            {
                AddLog(InputBuf);
                InputBuf[0] = '\0';
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
            Clear();

        ImGui::End();
    }
        
private:
    char InputBuf[256];
    ImVector<char*> Items;
    bool AutoScroll = true;

};

extern Logger* logger;

