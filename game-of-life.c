#include "game-of-life.h"

#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 15
#define LINE_WIDTH 2

#define ROWS HEIGHT / CELL_SIZE
#define COLS WIDTH / CELL_SIZE

#define DRAW_GRID draw_grid(surface)
#define DRAW_CELL(x, y, color) draw_cell(surface, x, y, color)
#define CLEAR_CELL(x, y) clear_cell(surface, x, y)

#define BLACK 0x000000
#define WHITE 0xffffff
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

int main(int argc, char *argv[])
{
  SDL_Event event;
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    printf("Failed to init video\n");
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Game Of Life - Conway | C", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

  SDL_Surface *surface = SDL_GetWindowSurface(window);

  int game = 1;
  bool is_drawing = false;
  bool is_erasing = false;
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
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
      {
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          is_drawing = true;
          int x = event.button.x / CELL_SIZE;
          int y = event.button.y / CELL_SIZE;
          DRAW_CELL(x, y, WHITE);
          printf("cell drawn at : x=%d,y=%d\n", x, y);
        }
        if (event.button.button == SDL_BUTTON_RIGHT)
        {
          is_erasing = true;
          int x = event.button.x / CELL_SIZE;
          int y = event.button.y / CELL_SIZE;
          CLEAR_CELL(x, y);
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
        if (is_drawing)
        {
          int x = event.button.x / CELL_SIZE;
          int y = event.button.y / CELL_SIZE;
          DRAW_CELL(x, y, WHITE);
          printf("cell drawn at : x=%d,y=%d\n", x, y);
        }

        if (is_erasing)
        {
          is_drawing = false;
          int x = event.button.x / CELL_SIZE;
          int y = event.button.y / CELL_SIZE;
          CLEAR_CELL(x, y);
          // printf("cell cleared at : x=%d,y=%d\n", x, y);
        }
      }
      SDL_UpdateWindowSurface(window);
    }

    DRAW_GRID;

    SDL_UpdateWindowSurface(window);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}