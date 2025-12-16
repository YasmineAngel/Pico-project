#pragma once 

void init_display(int x, int y, int w, int h);
void end_display();
void print_frame();
void set_bold(bool x);
void print_char_at(int x, int y, char c);
void print_line(int y, const char *s);
void show_display();
void print_at(int x, int y, const char *s);
int get_input();
