#include "game-of-life.h"

#define LINE_WIDTH 2

#define WIDTH 900
#define HEIGHT 600

#define ROWS 40
#define COLS 60

#define DRAW_GRID draw_grid(surface, width, height, cell_size)
#define DRAW_PREVIEW(x, y, color) draw_preview(surface, x, y, cell_size, color)
#define DRAW_CELL(x, y, color) draw_cell(surface, x, y, color)
#define CLEAR_CELL(x, y) clear_cell(surface, x, y)

#define BLACK 0x000000
#define WHITE 0xffffff
#define YELLOW 0xffff00
#define YELLOW_TRANSPARENT 0x96ffff00
#define GRID_COLOR 0x1f1f1f

void draw_grid(SDL_Surface *surface, int width, int height, int cell_size)
{
  SDL_Rect horizontal_line = {0, 0, width, LINE_WIDTH};
  for (horizontal_line.y = 0; horizontal_line.y < height; horizontal_line.y += cell_size)
    SDL_FillSurfaceRect(surface, &horizontal_line, GRID_COLOR);

  SDL_Rect vertical_line = {0, 0, LINE_WIDTH, height};
  for (vertical_line.x = 0; vertical_line.x < width; vertical_line.x += cell_size)
    SDL_FillSurfaceRect(surface, &vertical_line, GRID_COLOR);
}

void draw_preview(SDL_Surface *surface, int x, int y, int cell_size, Uint32 color)
{
  SDL_Rect preview = {x * cell_size, y * cell_size, cell_size, cell_size};

  SDL_FillSurfaceRect(surface, &preview, color);
}

void draw_cell(SDL_Surface *surface, int x, int y, int cell_size, Uint32 color)
{
  SDL_Rect cell = {x * cell_size, y * cell_size, cell_size, cell_size};
  SDL_FillSurfaceRect(surface, &cell, color);
}

void clear_cell(SDL_Surface *surface, int x, int y, int cell_size)
{
  SDL_Rect cell = {x * cell_size, y * cell_size, cell_size, cell_size};
  SDL_FillSurfaceRect(surface, &cell, BLACK);
}

void render_cells(SDL_Surface *surface, int cell_size, int cells[COLS][ROWS])
{
  for (int x = 0; x < COLS; x++)
  {
    for (int y = 0; y < ROWS; y++)
    {
      if (cells[x][y] == 1) // alive
        draw_cell(surface, x, y, cell_size, WHITE);
    }
  }
}

int count_neighbors(int cells[COLS][ROWS], int x, int y)
{
  int count = 0;
  // counting around players
  for (int dx = -1; dx <= 1; dx++) // 3 cols
  {
    for (int dy = -1; dy <= 1; dy++) // 3 rows
    {
      if (dx == 0 && dy == 0) // the cell itself
        continue;
      int nx = x + dx;
      int ny = y + dy;
      if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS) // if cell is inside the window
        count += cells[nx][ny];                         // dead 0 | alive 1
    }
  }
  return count;
}

void step_simulation(int cells[COLS][ROWS])
{
  int next[COLS][ROWS] = {0};

  for (int x = 0; x < COLS; x++)
  {
    for (int y = 0; y < ROWS; y++)
    {
      int n = count_neighbors(cells, x, y);

      if (cells[x][y] == 1)
      {
        // survives if 2 or 3 neighbors
        if (n == 2 || n == 3)
          next[x][y] = 1;
      }
      else
      {
        // born if exactly 3 neighbors
        if (n == 3)
          next[x][y] = 1;
      }
    }
  }

  // copy back with the remaining dead cells (death if too many surrounding cells surpopulation)
  for (int x = 0; x < COLS; x++)
    for (int y = 0; y < ROWS; y++)
      cells[x][y] = next[x][y];
}

int alive_cells(int cells[COLS][ROWS])
{
  int count = 0;
  for (int i = 0; i < COLS; i++)
  {
    for (int j = 0; j < ROWS; j++)
    {
      if (cells[i][j])
        count++;
    }
  }
  return count;
}

void draw_text(SDL_Surface *target, TTF_Font *font, int x, int y, SDL_Color color, const char *fmt, ...)
{
  // Render to a surface
  char buffer[256]; // posix paths max
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  SDL_Surface *text_surf = TTF_RenderText_Blended(font, buffer, 0, color);
  if (!text_surf)
  {
    printf("Rendering text: %s\n", SDL_GetError());
    return;
  }

  SDL_Rect dest = {x, y, text_surf->w, text_surf->h};
  SDL_BlitSurface(text_surf, NULL, target, &dest);

  SDL_DestroySurface(text_surf); // cleanup temp surface
}

int main(int argc, char *argv[])
{
  int width;
  int height;
  int cell_size = 15;
  int rows = HEIGHT / cell_size;
  int cols = WIDTH / cell_size;

  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    printf("Failed to init video\n");
    return 1;
  }

  if (!TTF_Init())
  {
    printf("Failed to init Text renderer\n");
    return 1;
  }

  TTF_Font *font = TTF_OpenFont("PressStart2P.ttf", 16);
  if (!font)
  {
    printf("Failed to load font: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Game Of Life - Conway | C", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
  SDL_GetWindowSize(window, &width, &height);

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  SDL_Event event;
  int cells[COLS][ROWS] = {0};

  int game = 1;
  bool is_drawing = false;
  bool is_erasing = false;
  bool is_preview = false;
  bool is_running = false;
  int mouseX = -1, mouseY = -1;

  bool is_key_space_pressed = false;
  bool is_key_f_pressed = false;
  while (game)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_EVENT_QUIT)
      {
        game = 0;
      }
      if (event.type == SDL_EVENT_WINDOW_RESIZED)
      {
        SDL_GetWindowSize(window, &width, &height);
        cell_size = (width / COLS < height / ROWS)
                        ? width / COLS
                        : height / ROWS;
        surface = SDL_GetWindowSurface(window); // refresh surface after resize
        printf("Window resized: %dx%d -> rows=%d, cols=%d\n", width, height, rows, cols);
      }
      if (event.type == SDL_EVENT_KEY_DOWN)
      {
        if (event.key.key == SDLK_ESCAPE)
        {
          game = 0;
        }

        if (event.key.key == SDLK_F && !is_key_f_pressed)
        {
          SDL_MaximizeWindow(window);
          SDL_GetWindowSize(window, &width, &height);
          cell_size = (width / COLS < height / ROWS)
                          ? width / COLS
                          : height / ROWS;
          surface = SDL_GetWindowSurface(window);

          printf("Maximized: %dx%d -> cell_size=%d\n", width, height, cell_size);
          is_key_f_pressed = true;
        }

        if (event.key.key == SDLK_SPACE && !is_key_space_pressed)
        {
          if (!is_running)
            printf("starting simulation...\n");
          else
            printf("stopping simulation...\n");
          is_key_space_pressed = true;
          is_running = !is_running;
        }
      }
      if (event.type == SDL_EVENT_KEY_UP)
      {
        if (event.key.key == SDLK_SPACE && is_key_space_pressed)
        {
          is_key_space_pressed = false;
        }
        if (event.key.key == SDLK_F && is_key_f_pressed)
        {
          is_key_f_pressed = false;
        }
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && !is_running)
      {
        int x = event.button.x / cell_size;
        int y = event.button.y / cell_size;
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          is_drawing = true;
          cells[x][y] = 1; // mark alive
          printf("cell drawn at : x=%d,y=%d\n", x, y);
        }
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
          is_erasing = true;
          cells[x][y] = 0; // mark dead
          // printf("cell cleared at : x=%d,y=%d\n", x, y);
        }
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && !is_running)
      {
        if (event.button.button == SDL_BUTTON_LEFT)
          is_drawing = false;
        if (event.button.button == SDL_BUTTON_RIGHT)
          is_erasing = false;
      }
      if (event.type == SDL_EVENT_MOUSE_MOTION && !is_running)
      {
        mouseX = event.button.x / cell_size;
        mouseY = event.button.y / cell_size;
        if (is_drawing)
        {
          cells[mouseX][mouseY] = 1;
          printf("cell drawn at : x=%d,y=%d\n", mouseX, mouseY);
        }

        if (is_erasing)
        {
          is_drawing = false;
          cells[mouseX][mouseY] = 0;
          // printf("cell cleared at : x=%d,y=%d\n", x, y);
        }

        is_preview = true;
      }
      if (event.type == SDL_EVENT_WINDOW_MOUSE_LEAVE)
      {
        is_preview = false;
      }
      SDL_UpdateWindowSurface(window);
    }

    if (is_running)
    {
      step_simulation(cells);
      SDL_Delay(100); // slow down simulation (ms per step)
    }

    SDL_ClearSurface(surface, 0, 0, 0, 255);

    SDL_Color green = {0, 255, 0, 255};

    // also render saved cells here
    render_cells(surface, cell_size, cells);

    DRAW_GRID;

    if (is_preview && !is_running)
    {
      DRAW_PREVIEW(mouseX, mouseY, YELLOW_TRANSPARENT);
    }

    draw_text(surface, font, 0, 0, green, "alive_cells: %d", alive_cells(cells));

    SDL_UpdateWindowSurface(window);
  }

  TTF_CloseFont(font);
  TTF_Quit();

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}