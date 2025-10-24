#include <locale.h>
#if defined(_WIN32)
#define SDL_MAIN_HANDLED
#endif

#define NCURSES_WIDECHAR 1

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <ncursesw/curses.h>

#if defined(__linux__)
#include <sys/ioctl.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <math.h>

const wchar_t blocksProg[] = {
  L"\u258f" "\u258e" "\u258d" "\u258c"
  "\u258b" "\u258a" "\u2589" "\u2588"
};

int main(int argc, char *argv[]) {

  setlocale(LC_ALL, "");

#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

  if(argc < 2) {
    fprintf(stderr, "Not argument provided\n");
    return 0;
  }

  char *filename = calloc(1, strlen(argv[1]) + 1);
  char *basen = calloc(1, strlen(argv[1]) + 1);
  Mix_Music *music = NULL;
  strncpy(filename, argv[1], strlen(argv[1]));
  strncpy(basen, basename(filename), strlen(filename));

  if(SDL_Init(SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Error: %s\n", SDL_GetError());
    return 1;
  }

  if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    fprintf(stderr, "Error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  music = Mix_LoadMUS(filename);
  if(music == NULL) {
    fprintf(stderr, "Could not load file: %s\n", SDL_GetError());
    Mix_CloseAudio();
    SDL_Quit();
    return 1;
  }

  if(Mix_PlayMusic(music, 1) < 0) {
    fprintf(stderr, "Could not play music: %s\n", SDL_GetError());
    Mix_CloseAudio();
    SDL_Quit();
    return 1;
  }

#if defined(__linux__)

  struct winsize ws;

  if(ioctl(1, TIOCGWINSZ, &ws)) {
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();
  }

#elif defined(_WIN32)

  CONSOLE_SCREEN_BUFFER_INFO csbi;
  int columns, rows;

  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

#endif

  initscr();
  noecho();
  cbreak();
  nodelay(stdscr, TRUE);

  // if(can_change_color()) printf("Can change");
  // else {
  //   printf("Can't change");
  //   endwin();
  //   return 0;
  // }

  start_color();
  init_color(8, 333, 333, 333);
  init_pair(1, COLOR_WHITE, 8);

  cchar_t vb, hb, tlc, trc, blc, brc;
  setcchar(&vb, L"\u2502", 0, 0, NULL);
  setcchar(&hb, L"\u2500", 0, 0, NULL);
  setcchar(&tlc, L"\u256d", 0, 0, NULL);
  setcchar(&trc, L"\u256e", 0, 0, NULL);
  setcchar(&blc, L"\u256f", 0, 0, NULL);
  setcchar(&brc, L"\u2570", 0, 0, NULL);

  double duration = Mix_MusicDuration(music);
  float dur_minutes = floor(duration / 60.0f);
  float dur_seconds = floor(fmodf(duration, 60.0f));
  const char *title = Mix_GetMusicTitleTag(music);
  const char *artist = Mix_GetMusicArtistTag(music);
  const char *album = Mix_GetMusicAlbumTag(music);

  int is_paused = 0, showing_info = 0;
  WINDOW *info, *out_box;
  int chr = 0, showingInfo = 0;
  float height, width, x, y;

#if defined(__linux__)
  width = ws.ws_col / 10.0f * 8.0f;
  height = ws.ws_row / 10.0f * 5.0f;
  x = (float)ws.ws_col / 2 - width / 2;
  y = (float)ws.ws_row / 2 - height / 2;
#elif defined(_WIN32)
  width = columns / 10.0f * 6.0f;
  height = rows / 10.0f * 3.0f;
  x = (float)columns / 2 - width / 2;
  y = (float)rows / 2 - height / 2;
#endif

  char *incomplete = calloc(width + 1, 1);
  wchar_t *progress = calloc(width + 1, sizeof(wchar_t));
  float filledBlocks = 0, blocks = (width / duration);

  while(Mix_PlayingMusic()) {
    if(!showingInfo) {
      chr = getch();
      if(chr == 'i') {
        info = newwin(height, width, y, x);
        nodelay(info, TRUE);
        nodelay(stdscr, FALSE);
        out_box = newwin(height + 2, width + 2, y - 1, x - 1);
        wborder_set(out_box, &vb, &vb, &hb, &hb, &tlc, &trc, &brc, &blc);
        wrefresh(out_box);
        wrefresh(info);
        showingInfo = 1;
      }
    }

    if(showingInfo){
      chr = wgetch(info);
      if(chr == 'i') {
        wclear(info);
        wclear(out_box);
        wrefresh(info);
        wrefresh(out_box);
        delwin(info);
        delwin(out_box);
        nodelay(stdscr, TRUE);
        refresh();
        showingInfo = 0;
      } else {
        double position = Mix_GetMusicPosition(music);
        float minutes = floor(position / 60);
        float seconds = floor(fmodf(position, 60));
        float percentage = (position / duration) * 100;
        int line = 0;
        filledBlocks = blocks * position;
        if(is_paused) mvwaddstr(info, height - 3, width - 6, "Paused");
        else {
          wmove(info, height - 3, width - 6);
          wclrtoeol(info);
        }
        mvwprintw(info, line++, 0, "Title: %s", title[0] == 0 ? basen : title);
        artist[0] == 0 ? 0 : mvwprintw(info, line++, 0, "Artist: %s", artist);
        album[0] == 0 ? 0 : mvwprintw(info, line++, 0, "Album: %s", album);
        mvwprintw(info, height - 1, 0, "%02d:%02d", (int)minutes, (int)seconds);
        mvwprintw(info, height - 1, width - 5, "%02d:%02d", (int)dur_minutes, (int)dur_seconds);
        mvwprintw(info, line++, 0, "Loaded from: %s", filename);
        size_t len = wcslen(progress);
        size_t ilen = strlen(incomplete);
        for(int i = 0; i < (int)floorf(filledBlocks); i++)
          progress[len] = blocksProg[7], len++;
        int which = (int)floorf(fmodf(filledBlocks, 1.0f) / 0.125f);
        progress[len++] = blocksProg[which];
        progress[len] = '\0';
        for(int i = 0; i < (int)width - ((int)filledBlocks + 1); i++)
          incomplete[i] = ' ', ilen++;
        incomplete[ilen] = '\0';
        wattron(info, COLOR_PAIR(1));
        mvwaddwstr(info, height - 2, 0, progress);
        mvwaddstr(info, height - 2, (int)filledBlocks + 1, incomplete);
        wattroff(info, COLOR_PAIR(1));
        wmove(info, 0, 0);
        wrefresh(info);
      }
      memset(progress, 0, (width + 1) * sizeof(wchar_t));
      memset(incomplete, 0, (width + 1) * sizeof(char));
    }

    free(progress);
    free(incomplete);
    progress = NULL;
    incomplete = NULL;

    if(chr == 'q') {
      break;
      goto end;
    } else if(chr == 0x20) {
      if(is_paused) Mix_ResumeMusic();
      else Mix_PauseMusic();
      is_paused = !is_paused;
    }
  }

end:
  if(showingInfo) {
    wclear(info);
    wclear(out_box);
    wrefresh(info);
    wrefresh(out_box);
    delwin(info);
    delwin(out_box);
  }
  endwin();
  Mix_FreeMusic(music);
  Mix_CloseAudio();
  SDL_Quit();

  endwin();

  return 0;
}
