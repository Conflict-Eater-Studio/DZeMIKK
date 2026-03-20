#include <core/engine.h>
#include <extern/imgui/imgui.h>
int main() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    dzemikk::Engine e;
}