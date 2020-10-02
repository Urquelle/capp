#include <windows.h>

typedef struct {
    HDC  dc;
    HWND window_handle;
} Os_Win32_Meta;

/* win32_update_button {{{ */
void
win32_update_button(Os_Button *button, bool down, bool shift) {
    bool was_down      = button->down;
    button->down      = down;
    button->pressed   = !was_down && down;
    button->released  = was_down && !down;
    button->lowercase = !shift;
    button->uppercase = shift;
}
/* }}} */
/* win32_main_window_callback {{{ */
LRESULT CALLBACK
win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;

    Os *os = (Os *)GetWindowLongPtrA(window, GWLP_USERDATA);

    switch (message) {
        case WM_DESTROY: {
            os->quit = true;
        } break;

        case WM_SIZE: {
            os->window.dim.width  = (short) LOWORD(l_param);
            os->window.dim.height = (short) HIWORD(l_param);
        } break;

        case WM_MOVE: {
            os->window.pos.x = (short) LOWORD(l_param);
            os->window.pos.y = (short) HIWORD(l_param);
        }; break;

        case WM_PAINT: {
            result = DefWindowProc(window, message, w_param, l_param);
        } break;

        case WM_CHAR: {
            /* @AUFGABE: virtual keys ebenfalls übernehmen */
            CHAR character = (CHAR)w_param;

            os->chars[os->num_chars] = character;
            os->chars[os->num_chars + 1] = 0;
            os->num_chars += 1;
        } break;

        case WM_TIMER: {
            // SwitchToFiber(app->win32.main_fiber);
        } break;

        case WM_ENTERMENULOOP:
        case WM_ENTERSIZEMOVE: {
            SetTimer(window, 1, 1, 0);
        } break;

        case WM_EXITMENULOOP:
        case WM_EXITSIZEMOVE: {
            KillTimer(window, 1);
        } break;

        case WM_INPUT: {
            UINT size;
            GetRawInputData((HRAWINPUT)l_param, RID_INPUT, 0, &size, sizeof(RAWINPUTHEADER));
            void* buffer = _alloca(size);

            if (GetRawInputData((HRAWINPUT)l_param, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) == size) {
                RAWINPUT *raw_input = (RAWINPUT *)buffer;

                if (raw_input->header.dwType == RIM_TYPEMOUSE && raw_input->data.mouse.usFlags == MOUSE_MOVE_RELATIVE) {
                    os->mouse.dpos.x += raw_input->data.mouse.lLastX;
                    os->mouse.dpos.y += raw_input->data.mouse.lLastY;

                    USHORT button_flags = raw_input->data.mouse.usButtonFlags;

                    // left button {{{
                    bool left_button_down = os->mouse.left.down;
                    if (button_flags & RI_MOUSE_LEFT_BUTTON_DOWN) {
                        left_button_down = true;
                    }

                    if (button_flags & RI_MOUSE_LEFT_BUTTON_UP) {
                        left_button_down = false;
                    }
                    win32_update_button(&os->mouse.left, left_button_down, false);
                    // }}}
                    // right button {{{
                    bool right_button_down = os->mouse.right.down;
                    if (button_flags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
                        right_button_down = true;
                    }

                    if (button_flags & RI_MOUSE_RIGHT_BUTTON_UP) {
                        right_button_down = false;
                    }
                    win32_update_button(&os->mouse.right, right_button_down, false);
                    // }}}
                    // middle button {{{
                    bool middle_button_down = os->mouse.middle.down;
                    if (button_flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
                        middle_button_down = true;
                    }

                    if (button_flags & RI_MOUSE_MIDDLE_BUTTON_UP) {
                        middle_button_down = false;
                    }
                    win32_update_button(&os->mouse.middle, middle_button_down, false);
                    // }}}

                    if (button_flags & RI_MOUSE_WHEEL) {
                        os->mouse.delta_wheel += ((SHORT)raw_input->data.mouse.usButtonData) / WHEEL_DELTA;
                    }
                }
            }

            result = DefWindowProc(window, message, w_param, l_param);
        } break;

        default: {
            result = DefWindowProc(window, message, w_param, l_param);
        }
    }

    return result;
}
/* }}} */
/* os api {{{ */
/* init {{{ */
OS_API_INIT() {
    Os_Win32_Meta *meta = (Os_Win32_Meta *)malloc(sizeof(Os_Win32_Meta));
    os->meta = meta;

    char CLASS_NAME[]  = "app_window_class";

    WNDCLASS wc = {0};

    wc.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    wc.lpfnWndProc   = win32_main_window_callback;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    meta->window_handle = CreateWindowEx(
        0,
        CLASS_NAME,
        "ui",
        WS_OVERLAPPEDWINDOW,

        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,
        NULL,
        NULL,
        NULL
    );

    if (!meta->window_handle) {
        os->msg = "grafisches fenster konnte nicht erstellt werden";

        return OS_FAILURE;
    }

    /*
    meta->main_fiber    = ConvertThreadToFiber(0);
    meta->message_fiber = CreateFiber(0, (PFIBER_START_ROUTINE)window_message_fiber_proc, app);
    */

    SetWindowLongPtr(meta->window_handle, GWLP_USERDATA, (LONG_PTR)os);
    ShowWindow(meta->window_handle, SW_SHOW);
    UpdateWindow(meta->window_handle);
    meta->dc = GetDC(meta->window_handle);

    /*
    meta->backbuffer.bytes_per_pixel = 4;
    win32_resize_dib_section(app, app->window.size.width, app->window.size.height);

    Win32_Backbuffer *win_buffer = &app->win32.backbuffer;
    Bitmap           *app_buffer = &app->graphics.backbuffer;

    app_buffer->width           = win_buffer->width;
    app_buffer->height          = win_buffer->height;
    app_buffer->pitch           = win_buffer->pitch;
    app_buffer->bytes_per_pixel = win_buffer->bytes_per_pixel;
    app_buffer->data            = win_buffer->data;
    */

    RAWINPUTDEVICE raw_input_device = {0};

    raw_input_device.usUsagePage = 0x01;
    raw_input_device.usUsage     = 0x02;
    raw_input_device.hwndTarget  = meta->window_handle;

    if (!RegisterRawInputDevices(&raw_input_device, 1, sizeof(raw_input_device))) {
        os->msg = "maus konnte nicht eingebunden werden";

        return OS_FAILURE;
    }

    return OS_SUCCESS;
}
/* }}} */
/* pull {{{ */
OS_API_PULL() {
    Os_Win32_Meta *meta = (Os_Win32_Meta *)os->meta;
    MSG msg;

    /* ereignisbehandlung */
    GetMessage( &msg, NULL, 0, 0 );
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    /* fenstergröße */
    RECT client_rect;
    GetClientRect(meta->window_handle, &client_rect);

    POINT window_pos = {client_rect.left, client_rect.top};
    ClientToScreen(meta->window_handle, &window_pos);

    os->window.pos.x = (float)window_pos.x;
    os->window.pos.y = (float)window_pos.y;

    /* mauszeiger */
    POINT mouse_pos;
    GetCursorPos(&mouse_pos);

    os->mouse.pos.x = mouse_pos.x - os->window.pos.x;
    os->mouse.pos.y = mouse_pos.y - os->window.pos.y;
}
/* }}} */
/* push {{{ */
OS_API_PUSH() {
}
/* }}} */
/* readfile {{{ */
OS_API_READFILE() {
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    OVERLAPPED ol = {0};

    if ( file == INVALID_HANDLE_VALUE ) {
        return false;
    }

    if ( size ) {
        *size = GetFileSize(file, 0);
    }

    *result = (char *)malloc(*size);
    HRESULT h = ReadFileEx(file, *result, (DWORD)(*size), &ol, NULL);
    if ( FAILED(h) ) {
        CloseHandle(file);
        free(*result);
        *result = 0;

        return false;
    }

    return true;
}
/* }}} */
/* writefile {{{ */
OS_API_WRITEFILE() {
    HANDLE file = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if ( file == INVALID_HANDLE_VALUE ) {
        return false;
    }

    DWORD bytes_written = 0;
    HRESULT h = WriteFile(file, data, (DWORD)len, &bytes_written, NULL);

    CloseHandle(file);

    return SUCCEEDED(h);
}
/* }}} */
/* }}} */

