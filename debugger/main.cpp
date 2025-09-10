/**
 * @file font_display.c
 * @brief An SDL2 program implementing a scrollable, resizable, virtual text-mode screen.
 *
 * This program loads a classic DOS bitmap font and provides an API to "print"
 * characters to a large in-memory buffer. A smaller portion of this buffer (the viewport)
 * is rendered to the screen. The user can scroll this viewport using the mouse wheel
 * or by dragging the OS/2-style scrollbars.
 *
 * How it works:
 * 1.  The scrollbars are now a UI overlay drawn directly to the renderer, separate
 * from the main text content buffer. This fixes the "scrollbar trail" bug.
 * 2.  The virtual `TextScreen` holds only the content, not UI elements.
 * 3.  The main loop handles mouse events to update scrollbar state.
 * 4.  A `draw_scrollbars` function is now responsible for both drawing the UI
 * and calculating the clickable thumb rectangles for mouse detection.
 * 5.  The `render_text_screen` function is simplified to only render the
 * visible portion of the text buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h> // For round() and ceil()
#include <SDL2/SDL.h>

// --- Configuration ---
const char* FONT_FILENAME = "VGA9.F16";
const int FONT_CHAR_WIDTH = 8;
const int FONT_CHAR_HEIGHT = 16;
const int FONT_NUM_CHARS = 256;
const int FONT_BYTES_PER_CHAR = 16;

// --- Viewport Configuration ---
// The number of character cells visible in the window.
const int VIEW_COLS = 100;
const int VIEW_ROWS = 40;

// --- Data Structures ---
typedef struct {
    unsigned char character_code;
    SDL_Color fg_color;
    SDL_Color bg_color;
} VGAChar;

typedef struct {
    int cols;
    int rows;
    VGAChar* buffer;
} TextScreen;

// New struct to manage scrollbar state
typedef struct {
    float view_pos;       // Current scroll position (top/left of the view)
    int view_size;        // How many characters are visible
    int content_size;     // Total number of characters in the content
    SDL_Rect thumb_rect;  // Screen pixel coordinates of the thumb, for mouse picking
    int is_dragging;      // Flag for mouse interaction
} Scrollbar;

// --- Global State ---
SDL_Color dos_palette[16];
Scrollbar h_scrollbar;
Scrollbar v_scrollbar;

// --- API & Helper Function Prototypes ---
void init_dos_palette();
int load_font(const char* filename, unsigned char* buffer);
TextScreen* create_text_screen(int cols, int rows);
void destroy_text_screen(TextScreen* screen);
void clear_screen(TextScreen* screen, VGAChar clear_char);
void print_string(TextScreen* screen, int x, int y, const char* text, SDL_Color fg, SDL_Color bg);
void set_char(TextScreen* screen, int x, int y, unsigned char chr, SDL_Color fg, SDL_Color bg);
void draw_scrollbars(SDL_Renderer* renderer, const unsigned char* font_data);
void render_text_screen(SDL_Renderer* renderer, TextScreen* screen, const unsigned char* font_data);
void draw_char_on_renderer(SDL_Renderer* renderer, const unsigned char* font_data, unsigned char chr, int cx, int cy, SDL_Color fg, SDL_Color bg, float cell_w, float cell_h);


// --- Main Program ---
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow( "Virtual Text Mode", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, VIEW_COLS * FONT_CHAR_WIDTH * 2, VIEW_ROWS * FONT_CHAR_HEIGHT * 2, SDL_WINDOW_RESIZABLE );
    if (!window) { /* ... error handling ... */ return 1; }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) { /* ... error handling ... */ return 1; }

    init_dos_palette();

    const int FONT_DATA_SIZE = FONT_NUM_CHARS * FONT_BYTES_PER_CHAR;
    unsigned char font_data[FONT_DATA_SIZE];
    if (!load_font(FONT_FILENAME, font_data)) { /* ... cleanup ... */ return 1; }

    // Create a virtual screen much larger than the view
    TextScreen* screen = create_text_screen(VIEW_COLS, VIEW_ROWS);
    if (!screen) { /* ... cleanup ... */ return 1; }

    // --- Initialize Scrollbars ---
    h_scrollbar = (Scrollbar){ .view_pos = 0, .view_size = VIEW_COLS, .content_size = screen->cols, .is_dragging = 0 };
    v_scrollbar = (Scrollbar){ .view_pos = 0, .view_size = VIEW_ROWS, .content_size = screen->rows, .is_dragging = 0 };


    // --- Draw a Larger Demo Screen ---
    VGAChar clear_char = { 176, dos_palette[8], dos_palette[0] }; // Gray pattern on Black
    clear_screen(screen, clear_char);

    for (int i = 0; i < 200; ++i) {
        char buffer[10];
        sprintf(buffer, "%d", i);
        print_string(screen, i, i % 50, buffer, dos_palette[i % 16], dos_palette[(i+1) % 16]);
    }
    print_string(screen, 2, 2, "This text buffer is 200 columns wide and 100 rows tall.", dos_palette[15], dos_palette[4]);
    print_string(screen, 2, 3, "Use the mouse wheel or drag the scrollbars to navigate.", dos_palette[15], dos_palette[4]);


    // --- Main Loop ---
    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            // --- Handle Mouse Input for Scrolling ---
            else if (event.type == SDL_MOUSEWHEEL) {
                v_scrollbar.view_pos -= event.wheel.y * 2; // Scroll 2 chars at a time
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouse_x = event.button.x;
                    int mouse_y = event.button.y;
                    if (mouse_x >= h_scrollbar.thumb_rect.x && mouse_x < h_scrollbar.thumb_rect.x + h_scrollbar.thumb_rect.w &&
                        mouse_y >= h_scrollbar.thumb_rect.y && mouse_y < h_scrollbar.thumb_rect.y + h_scrollbar.thumb_rect.h) {
                        h_scrollbar.is_dragging = 1;
                    }
                     if (mouse_x >= v_scrollbar.thumb_rect.x && mouse_x < v_scrollbar.thumb_rect.x + v_scrollbar.thumb_rect.w &&
                        mouse_y >= v_scrollbar.thumb_rect.y && mouse_y < v_scrollbar.thumb_rect.y + v_scrollbar.thumb_rect.h) {
                        v_scrollbar.is_dragging = 1;
                    }
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    h_scrollbar.is_dragging = 0;
                    v_scrollbar.is_dragging = 0;
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                if (h_scrollbar.is_dragging) {
                    int window_w;
                    SDL_GetRendererOutputSize(renderer, &window_w, NULL);
                    float char_w = (float)window_w / VIEW_COLS;
                    float track_len_pixels = char_w * (VIEW_COLS - 2);
                    if (track_len_pixels > 0) {
                        h_scrollbar.view_pos += event.motion.xrel * (h_scrollbar.content_size / track_len_pixels);
                    }
                }
                if (v_scrollbar.is_dragging) {
                    int window_h;
                    SDL_GetRendererOutputSize(renderer, NULL, &window_h);
                    float char_h = (float)window_h / VIEW_ROWS;
                    float track_len_pixels = char_h * (VIEW_ROWS - 2);
                    if (track_len_pixels > 0) {
                        v_scrollbar.view_pos += event.motion.yrel * (v_scrollbar.content_size / track_len_pixels);
                    }
                }
            }
        }

        // Clamp scroll positions
        if (h_scrollbar.view_pos < 0) h_scrollbar.view_pos = 0;
        if (h_scrollbar.view_pos > h_scrollbar.content_size - h_scrollbar.view_size) h_scrollbar.view_pos = h_scrollbar.content_size - h_scrollbar.view_size;
        if (v_scrollbar.view_pos < 0) v_scrollbar.view_pos = 0;
        if (v_scrollbar.view_pos > v_scrollbar.content_size - v_scrollbar.view_size) v_scrollbar.view_pos = v_scrollbar.content_size - v_scrollbar.view_size;

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // 1. Render the text content from the buffer
        render_text_screen(renderer, screen, font_data);
        
        // 2. Render the UI overlay (scrollbars) on top
        //draw_scrollbars(renderer, font_data);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    // --- Cleanup ---
    destroy_text_screen(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


// --- API and Helper Function Implementations ---

void init_dos_palette() {
    dos_palette[0]  = (SDL_Color){0, 0, 0, 255};       // Black
    dos_palette[1]  = (SDL_Color){0, 0, 170, 255};     // Blue
    dos_palette[2]  = (SDL_Color){0, 170, 0, 255};     // Green
    dos_palette[3]  = (SDL_Color){0, 170, 170, 255};   // Cyan
    dos_palette[4]  = (SDL_Color){170, 0, 0, 255};     // Red
    dos_palette[5]  = (SDL_Color){170, 0, 170, 255};   // Magenta
    dos_palette[6]  = (SDL_Color){170, 85, 0, 255};    // Brown/Yellow
    dos_palette[7]  = (SDL_Color){170, 170, 170, 255}; // Light Gray
    dos_palette[8]  = (SDL_Color){85, 85, 85, 255};    // Dark Gray
    dos_palette[9]  = (SDL_Color){85, 85, 255, 255};   // Light Blue
    dos_palette[10] = (SDL_Color){85, 255, 85, 255};   // Light Green
    dos_palette[11] = (SDL_Color){85, 255, 255, 255};  // Light Cyan
    dos_palette[12] = (SDL_Color){255, 85, 85, 255};   // Light Red
    dos_palette[13] = (SDL_Color){255, 85, 255, 255};  // Light Magenta
    dos_palette[14] = (SDL_Color){255, 255, 85, 255};  // Yellow
    dos_palette[15] = (SDL_Color){255, 255, 255, 255}; // White
}

int load_font(const char* filename, unsigned char* buffer) {
    const int FONT_DATA_SIZE = FONT_NUM_CHARS * FONT_BYTES_PER_CHAR;
    FILE* font_file = fopen(filename, "rb");
    if (!font_file) {
        fprintf(stderr, "Error opening font file: %s\n", filename);
        return 0;
    }
    size_t bytes_read = fread(buffer, 1, FONT_DATA_SIZE, font_file);
    fclose(font_file);
    if (bytes_read != FONT_DATA_SIZE) {
        fprintf(stderr, "Error: Read %zu bytes, but expected %d bytes from font file.\n", bytes_read, FONT_DATA_SIZE);
        return 0;
    }
    printf("Successfully loaded font file '%s'.\n", filename);
    return 1;
}

TextScreen* create_text_screen(int cols, int rows) {
    TextScreen* screen = (TextScreen*)malloc(sizeof(TextScreen));
    if (!screen) return NULL;
    screen->cols = cols;
    screen->rows = rows;
    screen->buffer = (VGAChar*)malloc(sizeof(VGAChar) * cols * rows);
    if (!screen->buffer) {
        free(screen);
        return NULL;
    }
    return screen;
}

void destroy_text_screen(TextScreen* screen) {
    if (screen) {
        free(screen->buffer);
        free(screen);
    }
}

void clear_screen(TextScreen* screen, VGAChar clear_char) {
    int total_cells = screen->cols * screen->rows;
    for (int i = 0; i < total_cells; ++i) {
        screen->buffer[i] = clear_char;
    }
}

void print_string(TextScreen* screen, int x, int y, const char* text, SDL_Color fg, SDL_Color bg) {
    int current_x = x;
    int current_y = y;
    for (int i = 0; text[i] != '\0'; ++i) {
        if (current_x >= screen->cols) {
            current_x = 0;
            current_y++;
        }
        if (current_y >= screen->rows) break;
        set_char(screen, current_x, current_y, (unsigned char)text[i], fg, bg);
        current_x++;
    }
}

void set_char(TextScreen* screen, int x, int y, unsigned char chr, SDL_Color fg, SDL_Color bg) {
    if (x >= 0 && x < screen->cols && y >= 0 && y < screen->rows) {
        int index = y * screen->cols + x;
        screen->buffer[index].character_code = chr;
        screen->buffer[index].fg_color = fg;
        screen->buffer[index].bg_color = bg;
    }
}

/**
 * @brief Draws a single character directly to the renderer at a specific grid location.
 */
void draw_char_on_renderer(SDL_Renderer* renderer, const unsigned char* font_data, unsigned char chr, int cx, int cy, SDL_Color fg, SDL_Color bg, float cell_w, float cell_h) {
    // 1. Draw background
    SDL_Rect bg_rect = {(int)round(cx * cell_w), (int)round(cy * cell_h), (int)ceil(cell_w), (int)ceil(cell_h)};
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(renderer, &bg_rect);

    // 2. Draw foreground character pixels
    const unsigned char* char_data = font_data + (chr * FONT_BYTES_PER_CHAR);
    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, 255);

    for (int row = 0; row < FONT_CHAR_HEIGHT; ++row) {
        unsigned char row_data = char_data[row];
        for (int col = 0; col < FONT_CHAR_WIDTH; ++col) {
            if (row_data & (1 << (7 - col))) {
                SDL_Rect pixel_rect = {
                    (int)round(cx * cell_w + col * (cell_w / FONT_CHAR_WIDTH)),
                    (int)round(cy * cell_h + row * (cell_h / FONT_CHAR_HEIGHT)),
                    (int)ceil(cell_w / FONT_CHAR_WIDTH),
                    (int)ceil(cell_h / FONT_CHAR_HEIGHT)
                };
                SDL_RenderFillRect(renderer, &pixel_rect);
            }
        }
    }
}


/**
 * @brief Draws the scrollbar UI overlay and updates their thumb_rects for mouse detection.
 */
void draw_scrollbars(SDL_Renderer* renderer, const unsigned char* font_data) {
    int window_w, window_h;
    SDL_GetRendererOutputSize(renderer, &window_w, &window_h);
    float cell_w = (float)window_w / VIEW_COLS;
    float cell_h = (float)window_h / VIEW_ROWS;
    if (cell_w < 1.0f || cell_h < 1.0f) return;

    SDL_Color fg = dos_palette[7];  // Light Gray
    SDL_Color bg = dos_palette[8];  // Dark Gray
    SDL_Color thumb_bg = dos_palette[7];

    // --- Vertical Scrollbar ---
    int v_track_len = VIEW_ROWS - 2;
    if (v_track_len > 0) {
        float v_denominator = v_scrollbar.content_size - v_scrollbar.view_size;
        float v_pos_ratio = (v_denominator > 0) ? (v_scrollbar.view_pos / v_denominator) : 0;
        float v_ratio = (float)v_scrollbar.view_size / v_scrollbar.content_size;
        int v_thumb_size = fmax(1, v_track_len * v_ratio);
        int v_thumb_pos = 1 + round(v_pos_ratio * (v_track_len - v_thumb_size));

        // Draw track, thumb, and arrows directly to renderer
        draw_char_on_renderer(renderer, font_data, 24, VIEW_COLS - 1, 0, fg, bg, cell_w, cell_h); // Up
        for (int i = 1; i < VIEW_ROWS - 1; ++i) {
            draw_char_on_renderer(renderer, font_data, 177, VIEW_COLS - 1, i, fg, bg, cell_w, cell_h);
        }
        for (int i = 0; i < v_thumb_size; ++i) {
            draw_char_on_renderer(renderer, font_data, 219, VIEW_COLS - 1, v_thumb_pos + i, fg, thumb_bg, cell_w, cell_h);
        }
        draw_char_on_renderer(renderer, font_data, 25, VIEW_COLS - 1, VIEW_ROWS - 1, fg, bg, cell_w, cell_h); // Down
        
        // Update thumb_rect for mouse picking
        v_scrollbar.thumb_rect = (SDL_Rect){(int)round((VIEW_COLS-1)*cell_w), (int)round(v_thumb_pos*cell_h), (int)ceil(cell_w), (int)ceil(v_thumb_size * cell_h)};
    }


    // --- Horizontal Scrollbar ---
    int h_track_len = VIEW_COLS - 2;
    if (h_track_len > 0) {
        float h_denominator = h_scrollbar.content_size - h_scrollbar.view_size;
        float h_pos_ratio = (h_denominator > 0) ? (h_scrollbar.view_pos / h_denominator) : 0;
        float h_ratio = (float)h_scrollbar.view_size / h_scrollbar.content_size;
        int h_thumb_size = fmax(1, h_track_len * h_ratio);
        int h_thumb_pos = 1 + round(h_pos_ratio * (h_track_len - h_thumb_size));

        draw_char_on_renderer(renderer, font_data, 27, 0, VIEW_ROWS - 1, fg, bg, cell_w, cell_h); // Left
        for (int i = 1; i < VIEW_COLS - 1; ++i) {
            draw_char_on_renderer(renderer, font_data, 177, i, VIEW_ROWS - 1, fg, bg, cell_w, cell_h);
        }
        for (int i = 0; i < h_thumb_size; ++i) {
            draw_char_on_renderer(renderer, font_data, 219, h_thumb_pos + i, VIEW_ROWS - 1, fg, thumb_bg, cell_w, cell_h);
        }
        draw_char_on_renderer(renderer, font_data, 26, VIEW_COLS - 1, VIEW_ROWS - 1, fg, bg, cell_w, cell_h); // Right

        h_scrollbar.thumb_rect = (SDL_Rect){(int)round(h_thumb_pos*cell_w), (int)round((VIEW_ROWS-1)*cell_h), (int)ceil(h_thumb_size * cell_w), (int)ceil(cell_h)};
    }
}


/**
 * @brief Renders the visible portion of the text screen buffer to the SDL renderer.
 */
void render_text_screen(SDL_Renderer* renderer, TextScreen* screen, const unsigned char* font_data) {
    int window_w, window_h;
    SDL_GetRendererOutputSize(renderer, &window_w, &window_h);

    float cell_w = (float)window_w / VIEW_COLS;
    float cell_h = (float)window_h / VIEW_ROWS;
    if (cell_w < 1.0f || cell_h < 1.0f) return;

    for (int y = 0; y < VIEW_ROWS; ++y) {
        for (int x = 0; x < VIEW_COLS; ++x) {
            int buffer_x = (int)h_scrollbar.view_pos + x;
            int buffer_y = (int)v_scrollbar.view_pos + y;

            if (buffer_x >= screen->cols || buffer_y >= screen->rows) continue;

            int index = buffer_y * screen->cols + buffer_x;
            VGAChar vga_char = screen->buffer[index];

            // Use the dedicated drawing function
            draw_char_on_renderer(renderer, font_data, vga_char.character_code, x, y, vga_char.fg_color, vga_char.bg_color, cell_w, cell_h);
        }
    }
}

