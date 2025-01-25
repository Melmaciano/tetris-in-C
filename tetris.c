#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define F_WIDTH 12
#define F_HEIGHT 24

int rotate(int px, int py, int r);
bool does_pieces_fit(int n_tetromino, int n_rotation, int posX, int posY);
void get_key(bool *keys);
void delete_lines(char *field, int len);

int field_width = F_WIDTH;
int field_height = F_HEIGHT;
char *tetromino[7] = {
  "..X...X...X...X.",  
  "..X..XX...X.....",
  ".....XX..XX.....",
  "..X..XX..X......",
  ".X...XX...X.....",
  ".X...X...XX.....",
  "..X...X..XX....."
};
char field[F_WIDTH * F_HEIGHT];

int main(void) 
{
  srand(time(NULL));
  
  for (int x = 0; x < field_width; x++)
    for (int y = 0; y < field_height; y++)
      field[y * field_width + x] = 
          (x == 0 || x == field_width - 1 || y == field_height - 1) ?
          9 :
          0;
  // VARIABLES 
  // Keys
  bool keys[5]; // KEY_LEFT, KEY_RIGHT, KEY_DOWN, KEY_UP, KEY_BACKSPACE 
  // State of piece
  int current_piece = rand() % 7;
  int current_rotation = 0;
  // Piece location
  int current_y = 0;
  int current_x = field_width / 2;
  // State of game
  int speed = 10;
  int speed_count = 0;
  bool force_down = false;
  bool rotate_hold = true;
  int piece_count = 0;
  bool game_over = false;
  long score = 0;

  // WINDOWS
  int c_y, c_x;
  initscr();
  nodelay(stdscr, TRUE);
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  // LOGIC
  while (!game_over) 
  {
    // Timing
    usleep(50000);
    speed_count++;
    force_down = (speed_count == speed);
  
    // Input
    //flushinp();
    for (int i = 0; i < 5; i++)
      keys[i] = 0;
    for (int i = 0; i < 5; i++)
      get_key(keys);
    // Handle player movement
      current_x -= (keys[0] && does_pieces_fit(current_piece,
          current_rotation, current_x - 1, current_y)) ? 1 : 0;
      current_x += (keys[1] && does_pieces_fit(current_piece,
          current_rotation, current_x + 1, current_y)) ? 1 : 0;
      current_y += (keys[2] && does_pieces_fit(current_piece,
          current_rotation, current_x, current_y + 1)) ? 1 : 0;
      if (keys[4])
        while (does_pieces_fit(current_piece, current_rotation, 
              current_x, current_y + 1))
          current_y++; 
    // Rotate
    if (keys[3]) 
    {
      current_rotation += (rotate_hold && does_pieces_fit(current_piece,
          current_rotation + 1, current_x, current_y)) ? 1 : 0;
      rotate_hold = false;
    } 
    else
      rotate_hold = true;
    
    // Force the piece down
    if (force_down) 
    {
      delete_lines(field, F_WIDTH * F_HEIGHT);
      speed_count = 0;
      piece_count++;
  
      if (piece_count % 20 == 0)
        if (speed > 3) speed--;
  
      //Test if piece can be moved down
      if (does_pieces_fit(current_piece, current_rotation,
             current_x, current_y + 1)) 
        current_y++;
      else 
      {
        // Lock the piece in place, change zeros values to piece values
        for (int x = 0; x < 4; x++)
          for (int y = 0; y < 4; y++)
            if (tetromino[current_piece][rotate(x, y,
                  current_rotation)] != '.')
              field[(current_y + y) * 
                  field_width + (current_x + x)] = current_piece + 1;
      
        // Check for lines
        for (int y = 0; y < 4; y++)
          if (current_y + y < field_height - 1) {
            bool line = true;
            for (int x = 1; x < field_width - 1; x++)
              if (field[(current_y + y) * field_width + x] == 0) {
                line = false;
                score += 20;
                break;
              }

            if (line) 
            {
              for (int x = 1; x < field_width - 1; x++)
                field[(current_y + y) * field_width + x] = 8;
            }
          }
        
        current_x = field_width / 2;
        current_y = 0;
        current_rotation = 0;
        current_piece = rand() % 7;

        // Check gameover
        game_over = !does_pieces_fit(current_piece, current_rotation,
              current_x, current_y);
      }
    } 

    // DISPLAY
    // Draw field
    erase();
    
    for (int y = 0; y < field_height; y++)
      for (int x = 0; x < field_width; x++)
      {
        int ch = " ABCDEFG=#"[field[y * field_width + x]];
        addch(ch);
        addch(' ');
        if (x == field_width - 1)
          addch('\n');
      }
    // Draw current piece
    for (int y = 0; y < 4; y++)
      for (int x = 0; x < 4; x++)
      {
        if (tetromino[current_piece][rotate(x, y, 
              current_rotation)] != '.') 
        {
          int py = current_y + y;
          int px = current_x * 2 + x * 2;
          int ch = current_piece + 65;
          mvaddch(py, px, ch);
          addch(' ');
        }
      }
  
    // Draw score
    getyx(stdscr, c_y, c_x);
    mvaddstr(3, field_width * 2 + 2, "Your score:");
    mvprintw(2, field_width * 2 + 2, "%ld", score);
    move(c_y, c_x);
  
  } // END WHILE

  endwin();
  printf(" Your score is: %d\n", score);
  return 0;
}

void delete_lines(char *field, int len)
{
  char *p, *q;
  q = p = field + len;
  
  // 9 = '#', 8 = '='
  while (p >= field)
  {
    if (*q == 9)
    {
      q--;
      p--;
    }
    else if (*p == 8 || *p == 9)
      p--;
    else
    {
      *q = *p;
      p--;
      q--;
    }
  }

  while (q >= field)
    {
      if (*q != 9)
        *q = 0;
      q--;
    }
}

void get_key(bool *keys)
{
  int ch = getch();
  switch(ch) {
    case KEY_LEFT:
      keys[0] = 1;
      break;
    case KEY_RIGHT:
      keys[1] = 1;
      break;
    case KEY_DOWN:
      keys[2] = 1;
      break;
    case KEY_UP:
      keys[3] = 1;
      break;
    case 'x':
      keys[4] = 1;
      break;
    default:
      break;
  }
}

int rotate(int px, int py, int r)
{
  int pi = 0;

  switch (r % 4) 
  {              
    case 0: // 0 degrees        // 0  1  2  3
      pi = py * 4 + px;         // 4  5  6  7
      break;                    // 8  9  10 11
                                // 12 13 14 15

    case 1: // 90 degrees       // 12 8  4  0
      pi = 12 + py - (px * 4);  // 13 9  5  1
      break;                    // 14 10 6  2
                                // 15 11 7  3

    case 2: // 180 degrees      // 15 14 13 12
      pi = 15 - (py * 4) - px;  // 11 10 9  8
      break;                    // 7  6  5  4
                                // 3  2  1  0

    case 3: // 270 degrees      // 3  7  11 15
      pi = 3 - py + (px * 4);   // 2  6  10 14
      break;                    // 1  5  9  13
                                // 0  4  8  12
    default:
      break;
  }
 
  return pi;
}

bool does_pieces_fit(int n_tetromino, int n_rotation, int posX, int posY)
{
  for (int px = 0; px < 4; px++)
    for (int py = 0; py < 4; py++) 
    {
      // get index into piece
      int pi = rotate(px, py, n_rotation);
      // get index into field
      int fi = (posY + py) * field_width + (posX + px);

      if (posX + px >= 0 && posX + px < field_width &&
          posY + py >= 0 && posY + py < field_height)
        if (tetromino[n_tetromino][pi] != '.' &&
            field[fi] != 0)
          return false; // If two condition are true, piece does not fit
    }

  return true;
}
