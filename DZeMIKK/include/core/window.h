#ifndef DZEMIKK_WINDOW_H
#define DZEMIKK_WINDOW_H

struct GLFWwindow;

namespace dzemikk {
    class Window {

    private:
        GLFWwindow* window_;

    public:
        Window();
        Window(int width, int height, const char* title);
        ~Window();

        bool shouldClose() const;
        void pollEvents() const;
        void swapBuffers() const;
        void clear(float r, float g, float b, float a) const;
    };
} // dzemikk

#endif // DZEMIKK_WINDOW_H