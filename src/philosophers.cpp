#include <iostream>
#include <thread>
#include <ncurses.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

using std::cout;
using std::cin;
using std::endl;

std::mutex terminal;
std::mutex *w;
bool *ma_widelec;
std::condition_variable *cv;

void Wyswietlanie(int filozofowie) {
    //INTERFACE OF NON CHANGEABLE INFORMATION SUCH AS NAME OF COLLUMNS
    WINDOW *legenda = newwin(filozofowie*3, COLS, 0, 0);    
    wattron(legenda, A_BOLD);   
    mvwprintw(legenda, 0, 0, "PHILOSOPHERS:");
    mvwprintw(legenda, 0, COLS/3-2, "STATUS:");
    mvwprintw(legenda, 0, 2*COLS/3-2, "FORKS:");
    mvwprintw(legenda, filozofowie+2, 0, "LEGEND:");
    wattroff(legenda, A_BOLD);
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); 
    wattron(legenda, COLOR_PAIR(1));
    mvwprintw(legenda, filozofowie+3, 0, "T - THINKS");
    wattroff(legenda, COLOR_PAIR(1));
    start_color();
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); 
    wattron(legenda, COLOR_PAIR(2));
    mvwprintw(legenda, filozofowie+4, 0, "W - WAITS");
    wattroff(legenda, COLOR_PAIR(2));
    start_color();
    init_pair(3, COLOR_RED, COLOR_BLACK); 
    wattron(legenda, COLOR_PAIR(3));
    mvwprintw(legenda, filozofowie+5, 0, "E - EATS");
    wattroff(legenda, COLOR_PAIR(3));
    wrefresh(legenda);
}

void Filozof(int id, int filozofowie) {
    
    WINDOW *filozof_win = newwin(1, COLS/3, id+1, 0);
    WINDOW *status_win = newwin(1, COLS/3, id+1, COLS/3);
    WINDOW *widelce_win = newwin(1, COLS/3, id+1, 2*COLS/3);
    
    while(true){
        // PSEUDO-RANDOM NUMBERS GENERATOR
        std::random_device rd;
        std::mt19937 mt(rd()); //mersenne twister generator lcizb losowych do losowego czasu jedzenia przez filozofa
        std::uniform_int_distribution<int> dist(1000, 2000);
        int random_time = dist(mt);

        // -----THINKS-----
        
        std::unique_lock<std::mutex> terminal_lock(terminal); 
 
        mvwprintw(filozof_win, 0, 0, "Philosopher %d", id);
        wrefresh(filozof_win);
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK); 
        wattron(status_win, COLOR_PAIR(1));
        mvwprintw(status_win, 0, 0, "T");
        wattroff(status_win, COLOR_PAIR(1));
        wrefresh(status_win);
        terminal_lock.unlock();

        //SOLUTION FOR DEADLOCK AND STARVATION
        int lewy_widelec_id = id;
        int prawy_widelec_id = (id + 1) % filozofowie;

        if (id % 2 == 1) 
        {
           std::swap(lewy_widelec_id, prawy_widelec_id);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        
        // -----TAKE LEFT FORK----- 
        std::unique_lock<std::mutex> lewy_widelec_lock(w[lewy_widelec_id]); 
        cv[lewy_widelec_id].wait(lewy_widelec_lock, [lewy_widelec_id]{return !ma_widelec[lewy_widelec_id];}); 
        ma_widelec[lewy_widelec_id] = true; 
        
        terminal_lock.lock();
        start_color();
        init_pair(2, COLOR_YELLOW, COLOR_BLACK); 
        wattron(status_win, COLOR_PAIR(2));
        mvwprintw(status_win, 0, 0, "W");
        wattroff(status_win, COLOR_PAIR(2));
        wrefresh(status_win);
        mvwprintw(widelce_win, 0, 0, "%d", lewy_widelec_id);
        wrefresh(widelce_win); 
        terminal_lock.unlock();
        
        // -----TAKE RIGHT FORK-----
        std::unique_lock<std::mutex> prawy_widelec_lock(w[prawy_widelec_id]);
        cv[prawy_widelec_id].wait(prawy_widelec_lock, [prawy_widelec_id]{return !ma_widelec[prawy_widelec_id];});
        ma_widelec[prawy_widelec_id] = true;
        
        terminal_lock.lock();
        start_color();
        init_pair(2, COLOR_YELLOW, COLOR_BLACK); 
        wattron(status_win, COLOR_PAIR(2));
        mvwprintw(status_win, 0, 0, "W");
        wattroff(status_win, COLOR_PAIR(2));
        wrefresh(status_win);
        mvwprintw(widelce_win, 0, 0, "  %d", prawy_widelec_id);
        wrefresh(widelce_win); 
        terminal_lock.unlock();
        
        // -----EAT-----
        terminal_lock.lock();
        start_color();
        init_pair(3, COLOR_RED, COLOR_BLACK); 
        wattron(status_win, COLOR_PAIR(3));
        mvwprintw(status_win, 0, 0, "E");
        wattroff(status_win, COLOR_PAIR(3));
        wrefresh(status_win);
        mvwprintw(widelce_win, 0, 0, "%d %d", id, (id + 1) % filozofowie);
        wrefresh(widelce_win);
        terminal_lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(random_time));

        // -----PUT OFF LEFT FORK-----
        terminal_lock.lock();
        lewy_widelec_lock.unlock(); //filozof zwalnia blokade na lewym widelcu 
        ma_widelec[lewy_widelec_id] = false;
        cv[lewy_widelec_id].notify_one(); //powiadamiamy filozofa ktory czeka na zmienna warunkowa     
        mvwprintw(widelce_win, 0, 0, " ");
        wrefresh(widelce_win);
        terminal_lock.unlock();

        // -----PUT OFF RIGHT FORK-----
        terminal_lock.lock();
        prawy_widelec_lock.unlock();
        ma_widelec[prawy_widelec_id] = false;
        cv[prawy_widelec_id].notify_one();
        mvwprintw(widelce_win, 0, 0, "   ");
        wrefresh(widelce_win);
        terminal_lock.unlock();

    } 
}

int main(int argc, char* argv[]) {
    initscr();
    int filozofowie = atoi(argv[1]);

    w = new std::mutex[filozofowie]; 
    std::thread f[filozofowie]; 
    ma_widelec = new bool[filozofowie]; 
    cv = new std::condition_variable[filozofowie]; 
    
    Wyswietlanie(filozofowie);

    for (int i = 0; i < filozofowie; i++) 
    {
        ma_widelec[i] = false;
    }
    
    for (int i = 0; i < filozofowie; i++)
    {
        f[i] = std::thread(Filozof, i, filozofowie);
    }
    
    for (size_t i = 0; i < filozofowie; i++)
    {
        f[i].join();
    }
    endwin();
    return 0;
}