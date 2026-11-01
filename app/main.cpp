#include <iostream>
#include <array>
#include <string>
#include <vector>

class date{
  
  private:
  int day,month,year;
  
  public:
  date(int d = 1, int m = 1, int y = 2026): day{d}, month{m}, year{y}{}

};

class tranzaction{

  private:
    std::string title;
    int type;
    date date_tranzaction;
    double ammount;
    bool membership;
    bool win;

  public:
    tranzaction(std::string ti = "unspecified", int ty = 0, const date z, double am = 0, bool mem = false , bool w = false) : title{ti}, type{ty}, date_tranzaction{z}, ammount{am}, membership{mem}, win{w}{}
};

class money_goal{
  
  private:
    std::string title;
    double money;
    date goal_deadline;
  public:
    money_goal(std::string ti = "unspecified", double m = 0, const date g) : title{ti}, money{m}, goal_deadline{g}{}
};

class task{
  
  private:
    std::string title;
    std::string descprition;
    int Urgency;
    date task_deadline;
    double progress;
    bool done;
    bool missed;
 
  public:
    task(std::string ti = "unspecified", std::string desc = "unspecified", int u = 0, const date t_d, double p = 0, bool d = false, bool m = 0) : title{ti}, descprition{desc}, Urgency{u}, task_deadline{t_d}, progress{p}, done{d}, missed{m}{}

};

class budget{

  private:
    double total, left, lost;
    std::vector<tranzaction> tranzactions;
    std::vector<money_goal> goal;
  public:
    budget(double t=0, double le=0, double lo =0, std::vector<tranzaction> tr, std::vector<money_goal> go) : total{t}, left{le} , lost{lo}, tranzactions{tr}, goal{go}{}
};

class task_tracker{

  private:
  int total,done, tbd, missed;
  std::vector<task> tasks;

  public:
  task_tracker(int to=0, int d=0, int tb=0, int mi=0, std::vector<task> ta) : total{to}, done{d}, tbd{tb}, missed{mi}, tasks{ta}{}

};

class lifetracker{
  
  private:
  std::string User;
  std::string Password;
  date current_date;
  budget user_budget;
  task_tracker user_tasks;
  
  public:
  lifetracker(std::string user = "JhonDoe",std::string pass = "password1234",date c_d, budget u_b, task_tracker u_t) : User{user}, Password{pass}, current_date{c_d}, user_budget{u_b}, user_tasks{u_t}{}
};

int main() {
   
    return 0;
}
