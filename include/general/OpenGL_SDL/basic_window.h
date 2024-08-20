//
// Created by Palnit on 2023. 11. 11.
//

#ifndef BASIC_WINDOW_H_
#define BASIC_WINDOW_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rwops.h>

#include <GL/glew.h>

#include <glm/glm.hpp>

#include <imgui.h>
#include <implot.h>

/*!
 * \class BasicWindow
 * \brief An SDL2-Opengl-ImGui window generating virtual class should be inherited
 *
 * Creates and checks for errors during window and opengl context creation its a
 * its a class that's designed to be inherited from to completely abstract the sdl2
 * - opengl - ImGui window creation boiler plate and provide easy access to it's
 * variables while having the convenience to not pollute your code with
 * having to implement all keyboard functions even if you dont use them
 */
class BasicWindow {

public:
    /*!
     * Constructor
     * \param title The title of the window
     * \param x The horizontal position of the window
     * \param y The vertical position of the window
     * \param width The width of the window
     * \param height The height of the window
     * \param flags Flags for the sdl window creation function SDL_WINDOW_OPENGL
     * is always appended
     */

    BasicWindow(const char* title,
                int x,
                int y,
                int width,
                int height,
                uint32_t flags);

    ~BasicWindow();

    int InitWindow();
    int run();

    /*!
     * Virtual function \n
     * Event handling function for the SDL Keyboard Down event
     * \param ev The events data
     */
    virtual void KeyboardDown(const SDL_KeyboardEvent& ev) {};

    /*!
     * Virtual function \n
     * Event handling function for the SDL Keyboard Up event
     * \param ev The events data
     */
    virtual void KeyboardUp(const SDL_KeyboardEvent& ev) {};

    /*!
     * Virtual function \n
     * Event handling function for the SDL Mouse Button Down event
     * \param ev The events data
     */
    virtual void MouseDown(const SDL_MouseButtonEvent& ev) {};

    /*!
     * Virtual function \n
     * Event handling function for the SDL Mouse Button Up event
     * \param ev The events data
     */
    virtual void MouseUp(const SDL_MouseButtonEvent& ev) {};

    /*!
     * Virtual function \n
     * Event handling function for the SDL Mouse Wheel event
     * \param ev The events data
     */
    virtual void MouseWheel(const SDL_MouseWheelEvent& ev) {};

    /*!
     * Virtual function \n
     * Event handling function for the SDL Mouse Motion event
     * \param ev The events data
     */
    virtual void MouseMove(const SDL_MouseMotionEvent& ev) {};

    /*!
     * Virtual function \n
     * Event handling function for the SDL Window Event Size Change event
     * The new sizes of the window is stored int he protected variables m_width
     * and m_height
     */
    virtual void Resize() {};

    /*!
     * Virtual function \n
     * Initialization function called at the very beginning of the main loop
     */
    virtual int Init() { return 0; };

    /*!
     * Virtual function \n
     * Update function called at every iteration of the main loop
     */
    virtual void Update() {};

    /*!
    * Virtual function \n
    * Rendering function called at every iteration of the main loop
    */
    virtual void Render() {
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
    };

    /*!
    * Virtual function \n
    * ImGui Rendering function called at every iteration of the main loop
    */
    virtual void RenderImGui() {
        bool render = true;
        ImGui::ShowDemoWindow(&render);
    };

protected:
    const char* m_title;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    bool m_running;
    uint32_t m_flags;
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_context;
    SDL_Event m_ev;
};

#endif//BASIC_WINDOW_H_
