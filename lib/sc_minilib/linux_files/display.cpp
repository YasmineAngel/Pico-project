#include <ncurses.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <display.h>

static int height = 0;
static int width = 0;
static int posx = 0;
static int posy = 0;

void print_at(int x, int y, const char *s)
{
    mvprintw(y, x, s);
}

void print_frame()
{
    int i = 0;
    std::stringstream ss;

    ss << "+" << std::setw(width) << std::setfill('-') << "+";
    print_at(posx, posy, ss.str().c_str());
    for (i=0; i<height; i++) {
        std::stringstream ss;
        ss << std::setw(1) << "|" << std::setw(width) << std::setfill(' ') << "|";
        print_at(posx, posy+i+1, ss.str().c_str());
    }
    std::stringstream ss1;

    ss1 << std::setw(1) << "+" << std::setw(width) << std::setfill('-') << "+";
    print_at(posx, posy+i+1, ss1.str().c_str());
}

void init_display(int x, int y, int w, int h)
{
    height = h;
    width = w;
    posx = x;
    posy= y;
    initscr();
    raw();
    noecho();
    print_frame();
    show_display();
}

void end_display()
{
    endwin();
}

void show_display()
{
    refresh();
}

void set_bold(bool x)
{
    if (x) attron(A_BOLD);
    else attroff(A_BOLD);
}

void print_char_at(int x, int y, char c)
{
    move(y+posy+1,x+posx+1);
    addch(c);
}

void print_line(int y, const char *s)
{
    std::stringstream ss;
    ss << "|" << s << std::setw(width - strlen(s)) << std::setfill(' ') << "|";
    print_at(posx, posy+y, ss.str().c_str());
}

int get_input()
{
    return getch();
}
