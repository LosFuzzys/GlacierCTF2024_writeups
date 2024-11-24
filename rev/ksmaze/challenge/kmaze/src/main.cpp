#include "utils.h"
#include <ncurses.h>
#include <assert.h>
#include <random>
#include <sys/mman.h>

class Maze {

public:

  // Dimensions
  static constexpr uint32_t DIMS_X = 64;
  static constexpr uint32_t DIMS_Y = 64;
  // Initial coordinates
  static constexpr uint32_t STRT_X = 1;
  static constexpr uint32_t STRT_Y = DIMS_Y - 2;
  // Character representation
  static constexpr char   WALL_C = '#';
  static constexpr char   PLAY_C = '@';
  static constexpr char   EMPT_C = ' ';
  static constexpr char   COIN_C = '.';
  static constexpr char   ENEM_C = '&';
  static constexpr char   TREE_C = '^';
  static constexpr char   FLAG_C = 'F';

private:

  // Maze map in memory
  unsigned char (*memory)[DIMS_Y];

public:

  Maze() {
    memory = (unsigned char (*)[DIMS_X]) std::aligned_alloc(DIMS_X * DIMS_Y,
                                                            DIMS_X * DIMS_Y);

    static std::random_device r;
    static std::default_random_engine de(r());
    static std::uniform_int_distribution<uint8_t> uniform_dist(0, 100);

    // Randomly generate the map
    for(uint32_t y = 0 ; y < DIMS_Y ; y++) {
      for(uint32_t x = 0 ; x < DIMS_X ; x++) {
        // Walls in the borders
        if(x == 0 || y == 0 || x == DIMS_X - 1 || y == DIMS_Y - 1) {
          this->set(x, y, WALL_C);
          continue;
        }
        // Randomly generate cell
        auto c = uniform_dist(de);
        if(c <= 20)
            this->set(x, y, COIN_C);
        else if(c <= 30)
            this->set(x, y, ENEM_C);
        else if(c <= 40)
            this->set(x, y, TREE_C);
        else if(c <= 70)
            this->set(x, y, WALL_C);
        else
            this->set(x, y, EMPT_C);
      }
    }

    // Get center of the map
    assert((DIMS_X / 2) >= 3);
    assert((DIMS_Y / 2) >= 3);
    uint32_t x_h = (DIMS_X / 2) - 3;
    uint32_t y_h = (DIMS_Y / 2) - 3;

    // Hardcode the flag in the middle
    for(uint32_t y = y_h ; y < y_h + 6; y++) {
      for(uint32_t x = x_h ; x < x_h + 6; x++) {
        if(x == x_h || x == x_h + 5 || y == y_h || y == y_h + 5)
          this->set(x, y, ENEM_C);
        else if(x == x_h + 1 || x == x_h + 4 || y == y_h + 1|| y == y_h + 4)
          this->set(x, y, WALL_C);
        else
          this->set(x, y, FLAG_C);
      }
    }

    // Make space for the player
    this->set(STRT_X, STRT_Y, Maze::PLAY_C);
    
    // Save on memory
    madvise(memory, DIMS_X * DIMS_Y, MADV_MERGEABLE);
  }

  ~Maze() {
    madvise(memory, DIMS_X * DIMS_Y, MADV_UNMERGEABLE);
    std::free(memory);
  }

  inline unsigned char get(uint32_t x, uint32_t y) {
    if(x >= DIMS_X || y >= DIMS_Y) [[unlikely]]
      throw std::invalid_argument("Out of bounds access");
    return memory[y][x];
  }

  inline void set(uint32_t x, uint32_t y, unsigned char v) {
    if(x >= DIMS_X || y >= DIMS_Y) [[unlikely]]
      throw std::invalid_argument("Out of bounds access");
    memory[y][x] = v;
  }

  // Check for valid movement
  bool valid(uint32_t x, uint32_t y) {
    if(x >= DIMS_X || y >= DIMS_Y) [[unlikely]]
      return false;
    return memory[y][x] != WALL_C;
  }

};

class Player {

private:

  uint32_t x = Maze::STRT_X, y = Maze::STRT_Y;

public:

  inline std::pair<typeof(x), typeof(y)> getpos() {
    return {x, y};
  }

  inline void setpos(typeof(x) _x, typeof(y) _y) {
    this->x = _x;
    this->y = _y;
  }
};

class Game {

private:

  // Player and Maze
  Player pl;
  Maze   mz;
  // Ncurses Windows
  WINDOW *win_m;
  WINDOW *win_p;
  WINDOW *win_f;
  // Statistics
  uint32_t score = 0;
  uint32_t deaths = 0;

  // Print maze's `memory` with ncurses
  void
  showmaze()
  {
    for(uint32_t y = 0 ; y < Maze::DIMS_Y ; y++) {
      for(uint32_t x = 0 ; x < Maze::DIMS_X ; x++) {
        uint32_t ch;
        switch(ch = mz.get(x, y)) {
          case Maze::WALL_C:
            mvwaddch(win_m, y, x, ch | COLOR_PAIR(1));
            break;
          case Maze::PLAY_C:
            mvwaddch(win_m, y, x, ch | COLOR_PAIR(2) | A_BOLD);
            break;
          case Maze::ENEM_C:
            mvwaddch(win_m, y, x, ch | COLOR_PAIR(3) | A_BOLD);
            break;
          case Maze::COIN_C:
            mvwaddch(win_m, y, x, ch | COLOR_PAIR(4) | A_BOLD);
            break;
          case Maze::TREE_C:
            mvwaddch(win_m, y, x, ch | COLOR_PAIR(5) | A_BOLD);
            break;
          case Maze::FLAG_C:
            mvwaddch(win_m, y, x, ch | COLOR_PAIR(6) | A_BOLD);
            break;
          case Maze::EMPT_C:
            break;
          default:
            throw std::invalid_argument("Malformed Map. Should never happen");
        }
      }
    }
    wrefresh(win_m);
  }

public:

  Game() 
  {
    // Maze Ncurses Window
    win_m = newwin(Maze::DIMS_Y, Maze::DIMS_X, 0, 0);
    if(win_m == nullptr)
      throw std::runtime_error("Couldn't create new curses window");

    // Scores Ncurses Window
    win_p = newwin(4, 60, 0, Maze::DIMS_Y + 4);
    if(win_p == nullptr)
      throw std::runtime_error("Couldn't create new curses window");
    box(win_p, 0 , 0);
    update_score();
    
    // Flag Ncurses Window
    win_f = newwin(3, 60, 4, Maze::DIMS_Y + 4);
    if(win_f == nullptr)
      throw std::runtime_error("Couldn't create new curses window");
    box(win_f, 0 , 0);
    update_flag({});

    // Decoration options
    curs_set(0);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);

    // Print initial maze
    this->reset();
  }

  // Move the player by some offset (-1,0,1)
  bool
  move(int32_t x_of, int32_t y_of)
  {
    if(x_of < -1 || x_of > 1)
      throw std::invalid_argument("Wrong arg to x");
    if(y_of < -1 || y_of > 1)
      throw std::invalid_argument("Wrong arg to y");

    auto [px, py] = pl.getpos();
    auto npx = static_cast<uint32_t>(static_cast<int32_t>(px) + x_of),
         npy = static_cast<uint32_t>(static_cast<int32_t>(py) + y_of);

    if(not mz.valid(npx, npy))
      return false;

    switch(mz.get(npx, npy)) {
      case Maze::ENEM_C: {
        deaths++;
        mz.set(px, py, ' ');
        update_score();
        update_flag({});
        reset();
        return true;
      }
      case Maze::COIN_C: {
        score++;
        update_score();
        break;
      }
      case Maze::TREE_C: {
        if(score > 0) {
          score--;
          update_score();
        }
        break;
      }
      case Maze::FLAG_C: {
        update_flag("/flag.txt");
        break;
      }
      default:
        break;
    }

    // Move
    mvwaddch(win_m, py, px, Maze::EMPT_C);
    pl.setpos(npx, npy);
    mvwaddch(win_m, npy, npx, Maze::PLAY_C | A_BOLD | COLOR_PAIR(2));
    mz.set(px, py, ' ');
    mz.set(npx, npy, '@');

    wrefresh(win_m);

    return true;
  }

  void
  reset()
  {
    auto [px, py] = pl.getpos();
    mvwaddch(win_m, py, px, Maze::EMPT_C);
    pl.setpos(Maze::STRT_X, Maze::STRT_Y);
    this->showmaze();
    this->move(0, 0);
    wrefresh(win_m);
  }

  // Update game score
  void
  update_score()
  {
      wmove(win_p, 1, 2);
      wprintw(win_p, "Score:  %u", score);
      wmove(win_p, 2, 2);
      wprintw(win_p, "Deaths: %u", deaths);
      wrefresh(win_p);
  }
 
  // Update flag score if we found it
  void
  update_flag(std::optional<std::filesystem::path> flagpath)
  {
    std::optional<std::string> flag = {};
    if(flagpath.has_value()) {
      flag = get_file_contents(flagpath.value());
      if(flag.has_value())
        flag->pop_back(); // Remove \n
    }
    wmove(win_f, 1, 2);
    wprintw(win_f, "Flag:  %s", flag.value_or("").c_str());
    wrefresh(win_f);
  }

  // Access keystrokes to ncurses maze win
  inline auto
  wgetch()
  {
    return ::wgetch(win_m);
  }

};

int
main(int argc, char *argv[])
{
  initscr();
  cbreak();
  noecho();
  start_color();

  Game g;

  int ch;
  while((ch = g.wgetch())) {
    switch(ch) {
      case 'w':
        g.move(0, -1);
        break;
      case 'a':
        g.move(-1, 0);
        break;
      case 's':
        g.move(0, 1);
        break;	
      case 'd':
        g.move(1, 0);
        break;
      default:
        break;
    }
  }

  endwin();
  return 0;
}
