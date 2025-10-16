#if defined(_WIN32)
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <ncurses/ncurses.h>

#if defined(__linux__)
#include <sys/ioctl.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char *argv[]) {

  if(argc < 2) {
    fprintf(stderr, "Not argument provided\n");
    return 0;
  }

  char *filename = calloc(1, strlen(argv[1]) + 1);
  Mix_Music *music = NULL;
  strncpy(filename, argv[1], strlen(argv[1]));

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

  double duration = Mix_MusicDuration(music);
  float dur_minutes = floor(duration / 60.0f);
  float dur_seconds = floor(fmodf(duration, 60.0f));
  const char *title = Mix_GetMusicTitleTag(music);
  const char *artist = Mix_GetMusicArtistTag(music);
  const char *album = Mix_GetMusicAlbumTag(music);
  int is_paused = 0, showing_info = 0;
  WINDOW *info, *out_box;
  int chr = 0, showingInfo = 0;

  while(Mix_PlayingMusic()) {
    if(!showingInfo) {
      chr = getch();
      if(chr == 'i') {
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
        info = newwin(height, width, y, x);
        nodelay(info, TRUE);
        nodelay(stdscr, FALSE);
        out_box = newwin(height + 2, width + 2, y - 1, x - 1);
        box(out_box, 0, 0);
        wrefresh(out_box);
        wrefresh(info);
        showingInfo = 1;
      } else if(chr == 'q') {
        break;
        goto end;
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
        mvwprintw(info, 0, 0, "Title: %s", title);
        mvwprintw(info, 1, 0, "Artist: %s", artist);
        mvwprintw(info, 2, 0, "Album: %s", album);
        mvwprintw(info, 3, 0, "Time: %02d:%02d / %02d:%02d",
          (int)minutes, (int)seconds,
          (int)dur_minutes, (int)dur_seconds
        );
        mvwprintw(info, 4, 0, "Loaded from: %s", filename);
        wrefresh(info);
      }
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
