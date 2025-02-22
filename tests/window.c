//========================================================================
// Window properties test
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

// TODO: Implement the postEmptyEvent test on multiple platforms. Works for Windows so far
#ifdef WIN32
#define USE_WIN32_THREAD_EMPTY_EVENT_TEST
#endif // WIN32

#ifdef USE_WIN32_THREAD_EMPTY_EVENT_TEST
#include <Windows.h>
#endif // USE_WIN32_THREAD_EMPTY_EVENT_TEST

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdarg.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifdef USE_WIN32_THREAD_EMPTY_EVENT_TEST

DWORD WINAPI win32ThreadEmptyEventTest(LPVOID lpThreadParameter)
{
    // Sleep for 3 seconds, and then post an empty event. The onEmptyEventPosted
    // method should then be fired shortly after, on the GLFW main thread.

    Sleep(3000);
    glfwPostEmptyEvent();
    return 0;
}

void onEmptyEventPosted(void)
{
    // If you hold your LMB down on the window border (aka initialise the resize
    // phase), you will see a big difference in stack trace here compared to
    // when not resizing. This is because DefWindowProc seems to use its own
    // message loop temporarily, and passes any unhandled messages back to the
    // dummy window's wndproc, which detects the empty event message, firing the
    // event callback.
    // This is the only effective way to run code on the main thread during the
    // window resize phase while the user isn't moving their mouse, apart from the
    // actual resize events obviously
    MessageBoxW((HWND)NULL, L"Callback event received!", L"Empty Event Callback", (UINT)0);
    // printf("Empty event received! Put a break point here, and this will be fired on the main thread");
}

#endif // USE_WIN32_THREAD_EMPTY_EVENT_TEST

int main(int argc, char** argv)
{
    int windowed_x, windowed_y, windowed_width, windowed_height;
    int last_xpos = INT_MIN, last_ypos = INT_MIN;
    int last_width = INT_MIN, last_height = INT_MIN;
    int limit_aspect_ratio = false, aspect_numer = 1, aspect_denom = 1;
    int limit_min_size = false, min_width = 400, min_height = 400;
    int limit_max_size = false, max_width = 400, max_height = 400;
    char width_buffer[12] = "", height_buffer[12] = "";
    char xpos_buffer[12] = "", ypos_buffer[12] = "";
    char numer_buffer[12] = "", denom_buffer[12] = "";
    char min_width_buffer[12] = "", min_height_buffer[12] = "";
    char max_width_buffer[12] = "", max_height_buffer[12] = "";
    int may_close = true;

    if (!glfwInit())
        exit(EXIT_FAILURE);

#ifdef USE_WIN32_THREAD_EMPTY_EVENT_TEST
    glfwSetEmptyEventCallback(onEmptyEventPosted);
#endif // USE_WIN32_THREAD_EMPTY_EVENT_TEST

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(750, 600, "Window Features", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0);

    bool position_supported = true;

    glfwGetError(NULL);
    glfwGetWindowPos(window, &last_xpos, &last_ypos);
    sprintf(xpos_buffer, "%i", last_xpos);
    sprintf(ypos_buffer, "%i", last_ypos);
    if (glfwGetError(NULL) == GLFW_FEATURE_UNAVAILABLE)
        position_supported = false;

    glfwGetWindowSize(window, &last_width, &last_height);
    sprintf(width_buffer, "%i", last_width);
    sprintf(height_buffer, "%i", last_height);

    sprintf(numer_buffer, "%i", aspect_numer);
    sprintf(denom_buffer, "%i", aspect_denom);

    sprintf(min_width_buffer, "%i", min_width);
    sprintf(min_height_buffer, "%i", min_height);
    sprintf(max_width_buffer, "%i", max_width);
    sprintf(max_height_buffer, "%i", max_height);

    struct nk_context* nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);

    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    while (!(may_close && glfwWindowShouldClose(window)))
    {
        int width, height;

        glfwGetWindowSize(window, &width, &height);

        struct nk_rect area = nk_rect(0.f, 0.f, (float)width, (float)height);
        nk_window_set_bounds(nk, "main", area);

        nk_glfw3_new_frame();
        if (nk_begin(nk, "main", area, 0))
        {
            nk_layout_row_dynamic(nk, 30, 6);

            if (nk_button_label(nk, "Toggle Fullscreen"))
            {
                if (glfwGetWindowMonitor(window))
                {
                    glfwSetWindowMonitor(window, NULL,
                        windowed_x, windowed_y,
                        windowed_width, windowed_height, 0);
                }
                else
                {
                    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    glfwGetWindowPos(window, &windowed_x, &windowed_y);
                    glfwGetWindowSize(window, &windowed_width, &windowed_height);
                    glfwSetWindowMonitor(window, monitor,
                        0, 0, mode->width, mode->height,
                        mode->refreshRate);
                }
            }

            if (nk_button_label(nk, "Maximize"))
                glfwMaximizeWindow(window);
            if (nk_button_label(nk, "Iconify"))
                glfwIconifyWindow(window);
            if (nk_button_label(nk, "Restore"))
                glfwRestoreWindow(window);
            if (nk_button_label(nk, "Hide (briefly)"))
            {
                glfwHideWindow(window);

                const double time = glfwGetTime() + 3.0;
                while (glfwGetTime() < time)
                    glfwWaitEventsTimeout(1.0);
                glfwShowWindow(window);
            }

#ifdef USE_WIN32_THREAD_EMPTY_EVENT_TEST
            if (nk_button_label(nk, "Empty Event(3s)"))
            {
                CreateThread(NULL, 0, win32ThreadEmptyEventTest, NULL, 0, NULL);
            }
#endif // USE_WIN32_THREAD_EMPTY_EVENT_TEST

            nk_layout_row_dynamic(nk, 30, 1);

            if (glfwGetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH))
            {
                nk_label(nk, "Press H to disable mouse passthrough", NK_TEXT_CENTERED);

                if (glfwGetKey(window, GLFW_KEY_H))
                    glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, false);
            }

            nk_label(nk, "Press Enter in a text field to set value", NK_TEXT_CENTERED);

            nk_flags events;
            const nk_flags flags = NK_EDIT_FIELD |
                NK_EDIT_SIG_ENTER |
                NK_EDIT_GOTO_END_ON_ACTIVATE;

            if (position_supported)
            {
                int xpos, ypos;
                glfwGetWindowPos(window, &xpos, &ypos);

                nk_layout_row_dynamic(nk, 30, 3);
                nk_label(nk, "Position", NK_TEXT_LEFT);

                events = nk_edit_string_zero_terminated(nk, flags, xpos_buffer,
                    sizeof(xpos_buffer),
                    nk_filter_decimal);
                if (events & NK_EDIT_COMMITED)
                {
                    xpos = atoi(xpos_buffer);
                    glfwSetWindowPos(window, xpos, ypos);
                }
                else if (xpos != last_xpos || (events & NK_EDIT_DEACTIVATED))
                    sprintf(xpos_buffer, "%i", xpos);

                events = nk_edit_string_zero_terminated(nk, flags, ypos_buffer,
                    sizeof(ypos_buffer),
                    nk_filter_decimal);
                if (events & NK_EDIT_COMMITED)
                {
                    ypos = atoi(ypos_buffer);
                    glfwSetWindowPos(window, xpos, ypos);
                }
                else if (ypos != last_ypos || (events & NK_EDIT_DEACTIVATED))
                    sprintf(ypos_buffer, "%i", ypos);

                last_xpos = xpos;
                last_ypos = ypos;
            }
            else
                nk_label(nk, "Position not supported", NK_TEXT_LEFT);

            nk_layout_row_dynamic(nk, 30, 3);
            nk_label(nk, "Size", NK_TEXT_LEFT);

            events = nk_edit_string_zero_terminated(nk, flags, width_buffer,
                sizeof(width_buffer),
                nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                width = atoi(width_buffer);
                glfwSetWindowSize(window, width, height);
            }
            else if (width != last_width || (events & NK_EDIT_DEACTIVATED))
                sprintf(width_buffer, "%i", width);

            events = nk_edit_string_zero_terminated(nk, flags, height_buffer,
                sizeof(height_buffer),
                nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                height = atoi(height_buffer);
                glfwSetWindowSize(window, width, height);
            }
            else if (height != last_height || (events & NK_EDIT_DEACTIVATED))
                sprintf(height_buffer, "%i", height);

            last_width = width;
            last_height = height;

            bool update_ratio_limit = false;
            if (nk_checkbox_label(nk, "Aspect Ratio", &limit_aspect_ratio))
                update_ratio_limit = true;

            events = nk_edit_string_zero_terminated(nk, flags, numer_buffer,
                sizeof(numer_buffer),
                nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                aspect_numer = abs(atoi(numer_buffer));
                update_ratio_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(numer_buffer, "%i", aspect_numer);

            events = nk_edit_string_zero_terminated(nk, flags, denom_buffer,
                sizeof(denom_buffer),
                nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                aspect_denom = abs(atoi(denom_buffer));
                update_ratio_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(denom_buffer, "%i", aspect_denom);

            if (update_ratio_limit)
            {
                if (limit_aspect_ratio)
                    glfwSetWindowAspectRatio(window, aspect_numer, aspect_denom);
                else
                    glfwSetWindowAspectRatio(window, GLFW_DONT_CARE, GLFW_DONT_CARE);
            }

            bool update_size_limit = false;

            if (nk_checkbox_label(nk, "Minimum Size", &limit_min_size))
                update_size_limit = true;

            events = nk_edit_string_zero_terminated(nk, flags, min_width_buffer,
                sizeof(min_width_buffer),
                nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                min_width = abs(atoi(min_width_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(min_width_buffer, "%i", min_width);

            events = nk_edit_string_zero_terminated(nk, flags, min_height_buffer,
                sizeof(min_height_buffer),
                nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                min_height = abs(atoi(min_height_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(min_height_buffer, "%i", min_height);

            if (nk_checkbox_label(nk, "Maximum Size", &limit_max_size))
                update_size_limit = true;

            events = nk_edit_string_zero_terminated(nk, flags, max_width_buffer,
                sizeof(max_width_buffer),
                nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                max_width = abs(atoi(max_width_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(max_width_buffer, "%i", max_width);

            events = nk_edit_string_zero_terminated(nk, flags, max_height_buffer,
                sizeof(max_height_buffer),
                nk_filter_decimal);
            if (events & NK_EDIT_COMMITED)
            {
                max_height = abs(atoi(max_height_buffer));
                update_size_limit = true;
            }
            else if (events & NK_EDIT_DEACTIVATED)
                sprintf(max_height_buffer, "%i", max_height);

            if (update_size_limit)
            {
                glfwSetWindowSizeLimits(window,
                    limit_min_size ? min_width : GLFW_DONT_CARE,
                    limit_min_size ? min_height : GLFW_DONT_CARE,
                    limit_max_size ? max_width : GLFW_DONT_CARE,
                    limit_max_size ? max_height : GLFW_DONT_CARE);
            }

            int fb_width, fb_height;
            glfwGetFramebufferSize(window, &fb_width, &fb_height);
            nk_label(nk, "Framebuffer Size", NK_TEXT_LEFT);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", fb_width);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", fb_height);

            float xscale, yscale;
            glfwGetWindowContentScale(window, &xscale, &yscale);
            nk_label(nk, "Content Scale", NK_TEXT_LEFT);
            nk_labelf(nk, NK_TEXT_LEFT, "%f", xscale);
            nk_labelf(nk, NK_TEXT_LEFT, "%f", yscale);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 5);
            int frame_left, frame_top, frame_right, frame_bottom;
            glfwGetWindowFrameSize(window, &frame_left, &frame_top, &frame_right, &frame_bottom);
            nk_layout_row_push(nk, 1.f / 3.f);
            nk_label(nk, "Frame Size:", NK_TEXT_LEFT);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_left);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_top);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_right);
            nk_layout_row_push(nk, 1.f / 6.f);
            nk_labelf(nk, NK_TEXT_LEFT, "%i", frame_bottom);
            nk_layout_row_end(nk);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 2);
            float opacity = glfwGetWindowOpacity(window);
            nk_layout_row_push(nk, 1.f / 3.f);
            nk_labelf(nk, NK_TEXT_LEFT, "Opacity: %0.3f", opacity);
            nk_layout_row_push(nk, 2.f / 3.f);
            if (nk_slider_float(nk, 0.f, &opacity, 1.f, 0.001f))
                glfwSetWindowOpacity(window, opacity);
            nk_layout_row_end(nk);

            nk_layout_row_begin(nk, NK_DYNAMIC, 30, 2);
            int should_close = glfwWindowShouldClose(window);
            nk_layout_row_push(nk, 1.f / 3.f);
            if (nk_checkbox_label(nk, "Should Close", &should_close))
                glfwSetWindowShouldClose(window, should_close);
            nk_layout_row_push(nk, 2.f / 3.f);
            nk_checkbox_label(nk, "May Close", &may_close);
            nk_layout_row_end(nk);

            nk_layout_row_dynamic(nk, 30, 1);
            nk_label(nk, "Attributes", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(nk, 30, width > 200 ? width / 200 : 1);

            int decorated = glfwGetWindowAttrib(window, GLFW_DECORATED);
            if (nk_checkbox_label(nk, "Decorated", &decorated))
                glfwSetWindowAttrib(window, GLFW_DECORATED, decorated);

            int resizable = glfwGetWindowAttrib(window, GLFW_RESIZABLE);
            if (nk_checkbox_label(nk, "Resizable", &resizable))
                glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizable);

            int floating = glfwGetWindowAttrib(window, GLFW_FLOATING);
            if (nk_checkbox_label(nk, "Floating", &floating))
                glfwSetWindowAttrib(window, GLFW_FLOATING, floating);

            int passthrough = glfwGetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH);
            if (nk_checkbox_label(nk, "Mouse Passthrough", &passthrough))
                glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, passthrough);

            int auto_iconify = glfwGetWindowAttrib(window, GLFW_AUTO_ICONIFY);
            if (nk_checkbox_label(nk, "Auto Iconify", &auto_iconify))
                glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, auto_iconify);

            nk_value_bool(nk, "Focused", glfwGetWindowAttrib(window, GLFW_FOCUSED));
            nk_value_bool(nk, "Hovered", glfwGetWindowAttrib(window, GLFW_HOVERED));
            nk_value_bool(nk, "Visible", glfwGetWindowAttrib(window, GLFW_VISIBLE));
            nk_value_bool(nk, "Iconified", glfwGetWindowAttrib(window, GLFW_ICONIFIED));
            nk_value_bool(nk, "Maximized", glfwGetWindowAttrib(window, GLFW_MAXIMIZED));
        }
        nk_end(nk);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(window);

        glfwWaitEvents();
    }

    nk_glfw3_shutdown();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

