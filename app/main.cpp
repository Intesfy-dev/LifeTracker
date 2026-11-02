#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class date{

  private:
    int day, month, year;

    bool leap() const { return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0; }

    int days_in_month() const {
      static const int dm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      return (month == 2 && leap()) ? 29 : dm[month - 1];
    }

    
    int to_days() const {
      int y = year - 1, days = y * 365 + y / 4 - y / 100 + y / 400;
      for(int m = 1; m < month; m++)
        days += date{1, m, year}.days_in_month();
      return days + day;
    }

  public:
    explicit date(int d = 1, int m = 1, int y = 2026): day{d}, month{m}, year{y}{
      if(month < 1 || month > 12) month = 1;
      if(day < 1 || day > days_in_month()) day = 1;
    }

    int days_until(const date& o) const { return o.to_days() - to_days(); }

    date tomorrow() const {
      if(day < days_in_month()) return date{day + 1, month, year};
      if(month < 12) return date{1, month + 1, year};
      return date{1, 1, year + 1};
    }

    bool same_month(const date& o) const { return month == o.month && year == o.year; }

    friend std::istream& operator>>(std::istream& is, date& d){
      int dd = 1, mm = 1, yy = 2026;
      char sep = '.';
      if(is >> dd >> sep >> mm >> sep >> yy) d = date{dd, mm, yy};
      return is;
    }

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

    static std::vector<std::string> names(){
      return {"food", "transportation", "shopping", "bills", "housing", "health", "entertainment", "education", "other", "salary", "bonus"};
    }

  public:
    explicit transaction(const std::string& ti = "unspecified", category c = category::other, const date& z = date{}, double am = 0, bool mem = false, bool w = false) : title{ti}, cat{c}, date_transaction{z}, amount{am}, membership{mem}, win{w}{}

    std::string category_name() const { return names()[static_cast<std::size_t>(cat)]; }

    double signed_amount() const { return is_income() ? amount : -amount; }
    bool is_membership() const { return membership; }
    bool in_month(const date& d) const { return date_transaction.same_month(d); }

    friend std::istream& operator>>(std::istream& is, transaction& t){
      std::string cat_name;
      char sep = ',';
      std::getline(is, t.title, ',');
      std::getline(is, cat_name, ',');
      is >> t.date_transaction >> sep >> t.amount >> sep >> t.membership >> sep >> t.win;
      const std::vector<std::string> all = names();
      auto it = std::find(all.begin(), all.end(), cat_name);
      t.cat = it == all.end() ? category::other : static_cast<category>(it - all.begin());
      return is;
    }

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
    int days_left(const date& today) const { return today.days_until(goal_deadline); }

    friend std::istream& operator>>(std::istream& is, money_goal& g){
      char sep = ',';
      std::getline(is, g.title, ',');
      return is >> g.money >> sep >> g.goal_deadline;
    }

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

    static constexpr int max_level = 3;

    int days_left(const date& today) const { return today.days_until(task_deadline); }

  public:
    static int levels(){ return max_level; }

    explicit task(const std::string& ti = "unspecified", const std::string& desc = "unspecified", int u = 3, const date& t_d = date{}, double p = 0, bool d = false, bool m = false) : title{ti}, description{desc}, urgency{std::clamp(u, 1, max_level)}, task_deadline{t_d}, progress{p}, done{d}, missed{m}{}

    const std::string& get_title() const { return title; }
    bool is_done() const { return done; }
    bool is_missed() const { return missed; }
    bool has_level(int l) const { return urgency == l && !done; }

    void finish(){
      done = true;
      missed = false;
      progress = 100;
    }

    void set_progress(double p){
      progress = std::clamp(p, 0.0, 100.0);
      if(progress >= 100) finish();
    }

    void check_missed(const date& today){
      if(!done && days_left(today) < 0) missed = true;
    }

    
    bool more_urgent(const task& o) const {
      if(done != o.done) return !done;
      return urgency < o.urgency;
    }

    friend std::istream& operator>>(std::istream& is, task& t){
      char sep = ',';
      double p = 0;
      std::getline(is, t.title, ',');
      std::getline(is, t.description, ',');
      if(is >> t.urgency >> sep >> t.task_deadline >> sep >> p){
        t.urgency = std::clamp(t.urgency, 1, max_level);
        t.done = t.missed = false;
        t.set_progress(p);
      }
      return is;
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

    double subscriptions_cost() const {
      double sum = 0;
      for(const auto& t : transactions)
        if(t.is_membership() && t.signed_amount() < 0) sum -= t.signed_amount();
      return sum;
    }

    void monthly_report(const date& d) const {
      double earned = 0, spent = 0, biggest = 0;
      for(const auto& t : transactions){
        if(!t.in_month(d)) continue;
        double s = t.signed_amount();
        if(s > 0) earned += s;
        else{
          spent -= s;
          biggest = std::max(biggest, -s);
        }
      }
      std::cout << "Month report: earned " << earned << ", spent " << spent << ", saved " << earned - spent << ", biggest expense " << biggest << "\n";
    }

    void category_report(const date& d) const {
      std::map<std::string, double> sums;
      for(const auto& t : transactions)
        if(t.in_month(d) && t.signed_amount() < 0) sums[t.category_name()] -= t.signed_amount();
      std::cout << "Spending by category:\n";
      for(const auto& p : sums) std::cout << "  " << p.first << ": " << p.second << "\n";
    }

    void goals_status(const date& today) const {
      for(const auto& g : goals){
        double target = g.get_money();
        int days = g.days_left(today);
        std::cout << "  " << g;
        if(target <= 0 || left >= target) std::cout << "    reached!\n";
        else if(days <= 0) std::cout << "    deadline passed, missing " << target - left << "\n";
        else std::cout << "    " << left / target * 100 << "% done, save " << (target - left) / (days / 30.0) << " / month\n";
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

    
    static bool higher(const task& a, const task& b){
      std::cout << "Is \"" << a.get_title() << "\" higher or lower than \"" << b.get_title() << "\" on the list? (1 = higher, 2 = lower): ";
      int ans = 0;
      while(ans != 1 && ans != 2){
        if(!(std::cin >> ans)){
          if(std::cin.eof()) ans = 2;
          else{
            std::cin.clear();
            std::cin.ignore(1000, '\n');
          }
        }
      }
      std::cout << "\n";
      return ans == 1;
    }

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

   
    void rank_level(int level){
      if(level < 1 || level > task::levels()){
        std::cout << "Invalid level\n";
        return;
      }
      std::cout << "Ranking level " << level << ":\n";
      std::vector<task> ranked, others;
      for(const auto& t : tasks){
        if(!t.has_level(level)){
          others.push_back(t);
          continue;
        }
        std::size_t lo = 0, hi = ranked.size();
        while(lo < hi){
          std::size_t mid = (lo + hi) / 2;
          if(higher(t, ranked[mid])) hi = mid;
          else lo = mid + 1;
        }
        ranked.insert(ranked.begin() + static_cast<std::ptrdiff_t>(lo), t);
      }
      others.insert(others.end(), ranked.begin(), ranked.end());
      tasks = others;
    }

    
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

template<typename T>
bool parse_line(const std::string& line, T& item){
  std::istringstream row{line};
  return static_cast<bool>(row >> item);
}

template<typename T>
std::vector<T> load_all(const std::string& path){
  std::vector<T> items;
  std::ifstream file{path};
  if(!file) std::cerr << "Cannot open " << path << "\n";
  std::string line;
  while(std::getline(file, line)){
    if(line.empty() || line[0] == '#') continue;
    T item;
    if(parse_line(line, item)) items.push_back(item);
    else std::cerr << path << ": skipped \"" << line << "\"\n";
  }
  return items;
}

template<typename T>
bool read_item(const std::string& prompt, T& item){
  std::string line;
  std::cout << prompt;
  std::getline(std::cin >> std::ws, line);
  std::cout << "\n";
  if(parse_line(line, item)) return true;
  std::cout << "Invalid input\n";
  return false;
}

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

    void next_day(){
      current_date = current_date.tomorrow();
      user_tasks.update_missed(current_date);
    }

    void summary() const {
      std::cout << "=== " << User << ", " << current_date << " ===\n";
      user_budget.monthly_report(current_date);
      user_budget.category_report(current_date);
      user_budget.goals_status(current_date);
      std::cout << "Tasks done: " << user_tasks.completion() << "%\n";
      std::vector<task> top = user_tasks.prioritize();
      if(!top.empty()) std::cout << "Next up: " << top.front();
    }

    friend std::ostream& operator<<(std::ostream& os, const lifetracker& l){
      return os << "User: " << l.User << " | date " << l.current_date << "\n" << l.user_budget << l.user_tasks;
    }
};

int main() {

    std::cout << std::boolalpha;

    std::ifstream user_file{"data/user.txt"};
    std::string user, pass;
    double balance = 0;
    char sep = ',';
    date start;
    std::getline(user_file, user, ',');
    std::getline(user_file, pass, ',');
    user_file >> start >> sep >> balance;
    if(!user_file){
      std::cerr << "Cannot read data/user.txt\n";
      return 1;
    }

    std::vector<transaction> txs = load_all<transaction>("data/transactions.csv");
    std::vector<money_goal> goals = load_all<money_goal>("data/goals.csv");
    std::vector<task> tasks = load_all<task>("data/tasks.csv");
    if(txs.empty() || goals.empty() || tasks.size() < 2){
      std::cerr << "Missing or empty data files\n";
      return 1;
    }

    // date
    std::cout << "Start: " << start << ", tomorrow: " << start.tomorrow() << ", days until tomorrow: " << start.days_until(start.tomorrow()) << ", same month: " << start.same_month(start.tomorrow()) << "\n\n";

    // transactions + goals
    const transaction& first_tx = txs.front();
    std::cout << first_tx << first_tx.category_name() << " " << first_tx.signed_amount() << " " << first_tx.is_membership() << " " << first_tx.in_month(start) << "\n";
    const money_goal& first_goal = goals.front();
    std::cout << first_goal << first_goal.get_money() << " " << first_goal.days_left(start) << "\n\n";

    // budget 
    budget b{balance, txs, goals};
    b.monthly_report(start);
    b.category_report(start);
    b.goals_status(start);
    std::cout << "Subscriptions: " << b.subscriptions_cost() << "\n" << b;
    budget copy{b};
    budget other;
    other = copy;
    std::cout << "\n";

    // tasks
    task& first_task = tasks.front();
    first_task.check_missed(start);
    std::cout << first_task << first_task.get_title() << " more urgent than " << tasks.back().get_title() << ": " << first_task.more_urgent(tasks.back()) << ", level 1: " << first_task.has_level(1) << ", done: " << first_task.is_done() << ", missed: " << first_task.is_missed() << "\n\n";

    // task tracker: ranking
    task_tracker tt{tasks};
    int level = 0;
    std::cout << "Which level do you want to rank? (1 = most urgent, " << task::levels() << " = least urgent): ";
    std::cin >> level;
    std::cout << "\n";
    tt.rank_level(level);
    std::cout << "Priority list:\n";
    for(const auto& t : tt.prioritize()) std::cout << "  " << t;
    tt.update_missed(start);
    std::string title;
    std::cout << "Which task did you finish? ";
    std::getline(std::cin >> std::ws, title);
    std::cout << "\n" << (tt.complete(title) ? "Completed " : "No open task named ") << title << "\n";
    std::cout << tt << tt.completion() << "%\n\n";

    // lifetracker
    lifetracker app{user, pass, start, b, tt};
    std::string typed_user, typed_pass;
    std::cout << "Login (user password): ";
    std::cin >> typed_user >> typed_pass;
    std::cout << "\n";
    if(!app.login(typed_user, typed_pass)){
      std::cout << "Wrong credentials\n";
      return 1;
    }

    transaction new_tx;
    if(read_item("New transaction (title,category,d.m.y,amount,subscription,win): ", new_tx)) app.add_transaction(new_tx);
    task new_task;
    if(read_item("New task (title,description,level,d.m.y,progress): ", new_task)) app.add_task(new_task);
    int days = 0;
    std::cout << "Days to advance: ";
    std::cin >> days;
    std::cout << "\n";
    for(int i = 0; i < days; i++) app.next_day();
    app.summary();
    std::cout << app << "\n";

    return 0;
}
