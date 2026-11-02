#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class date{

  private:
    int day, month, year;

  public:
    explicit date(int d = 1, int m = 1, int y = 2026) : day{d}, month{m}, year{y}{}

    bool before(const date& o) const {
      if(year != o.year) return year < o.year;
      if(month != o.month) return month < o.month;
      return day < o.day;
    }

    bool same_month(const date& o) const { return month == o.month && year == o.year; }

    friend std::ostream& operator<<(std::ostream& os, const date& d){
      return os << d.day << "." << d.month << "." << d.year;
    }
};

enum class category{ food, transportation, shopping, bills, housing, health, entertainment, education, other, salary, bonus };

class transaction{

  private:
    std::string title;
    category cat;
    date date_transaction;
    double amount;
    bool membership;
    bool win;

    bool is_income() const { return cat == category::salary || cat == category::bonus; }

  public:
    explicit transaction(const std::string& ti = "unspecified", category c = category::other, const date& z = date{}, double am = 0, bool mem = false, bool w = false) : title{ti}, cat{c}, date_transaction{z}, amount{am}, membership{mem}, win{w}{}

    std::string category_name() const {
      static const char* const names[] = {"food", "transportation", "shopping", "bills", "housing", "health", "entertainment", "education", "other", "salary", "bonus"};
      return names[static_cast<int>(cat)];
    }

    double signed_amount() const { return is_income() ? amount : -amount; }
    bool in_month(const date& d) const { return date_transaction.same_month(d); }

    friend std::ostream& operator<<(std::ostream& os, const transaction& t){
      os << t.title << " | " << t.category_name() << " | " << t.date_transaction << " | " << t.signed_amount();
      if(t.membership) os << " | subscription";
      if(t.win) os << " | win";
      return os << "\n";
    }
};

class money_goal{

  private:
    std::string title;
    double money;
    date goal_deadline;

  public:
    explicit money_goal(const std::string& ti = "unspecified", double m = 0, const date& g = date{}) : title{ti}, money{m}, goal_deadline{g}{}

    double get_money() const { return money; }

    friend std::ostream& operator<<(std::ostream& os, const money_goal& g){
      return os << g.title << " | target " << g.money << " | deadline " << g.goal_deadline << "\n";
    }
};

class task{

  private:
    std::string title, description;
    int urgency;
    date task_deadline;
    double progress;
    bool done, missed;

  public:
    explicit task(const std::string& ti = "unspecified", const std::string& desc = "unspecified", int u = 3, const date& t_d = date{}, double p = 0, bool d = false, bool m = false) : title{ti}, description{desc}, urgency{std::clamp(u, 1, 3)}, task_deadline{t_d}, progress{p}, done{d}, missed{m}{}

    const std::string& get_title() const { return title; }
    bool is_done() const { return done; }
    bool is_missed() const { return missed; }

    void finish(){
      done = true;
      missed = false;
      progress = 100;
    }

    void check_missed(const date& today){
      if(!done && task_deadline.before(today)) missed = true;
    }

    // TBD Urgency ranking on same level
    bool more_urgent(const task& o) const {
      if(done != o.done) return !done;
      return urgency < o.urgency;
    }

    friend std::ostream& operator<<(std::ostream& os, const task& t){
      os << t.title << " (" << t.description << ") | level " << t.urgency << " | due " << t.task_deadline << " | " << t.progress << "% | " << (t.done ? "done" : t.missed ? "missed" : "to do");
      return os << "\n";
    }
};

class budget{

  private:
    double total, left, lost;
    std::vector<transaction> transactions;
    std::vector<money_goal> goals;

    void update(const transaction& t){
      if(t.signed_amount() > 0) total += t.signed_amount();
      else lost -= t.signed_amount();
      left = total - lost;
    }

  public:
    explicit budget(double t = 0, const std::vector<transaction>& tr = {}, const std::vector<money_goal>& go = {}) : total{t}, left{t}, lost{0}, transactions{tr}, goals{go}{
      for(const auto& x : transactions) update(x);
    }

    budget(const budget& o) : total{o.total}, left{o.left}, lost{o.lost}, transactions{o.transactions}, goals{o.goals}{
      std::cout << "[budget copy constructor]\n";
    }

    budget& operator=(const budget& o){
      total = o.total;
      left = o.left;
      lost = o.lost;
      transactions = o.transactions;
      goals = o.goals;
      std::cout << "[budget operator=]\n";
      return *this;
    }

    ~budget(){ std::cout << "[budget destructor]\n"; }

    void add_transaction(const transaction& t){
      transactions.push_back(t);
      update(t);
    }

    void add_goal(const money_goal& g){ goals.push_back(g); }

    void monthly_report(const date& d) const {
      double earned = 0, spent = 0;
      for(const auto& t : transactions){
        if(!t.in_month(d)) continue;
        if(t.signed_amount() > 0) earned += t.signed_amount();
        else spent -= t.signed_amount();
      }
      std::cout << "Month report: earned " << earned << ", spent " << spent << ", saved " << earned - spent << "\n";
    }

    void goals_status() const {
      for(const auto& g : goals){
        std::cout << "  " << g;
        if(g.get_money() <= 0 || left >= g.get_money()) std::cout << "    reached!\n";
        else std::cout << "    " << left / g.get_money() * 100 << "% done\n";
      }
    }

    friend std::ostream& operator<<(std::ostream& os, const budget& b){
      os << "Budget: total " << b.total << ", left " << b.left << ", lost " << b.lost << "\n";
      for(const auto& t : b.transactions) os << "  " << t;
      for(const auto& g : b.goals) os << "  " << g;
      return os;
    }
};

class task_tracker{

  private:
    int total, done, tbd, missed;
    std::vector<task> tasks;

    void recount(){
      total = static_cast<int>(tasks.size());
      done = tbd = missed = 0;
      for(const auto& t : tasks){
        if(t.is_done()) done++;
        else if(t.is_missed()) missed++;
        else tbd++;
      }
    }

  public:
    explicit task_tracker(const std::vector<task>& ta = {}) : total{0}, done{0}, tbd{0}, missed{0}, tasks{ta}{
      recount();
    }

    void add_task(const task& t){
      tasks.push_back(t);
      recount();
    }

    bool complete(const std::string& title){
      for(auto& t : tasks)
        if(t.get_title() == title && !t.is_done()){
          t.finish();
          recount();
          return true;
        }
      return false;
    }

    void update_missed(const date& today){
      for(auto& t : tasks) t.check_missed(today);
      recount();
    }

    double completion() const { return total ? done * 100.0 / total : 0; }

    std::vector<task> prioritize() const {
      std::vector<task> sorted = tasks;
      std::stable_sort(sorted.begin(), sorted.end(), [](const task& a, const task& b){ return a.more_urgent(b); });
      return sorted;
    }

    friend std::ostream& operator<<(std::ostream& os, const task_tracker& tt){
      os << "Tasks: " << tt.total << " total, " << tt.done << " done, " << tt.tbd << " to do, " << tt.missed << " missed\n";
      for(const auto& t : tt.tasks) os << "  " << t;
      return os;
    }
};

class lifetracker{

  private:
    std::string User, Password;
    date current_date;
    budget user_budget;
    task_tracker user_tasks;

  public:
    explicit lifetracker(const std::string& user = "JhonDoe", const std::string& pass = "password1234", const date& c_d = date{}, const budget& u_b = budget{}, const task_tracker& u_t = task_tracker{}) : User{user}, Password{pass}, current_date{c_d}, user_budget{u_b}, user_tasks{u_t}{}

    bool login(const std::string& u, const std::string& p) const { return u == User && p == Password; }

    void add_transaction(const transaction& t){ user_budget.add_transaction(t); }
    void add_task(const task& t){ user_tasks.add_task(t); }

    void advance_to(const date& d){
      current_date = d;
      user_tasks.update_missed(current_date);
    }

    void summary() const {
      std::cout << "=== " << User << ", " << current_date << " ===\n";
      user_budget.monthly_report(current_date);
      user_budget.goals_status();
      std::cout << "Tasks done: " << user_tasks.completion() << "%\n";
      for(const auto& t : user_tasks.prioritize()) std::cout << "  " << t;
    }

    friend std::ostream& operator<<(std::ostream& os, const lifetracker& l){
      return os << "User: " << l.User << " | date " << l.current_date << "\n" << l.user_budget << l.user_tasks;
    }
};

int main() {

    date today{2, 11, 2026};

    budget b{500};
    b.add_transaction(transaction{"Salary", category::salary, today, 3000});
    b.add_transaction(transaction{"Netflix", category::entertainment, today, 50, true});
    b.add_transaction(transaction{"Groceries", category::food, today, 320});
    b.add_goal(money_goal{"Laptop", 5000, date{1, 3, 2027}});
    b.add_goal(money_goal{"Emergency fund", 1500, date{1, 1, 2027}});
    b.monthly_report(today);
    b.goals_status();
    std::cout << b;

    budget copy{b};
    budget other;
    other = copy;

    task_tracker tt;
    tt.add_task(task{"Lab 1", "OOP homework", 1, date{5, 11, 2026}, 40});
    tt.add_task(task{"Gym", "Leg day", 3, date{3, 11, 2026}});
    tt.add_task(task{"Taxes", "Submit declaration", 2, date{1, 11, 2026}});
    tt.complete("Taxes");
    for(const auto& t : tt.prioritize()) std::cout << t;
    std::cout << tt << tt.completion() << "%\n";

    lifetracker app{"Intesfy", "Cyberpunk", today, b, tt};
    std::cout << app.login("Intesfy", "Cyberpunk") << app.login("Intesfy", "0000") << "\n";
    app.advance_to(date{4, 11, 2026});
    app.summary();
    std::cout << app;

    return 0;
}
