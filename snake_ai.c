#include "snake.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGTH 1080

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

// laptop screen
#if 0
  // 1920/2+80
#define WINDOW_X 1040
#define WINDOW_Y 140

// 2nd screen
#else
// 1920 + 1920/2+80
#define WINDOW_X 2000
// #define WINDOW_X 2960
#define WINDOW_Y 140
#endif

#define GRID_SIZE 30
#define CELL_SIZE 20

#define PAD_X 100
#define PAD_Y 100

// 30^2=900
#define SNAKE_MAX_LEN 900

#define FONT "JetBrainsMono-Light.ttf"
#define FONT_SIZE 50

#define RENDER_DELAY 0

typedef enum { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 } Direction;

typedef struct {
  size_t x;
  size_t y;
} Cell;

typedef struct {
  size_t len;
  Cell body[SNAKE_MAX_LEN];
  Cell *head;
  Direction dir;
} Snake;

typedef struct {
  size_t x;
  size_t y;
} Apple;

size_t rand_size_t(size_t min, size_t max) {
  return min + (rand() % (max - min + 1));
}

void SDL_RenderFillCircle(SDL_Renderer *renderer, int center_x, int center_y,
                          int r, SDL_Color c) {
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
  int x, y_top, y_bot;
  for (int dy = 0; dy < r; ++dy) {
    x = sqrt(r * r - dy * dy);
    y_top = center_y + dy;
    y_bot = center_y - dy;
    SDL_RenderDrawLine(renderer, center_x - x, y_top, center_x + x, y_top);
    SDL_RenderDrawLine(renderer, center_x - x, y_bot, center_x + x, y_bot);
  }
}

void grid_render(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0x40, 0x40, 0x50, 0xFF);
  SDL_Rect rect = (SDL_Rect){.w = CELL_SIZE, .h = CELL_SIZE};

  for (size_t i = 0; i < GRID_SIZE; ++i) {
    rect.y = PAD_Y + (i * CELL_SIZE);
    for (size_t j = 0; j < GRID_SIZE; ++j) {
      rect.x = PAD_X + (j * CELL_SIZE);
      if (i == 0 || j == 0 || i == GRID_SIZE - 1 || j == GRID_SIZE - 1) {
        // fill outer rects
        SDL_RenderFillRect(renderer, &rect);
      } else {
        SDL_RenderDrawRect(renderer, &rect);
      }
    }
  }
}

void apple_render(SDL_Renderer *renderer, Apple apple) {
#if 0
  // square apple
  SDL_Rect rect = {
      .h = CELL_SIZE,
      .w = CELL_SIZE,
      .x = PAD_X + CELL_SIZE * apple.x,
      .y = PAD_Y + CELL_SIZE * apple.y,
  };
  SDL_SetRenderDrawColor(renderer, 0xDD, 0x23, 0x23, 0xFF);
  SDL_RenderFillRect(renderer, &rect);
#else
  // circular apple
  SDL_Color color = {0xDD, 0x23, 0x23, 0xFF};
  size_t r = CELL_SIZE / 2;
  size_t x = PAD_X + CELL_SIZE * apple.x + r;
  size_t y = PAD_Y + CELL_SIZE * apple.y + r;
  SDL_RenderFillCircle(renderer, x, y, r, color);
#endif
}

void apple_spawn(Apple *apple, Snake snake) {
  size_t x, y;
  bool is_on_snake = true;
  while (is_on_snake) {
    is_on_snake = false;
    x = rand_size_t(1, GRID_SIZE - 2);
    y = rand_size_t(1, GRID_SIZE - 2);
    for (size_t i = 0; i < snake.len; ++i) {
      if (x == snake.body[i].x && y == snake.body[i].y) {
        is_on_snake = true;
        break;
      }
    }
  }
  apple->x = x;
  apple->y = y;
}

void snake_render(SDL_Renderer *renderer, Snake snake) {
  SDL_SetRenderDrawColor(renderer, 0x33, 0xCC, 0x33, 0xFF);
  SDL_Rect rect = {.h = CELL_SIZE, .w = CELL_SIZE};
  for (size_t i = 0; i < snake.len; ++i) {
    rect.x = PAD_X + snake.body[i].x * CELL_SIZE;
    rect.y = PAD_Y + snake.body[i].y * CELL_SIZE;
    SDL_RenderFillRect(renderer, &rect);
  }
}

void snake_reset(Snake *snake) {
  snake->len = 3;
  for (size_t i = 0; i < 3; ++i) {
    snake->body[i].x = 15 - i;
    snake->body[i].y = 15;
  }
  snake->dir = RIGHT;
}

void snake_grow(Snake *snake) {
  if (snake->len < SNAKE_MAX_LEN) {
    ++snake->len;
  }
}

void snake_move(Snake *snake) {
  for (size_t i = snake->len - 1; i > 0; --i) {
    snake->body[i] = snake->body[i - 1];
  }
  switch (snake->dir) {
  case UP:
    --snake->head->y;
    break;
  case DOWN:
    ++snake->head->y;
    break;
  case LEFT:
    --snake->head->x;
    break;
  case RIGHT:
    ++snake->head->x;
    break;
  }
}

bool is_collision_snake_wall(Snake snake) {
  if (snake.head->x < 1 || snake.head->x > GRID_SIZE - 2 || snake.head->y < 1 ||
      snake.head->y > GRID_SIZE - 2) {
    return true;
  }
  return false;
}

bool is_collision_snake_apple(Snake snake, Apple apple) {
  if (snake.head->x == apple.x && snake.head->y == apple.y) {
    return true;
  }
  return false;
}

bool is_collision_snake_snake(Snake snake) {
  for (size_t i = 1; i < snake.len; ++i) {
    if (snake.head->x == snake.body[i].x && snake.head->y == snake.body[i].y) {
      return true;
    }
  }
  return false;
}

// Q-Learning AI

size_t square(size_t x) { return x * x; }

int eval_action(Snake *snake, Apple apple, Direction next_dir) {
  int reward = 0;

  size_t next_x = snake->head->x;
  size_t next_y = snake->head->y;

  // change next_x, next_y to the potential future snake location
  switch (next_dir) {
  case UP:
    if (next_dir != DOWN) {
      --next_y;
    }
    break;
  case DOWN:
    if (next_dir != UP) {
      ++next_y;
    }
    break;
  case LEFT:
    if (next_dir != RIGHT) {
      --next_x;
    }
    break;
  case RIGHT:
    if (next_dir != LEFT) {
      ++next_x;
    }
    break;
  default:
    break;
  }

  // large negative reward when hitting a wall
  if (next_x < 1 || next_x > GRID_SIZE - 2 || next_y < 1 ||
      next_y > GRID_SIZE - 2) {
    reward -= 100;
  }

  // // small negative reward when touching wall
  // if (next_x < 2 || next_x > GRID_SIZE - 3 || next_y < 2 ||
  //     next_y > GRID_SIZE - 3) {
  //   reward -= 1;
  // }

  // large negative reward when hitting snake
  for (size_t i = 1; i < snake->len; ++i) {
    if (next_x == snake->body[i].x && next_y == snake->body[i].y) {
      reward -= 100;
      break;
    }
  }

  // large positive reward when hitting an apple
  if (next_x == apple.x && next_y == apple.y) {
    reward += 100;
  }

  // fixed reward when moving closer to apple
  int s_next, s_curr;
  if (0) {
    // straight lines distance - moves more like a human
    s_next = abs((int)next_x - (int)apple.x) + abs((int)next_y - (int)apple.y);
    s_curr = abs((int)snake->head->x - (int)apple.x) + abs((int)snake->head->y - (int)apple.y);
  } else {
    // closest distance - moves more like an AI
    s_next = sqrt(square(next_x - apple.x) + square(next_y - apple.y));
    s_curr = sqrt(square(snake->head->x - apple.x) + square(snake->head->y - apple.y));
  }
  if (s_next < s_curr) {
    reward += 2;
  }

  // penalty when turning into a potential enclosemnt
  for (size_t i = 1; i < snake->len; ++i) {
    if (snake->dir != next_dir && (next_dir == UP || next_dir == DOWN)) {
      if (snake->head->x != snake->body[i].x) {
        continue;
      }
      if (snake->dir == RIGHT || snake->dir == LEFT) {
        if (next_dir == UP && snake->head->y > snake->body[i].y) {
          // moving towards body
          reward -= 1;
          break;
        }
        if (next_dir == DOWN && snake->head->y < snake->body[i].y) {
          // moving towards body
          reward -= 1;
          break;
        }
      }
    }
    if (snake->dir != next_dir && (next_dir == LEFT || next_dir == RIGHT)) {
      if (snake->head->y != snake->body[i].y) {
        continue;
      }
      if (snake->dir == UP || snake->dir == DOWN) {
        if (next_dir == LEFT && snake->head->x > snake->body[i].x) {
          // moving towards body
          reward -= 1;
          break;
        }
        if (next_dir == RIGHT && snake->head->x < snake->body[i].x) {
          // moving towards body
          reward -= 1;
          break;
        }
      }
    }
  }

  return reward;
}

void snake_move_forward() { ; }

void snake_move_left(Snake *snake) {
  switch (snake->dir) {
  case UP:
    snake->dir = LEFT;
    break;
  case DOWN:
    snake->dir = RIGHT;
    break;
  case LEFT:
    snake->dir = DOWN;
    break;
  case RIGHT:
    snake->dir = UP;
    break;
  default:
    break;
  }
}

void snake_move_right(Snake *snake) {
  switch (snake->dir) {
  case UP:
    snake->dir = RIGHT;
    break;
  case DOWN:
    snake->dir = LEFT;
    break;
  case LEFT:
    snake->dir = UP;
    break;
  case RIGHT:
    snake->dir = DOWN;
    break;
  default:
    break;
  }
}

void snake_ai(Snake *snake, Apple apple) {
  // eval action: moving forward, left, right
  int score_forward = eval_action(snake, apple, snake->dir);
  int score_left =
      eval_action(snake, apple, snake->dir > UP ? snake->dir - 1 : LEFT);
  int score_right =
      eval_action(snake, apple, snake->dir < LEFT ? snake->dir + 1 : UP);

  if (score_forward >= score_left && score_forward >= score_right) {
    snake_move_forward();
  } else if (score_left >= score_right) {
    snake_move_left(snake);
  } else {
    snake_move_right(snake);
  }
}

// Entrypoint

int main() {
  srand(time(0));

  /*************
   * Setup SDL *
   *************/

  if (SDL_INIT_VIDEO < 0) {
    fprintf(stderr, "ERROR: SDL_INIT_VIDEO");
    return 1;
  }

  if (TTF_Init() < 0) {
    fprintf(stderr, "ERROR: TTF_Init");
    return 1;
  }

  TTF_Font *font = TTF_OpenFont(FONT, FONT_SIZE);
  if (!font) {
    fprintf(stderr, "ERROR: Failed to load font");
    return 1;
  }

  SDL_Window *window;
  SDL_Renderer *renderer;

  window = SDL_CreateWindow("Snake", WINDOW_X, WINDOW_Y, WINDOW_WIDTH,
                            WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);

  if (!window) {
    fprintf(stderr, "ERROR: SDL_CreateWindow");
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    fprintf(stderr, "ERROR: SDL_CreateRenderer");
    return 1;
  }

  /*************
   * Game Loop *
   **************/

  Snake snake;
  snake.head = snake.body;
  snake_reset(&snake);

  bool snake_has_moved;

  Apple apple;
  apple_spawn(&apple, snake);

  size_t score = 0;
  size_t highscore = 0;

  SDL_Event event;
  bool quit = false;
  bool paused = false;
  bool reset = false;
  while (!quit) {
    snake_has_moved = false;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_KEYUP:
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          quit = true;
          break;
        case SDLK_p:
          paused = paused ? false : true;
          break;
        case SDLK_r:
          reset = true;
          break;
        // arrow keys movement
        case SDLK_UP:
          if (snake.dir != DOWN && !snake_has_moved && !paused) {
            snake.dir = UP;
            snake_has_moved = true;
          }
          break;
        case SDLK_DOWN:
          if (snake.dir != UP && !snake_has_moved && !paused) {
            snake.dir = DOWN;
            snake_has_moved = true;
          }
          break;
        case SDLK_LEFT:
          if (snake.dir != RIGHT && !snake_has_moved && !paused) {
            snake.dir = LEFT;
            snake_has_moved = true;
          }
          break;
        case SDLK_RIGHT:
          if (snake.dir != LEFT && !snake_has_moved && !paused) {
            snake.dir = RIGHT;
            snake_has_moved = true;
          }
          break;
        // wasd movement
        case SDLK_w:
          if (snake.dir != DOWN && !snake_has_moved && !paused) {
            snake.dir = UP;
            snake_has_moved = true;
          }
          break;
        case SDLK_s:
          if (snake.dir != UP && !snake_has_moved && !paused) {
            snake.dir = DOWN;
            snake_has_moved = true;
          }
          break;
        case SDLK_a:
          if (snake.dir != RIGHT && !snake_has_moved && !paused) {
            snake.dir = LEFT;
            snake_has_moved = true;
          }
          break;
        case SDLK_d:
          if (snake.dir != LEFT && !snake_has_moved && !paused) {
            snake.dir = RIGHT;
            snake_has_moved = true;
          }
          break;
        } // switch(event.key.keysym.sym)
        break;
      } // switch(event.type)
    }   // while(SDL_PollEvent)

    // RENDER START
    SDL_RenderClear(renderer);

    if (paused) {
      SDL_Delay(50);
      continue;
    }

    if (is_collision_snake_apple(snake, apple)) {
      snake_grow(&snake);
      apple_spawn(&apple, snake);
      ++score;
    }
    snake_move(&snake);
    snake_has_moved = false;

    if (is_collision_snake_wall(snake)) {
      snake_reset(&snake);
      highscore = score > highscore ? score : highscore;
      score = 0;
    }
    if (is_collision_snake_snake(snake)) {
      snake_reset(&snake);
      highscore = score > highscore ? score : highscore;
      score = 0;
    }

    if (reset) {
      snake_reset(&snake);
      reset = false;
      highscore = score > highscore ? score : highscore;
      score = 0;
    }

    grid_render(renderer);
    apple_render(renderer, apple);
    snake_render(renderer, snake);

    snake_ai(&snake, apple);

    SDL_Surface *text_surface;
    SDL_Color color = {0x40, 0x40, 0x50, 0xFF};
    char buff[256];
    sprintf(buff, "Score: %zu   Best: %zu", score, highscore);
    text_surface = TTF_RenderText_Solid(font, buff, color);
    if (!text_surface) {
      fprintf(stderr, "ERROR: Failed to render text");
    }
    SDL_Texture *text_texture;
    text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect dest = {130, 25, text_surface->w, text_surface->h};
    SDL_FreeSurface(text_surface);
    SDL_RenderCopy(renderer, text_texture, NULL, &dest);
    SDL_DestroyTexture(text_texture);

    SDL_SetRenderDrawColor(renderer, 0x18, 0x18, 0x18, 0xFF); // rgba
    SDL_RenderPresent(renderer);
    SDL_Delay(RENDER_DELAY);
    // RENDER END

  } // while(!quit)

  /***********
   * Cleanup *
   ***********/

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();

  return 0;
}
