#include "game-of-life.h"

#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 15
#define LINE_WIDTH 2

#define ROWS HEIGHT / CELL_SIZE
#define COLS WIDTH / CELL_SIZE

#define DRAW_GRID draw_grid(surface)
#define DRAW_PREVIEW(x, y, color) draw_preview(surface, x, y, color)
#define DRAW_CELL(x, y, color) draw_cell(surface, x, y, color)
#define CLEAR_CELL(x, y) clear_cell(surface, x, y)

#define BLACK 0x000000
#define WHITE 0xffffff
#define YELLOW 0xffff00
#define YELLOW_TRANSPARENT 0x96ffff00
#define GRID_COLOR 0x1f1f1f

void draw_grid(SDL_Surface *surface)
{
  SDL_Rect horizontal_line = {0, 0, WIDTH, LINE_WIDTH};
  for (horizontal_line.y = 0; horizontal_line.y < HEIGHT; horizontal_line.y += CELL_SIZE)
    SDL_FillSurfaceRect(surface, &horizontal_line, GRID_COLOR);

  SDL_Rect vertical_line = {0, 0, LINE_WIDTH, HEIGHT};
  for (vertical_line.x = 0; vertical_line.x < WIDTH; vertical_line.x += CELL_SIZE)
    SDL_FillSurfaceRect(surface, &vertical_line, GRID_COLOR);
}

void draw_preview(SDL_Surface *surface, int x, int y, Uint32 color)
{
  SDL_Rect preview = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};

  SDL_FillSurfaceRect(surface, &preview, color);
}

void draw_cell(SDL_Surface *surface, int x, int y, Uint32 color)
{
  SDL_Rect cell = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
  SDL_FillSurfaceRect(surface, &cell, color);
}

void clear_cell(SDL_Surface *surface, int x, int y)
{
  SDL_Rect cell = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
  SDL_FillSurfaceRect(surface, &cell, BLACK);
}

void render_cells(SDL_Surface *surface, int cells[COLS][ROWS])
{
  for (int x = 0; x < COLS; x++)
  {
    for (int y = 0; y < ROWS; y++)
    {
      if (cells[x][y] == 1) // alive
        draw_cell(surface, x, y, WHITE);
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

  // copy back with the remaining dead cells
  for (int x = 0; x < COLS; x++)
    for (int y = 0; y < ROWS; y++)
      cells[x][y] = next[x][y];
}

int main(int argc, char *argv[])
{
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    printf("Failed to init video\n");
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Game Of Life - Conway | C", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  SDL_Event event;
  int cells[COLS][ROWS] = {0};

  int game = 1;
  bool is_drawing = false;
  bool is_erasing = false;
  bool is_preview = false;
  bool is_running = false;
  int mouseX = -1, mouseY = -1;
  while (game)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_EVENT_QUIT)
      {
        game = 0;
      }
      if (event.type == SDL_EVENT_KEY_DOWN)
      {
        if (event.key.key == SDLK_ESCAPE)
        {
          game = 0;
        }

        if (event.key.key == SDLK_SPACE)
        {
          is_running = !is_running;
        }
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
      {
        int x = event.button.x / CELL_SIZE;
        int y = event.button.y / CELL_SIZE;
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          is_drawing = true;
          cells[x][y] = 1; // mark alive
          printf("cell drawn at : x=%d,y=%d\n", x, y);
        }
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
          is_erasing = true;
          cells[x][y] = 1; // mark dead
          // printf("cell cleared at : x=%d,y=%d\n", x, y);
        }
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
      {
        if (event.button.button == SDL_BUTTON_LEFT)
          is_drawing = false;
        if (event.button.button == SDL_BUTTON_RIGHT)
          is_erasing = false;
      }
      if (event.type == SDL_EVENT_MOUSE_MOTION)
      {
        mouseX = event.button.x / CELL_SIZE;
        mouseY = event.button.y / CELL_SIZE;
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

    // also render saved cells here
    render_cells(surface, cells);
    DRAW_GRID;

    if (is_preview)
    {
      DRAW_PREVIEW(mouseX, mouseY, YELLOW_TRANSPARENT);
    }

    SDL_UpdateWindowSurface(window);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}