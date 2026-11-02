#include <algorithm>
#include <cmath>
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
      static const std::vector<int> dm = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      return (month == 2 && leap()) ? 29 : dm[static_cast<std::size_t>(month - 1)];
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

    int get_day() const { return day; }
    int get_month() const { return month; }
    int get_year() const { return year; }

    int days_until(const date& o) const { return o.to_days() - to_days(); }

    date tomorrow() const {
      if(day < days_in_month()) return date{day + 1, month, year};
      if(month < 12) return date{1, month + 1, year};
      return date{1, 1, year + 1};
    }

    date yesterday() const {
      if(day > 1) return date{day - 1, month, year};
      if(month > 1){
        int pm = month - 1;
        return date{date{1, pm, year}.days_in_month(), pm, year};
      }
      return date{31, 12, year - 1};
    }

    date add_days(int n) const {
      date cur = *this;
      for(int i = 0; i < n; i++) cur = cur.tomorrow();
      for(int i = 0; i > n; i--) cur = cur.yesterday();
      return cur;
    }

    int day_of_week() const {
      int d = to_days() % 7;
      return d < 0 ? d + 7 : d;
    }

    std::string weekday_name() const {
      static const std::vector<std::string> wn = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
      return wn[static_cast<std::size_t>(day_of_week())];
    }

    bool is_weekend() const {
      int dw = day_of_week();
      return dw == 0 || dw == 6;
    }

    int quarter() const { return (month - 1) / 3 + 1; }

    date next_billing(const date& today) const {
      int max_d = date{1, today.month, today.year}.days_in_month();
      date due{std::min(day, max_d), today.month, today.year};
      if(today.days_until(due) > 0) return due;
      int nm = today.month == 12 ? 1 : today.month + 1;
      int ny = today.month == 12 ? today.year + 1 : today.year;
      return date{std::min(day, date{1, nm, ny}.days_in_month()), nm, ny};
    }

    bool same_day(const date& o) const { return day == o.day && month == o.month && year == o.year; }
    bool same_week(const date& o) const { int diff = days_until(o); return diff >= -6 && diff <= 6; }
    bool same_month(const date& o) const { return month == o.month && year == o.year; }
    bool same_year(const date& o) const { return year == o.year; }
    int ics_code() const { return year * 10000 + month * 100 + day; }

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

    const std::string& get_title() const { return title; }
    const date& get_date() const { return date_transaction; }
    std::string category_name() const { return names()[static_cast<std::size_t>(cat)]; }

    double signed_amount() const { return is_income() ? amount : -amount; }
    double raw_amount() const { return amount; }
    bool is_membership() const { return membership; }
    bool is_win() const { return win; }

    bool is_essential() const {
      return cat == category::food || cat == category::transportation || cat == category::bills || cat == category::housing || cat == category::health || cat == category::education;
    }

    bool in_day(const date& d) const { return date_transaction.same_day(d); }
    bool in_week(const date& d) const { return date_transaction.same_week(d); }
    bool in_month(const date& d) const { return date_transaction.same_month(d); }
    bool in_year(const date& d) const { return date_transaction.same_year(d); }
    date next_billing_date(const date& today) const { return date_transaction.next_billing(today); }

    void apply_discount(double percent){
      double p = std::clamp(percent, 0.0, 100.0);
      amount = amount * (1.0 - p / 100.0);
    }

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

    const std::string& get_title() const { return title; }
    double get_money() const { return money; }
    const date& get_deadline() const { return goal_deadline; }
    int days_left(const date& today) const { return today.days_until(goal_deadline); }

    double daily_target(double current_saved, const date& today) const {
      int d = days_left(today);
      if(d <= 0 || current_saved >= money) return 0;
      return (money - current_saved) / static_cast<double>(d);
    }

    void extend_deadline(int extra_days){
      if(extra_days > 0) goal_deadline = goal_deadline.add_days(extra_days);
    }

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
    const std::string& get_description() const { return description; }
    const date& get_deadline() const { return task_deadline; }
    int get_urgency() const { return urgency; }
    double get_progress() const { return progress; }
    bool is_done() const { return done; }
    bool is_missed() const { return missed; }
    bool has_level(int l) const { return urgency == l && !done; }
    int deadline_diff(const task& o) const { return task_deadline.days_until(o.task_deadline); }

    double urgency_score(const date& today) const {
      if(done) return 0;
      int d = std::max(1, days_left(today));
      double base = static_cast<double>(max_level + 1 - urgency) * 100.0;
      double remaining = (100.0 - progress) / 100.0;
      return (base * remaining) / static_cast<double>(d);
    }

    void finish(){
      done = true;
      missed = false;
      progress = 100;
    }

    void set_progress(double p){
      progress = std::clamp(p, 0.0, 100.0);
      if(progress >= 100) finish();
    }

    void postpone(int extra_days){
      if(extra_days > 0){
        task_deadline = task_deadline.add_days(extra_days);
        missed = false;
      }
    }

    void escalate(){
      if(urgency > 1) urgency--;
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

    void report_range(const std::string& label, const date& d, int mode) const {
      double earned = 0, spent = 0, biggest = 0;
      for(const auto& t : transactions){
        bool match = (mode == 0 && t.in_day(d)) || (mode == 1 && t.in_week(d)) || (mode == 2 && t.in_month(d)) || (mode == 3 && t.in_year(d));
        if(!match) continue;
        double s = t.signed_amount();
        if(s > 0) earned += s;
        else{
          spent -= s;
          biggest = std::max(biggest, -s);
        }
      }
      std::cout << label << " report: earned " << earned << ", spent " << spent << ", saved " << earned - spent << ", biggest expense " << biggest << "\n";
    }

  public:
    explicit budget(double t = 0, const std::vector<transaction>& tr = {}, const std::vector<money_goal>& go = {}) : total{t}, left{t}, lost{0}, transactions{tr}, goals{go}{
      for(const auto& x : transactions) update(x);
    }

    budget(const budget& o) : total{o.total}, left{o.left}, lost{o.lost}, transactions{o.transactions}, goals{o.goals}{
      std::cout << "[budget copy constructor]\n";
    }

    budget& operator=(const budget& o){
      if(this != &o){
        total = o.total;
        left = o.left;
        lost = o.lost;
        transactions = o.transactions;
        goals = o.goals;
      }
      std::cout << "[budget operator=]\n";
      return *this;
    }

    ~budget(){ std::cout << "[budget destructor]\n"; }

    double get_left() const { return left; }

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

    void subscriptions_status(const date& today) const {
      double reserve = 0;
      std::cout << "Subscriptions tracker:\n";
      for(const auto& t : transactions){
        if(!t.is_membership() || t.signed_amount() >= 0) continue;
        double cost = -t.signed_amount();
        date due = t.next_billing_date(today);
        int days = std::max(1, today.days_until(due));
        reserve += cost;
        std::cout << "  " << t.get_title() << ": " << cost << " due on " << due << " (" << days << " days left, set aside " << cost / static_cast<double>(days) << "/day)\n";
      }
      std::cout << "  Total to keep aside for subscriptions: " << reserve << " (available after reserve: " << left - reserve << ")\n";
    }

    void monthly_report(const date& d) const { report_range("Month", d, 2); }

    void period_report(const date& d) const {
      report_range("Day", d, 0);
      report_range("Week", d, 1);
      report_range("Month", d, 2);
      report_range("Year", d, 3);
    }

    void category_report(const date& d) const {
      std::map<std::string, double> sums;
      for(const auto& t : transactions)
        if(t.in_month(d) && t.signed_amount() < 0) sums[t.category_name()] -= t.signed_amount();
      std::cout << "Spending by category:\n";
      for(const auto& p : sums) std::cout << "  " << p.first << ": " << p.second << "\n";
    }

    void spending_split(const date& d) const {
      double essential = 0, discretionary = 0, wins = 0;
      for(const auto& t : transactions){
        if(!t.in_month(d)) continue;
        if(t.is_win() && t.signed_amount() > 0) wins += t.signed_amount();
        if(t.signed_amount() >= 0) continue;
        if(t.is_essential()) essential -= t.signed_amount();
        else discretionary -= t.signed_amount();
      }
      double sum = essential + discretionary;
      double ess_pct = sum > 0 ? essential * 100.0 / sum : 0;
      std::cout << "Monthly split: essential " << essential << " (" << ess_pct << "%), discretionary " << discretionary << ", bonus wins " << wins << "\n";
    }

    double forecast_balance(int months_ahead, const date& d) const {
      double earned = 0, spent = 0;
      for(const auto& t : transactions){
        if(!t.in_month(d)) continue;
        if(t.signed_amount() > 0) earned += t.signed_amount();
        else spent -= t.signed_amount();
      }
      double net_monthly = earned - spent;
      double projected = left + net_monthly * static_cast<double>(months_ahead);
      std::cout << "Forecast in " << months_ahead << " months (net " << net_monthly << "/month): " << projected << "\n";
      return projected;
    }

    void goals_status(const date& today) const {
      for(const auto& g : goals){
        double target = g.get_money();
        int days = g.days_left(today);
        std::cout << "  " << g;
        if(target <= 0 || left >= target) std::cout << "    reached!\n";
        else if(days <= 0) std::cout << "    deadline passed, missing " << target - left << "\n";
        else std::cout << "    " << left / target * 100 << "% done, save " << (target - left) / (static_cast<double>(days) / 30.0) << " / month (" << g.daily_target(left, today) << " / day)\n";
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
      int diff = a.deadline_diff(b);
      std::cout << "Compare [" << a.get_title() << " | due " << a.get_deadline() << "] vs [" << b.get_title() << " | due " << b.get_deadline() << "] (deadline diff: " << (diff >= 0 ? "+" : "") << diff << " days)\n";
      std::cout << "Is \"" << a.get_title() << "\" more important than \"" << b.get_title() << "\"? (1 = yes, 0 = no): ";
      int ans = -1;
      while(ans != 1 && ans != 0 && ans != 2){
        if(!(std::cin >> ans)){
          if(std::cin.eof()) ans = 0;
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

    bool complete(const std::string& task_title){
      for(auto& t : tasks)
        if(t.get_title() == task_title && !t.is_done()){
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

    int escalate_due_soon(const date& today, int threshold_days){
      int count = 0;
      for(auto& t : tasks){
        if(t.is_done()) continue;
        int d = today.days_until(t.get_deadline());
        if(d >= 0 && d <= threshold_days && t.get_urgency() > 1){
          t.escalate();
          count++;
        }
      }
      return count;
    }

    void focus_plan(const date& today, int max_items) const {
      std::vector<task> open;
      for(const auto& t : tasks)
        if(!t.is_done()) open.push_back(t);
      std::stable_sort(open.begin(), open.end(), [&today](const task& a, const task& b){
        return a.urgency_score(today) > b.urgency_score(today);
      });
      std::cout << "Smart focus plan for " << today << ":\n";
      int shown = 0;
      for(const auto& t : open){
        if(shown >= max_items) break;
        std::cout << "  #" << (shown + 1) << " [score " << t.urgency_score(today) << "] " << t;
        shown++;
      }
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

class calendar_event{

  private:
    std::string title;
    date event_date;
    int start_hour, duration_min;
    bool synced;

    int start_minutes() const { return start_hour * 60; }
    int end_minutes() const { return start_hour * 60 + duration_min; }

  public:
    explicit calendar_event(const std::string& ti = "unspecified", const date& d = date{}, int sh = 9, int dur = 60, bool sy = false) : title{ti}, event_date{d}, start_hour{std::clamp(sh, 0, 23)}, duration_min{std::max(15, dur)}, synced{sy}{}

    const std::string& get_title() const { return title; }
    const date& get_date() const { return event_date; }
    int get_start_hour() const { return start_hour; }
    int get_duration() const { return duration_min; }
    bool is_synced() const { return synced; }

    void mark_synced(){ synced = true; }

    void reschedule(const date& d, int sh){
      event_date = d;
      start_hour = std::clamp(sh, 0, 23);
      synced = false;
    }

    bool overlaps(const calendar_event& o) const {
      if(!event_date.same_day(o.event_date)) return false;
      return start_minutes() < o.end_minutes() && o.start_minutes() < end_minutes();
    }

    void write_ics(std::ostream& os) const {
      os << "BEGIN:VEVENT\nSUMMARY:" << title << "\nDTSTART:" << event_date.ics_code() << "T" << (start_hour < 10 ? "0" : "") << start_hour << "0000\nDURATION:PT" << duration_min << "M\nEND:VEVENT\n";
    }

    friend std::istream& operator>>(std::istream& is, calendar_event& e){
      char sep = ',';
      std::getline(is, e.title, ',');
      if(is >> e.event_date >> sep >> e.start_hour >> sep >> e.duration_min >> sep >> e.synced){
        e.start_hour = std::clamp(e.start_hour, 0, 23);
        e.duration_min = std::max(15, e.duration_min);
      }
      return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const calendar_event& e){
      return os << e.title << " | " << e.event_date << " at " << e.start_hour << ":00 (" << e.duration_min << "m) | " << (e.synced ? "google-synced" : "local-modified") << "\n";
    }
};

class calendar{

  private:
    std::string google_account;
    std::vector<calendar_event> events;

    bool has_conflict(const calendar_event& ev) const {
      for(const auto& x : events)
        if(x.overlaps(ev)) return true;
      return false;
    }

  public:
    explicit calendar(const std::string& acc = "user@gmail.com", const std::vector<calendar_event>& ev = {}) : google_account{acc}, events{ev}{}

    bool add_event(const calendar_event& e){
      bool clash = has_conflict(e);
      if(clash) std::cout << "Calendar conflict detected for: " << e.get_title() << "\n";
      events.push_back(e);
      return !clash;
    }

    bool modify_event(const std::string& ev_title, const date& new_date, int new_hour){
      for(auto& e : events)
        if(e.get_title() == ev_title){
          e.reschedule(new_date, new_hour);
          return true;
        }
      return false;
    }

    int detect_conflicts() const {
      int count = 0;
      for(std::size_t i = 0; i < events.size(); i++)
        for(std::size_t j = i + 1; j < events.size(); j++)
          if(events[i].overlaps(events[j])){
            std::cout << "  Conflict: \"" << events[i].get_title() << "\" overlaps with \"" << events[j].get_title() << "\" on " << events[i].get_date() << "\n";
            count++;
          }
      return count;
    }

    int auto_resolve_conflicts(){
      int fixed = 0;
      for(std::size_t i = 0; i < events.size(); i++){
        for(std::size_t j = i + 1; j < events.size(); j++){
          if(!events[i].overlaps(events[j])) continue;
          date d = events[j].get_date();
          int dur = events[j].get_duration();
          for(int h = 8; h <= 21; h++){
            calendar_event probe{events[j].get_title(), d, h, dur, false};
            if(!has_conflict(probe)){
              events[j].reschedule(d, h);
              fixed++;
              break;
            }
          }
        }
      }
      return fixed;
    }

    int find_free_slot(const date& d, int slot_minutes) const {
      for(int h = 8; h <= 21; h++){
        calendar_event candidate{"slot", d, h, slot_minutes, false};
        if(!has_conflict(candidate)) return h;
      }
      return -1;
    }

    void week_agenda(const date& today) const {
      std::cout << "Week agenda (" << google_account << "):\n";
      for(const auto& e : events){
        int d = today.days_until(e.get_date());
        if(d >= 0 && d < 7) std::cout << "  " << e;
      }
    }

    int sync_google(const std::string& ics_path){
      std::ofstream out{ics_path};
      out << "BEGIN:VCALENDAR\nVERSION:2.0\nX-WR-CALNAME:" << google_account << "\n";
      int pushed = 0;
      for(auto& e : events){
        if(!e.is_synced()){
          e.mark_synced();
          pushed++;
        }
        e.write_ics(out);
      }
      out << "END:VCALENDAR\n";
      std::cout << "Google Calendar sync (" << google_account << "): pushed " << pushed << " updates to " << ics_path << "\n";
      return pushed;
    }

    friend std::ostream& operator<<(std::ostream& os, const calendar& c){
      os << "Calendar (" << c.google_account << ", " << c.events.size() << " events):\n";
      for(const auto& e : c.events) os << "  " << e;
      return os;
    }
};

class roadmap_node{

  private:
    int id, parent_id;
    std::string title;
    double weight;
    bool done;

  public:
    explicit roadmap_node(int i = 1, int p = 0, const std::string& ti = "unspecified", double w = 1.0, bool d = false) : id{i}, parent_id{p}, title{ti}, weight{std::max(0.1, w)}, done{d}{}

    int get_id() const { return id; }
    int get_parent() const { return parent_id; }
    const std::string& get_title() const { return title; }
    double get_weight() const { return weight; }
    bool is_done() const { return done; }
    void finish(){ done = true; }

    friend std::istream& operator>>(std::istream& is, roadmap_node& n){
      char sep = ',';
      if(is >> n.id >> sep >> n.parent_id >> sep){
        std::getline(is, n.title, ',');
        is >> n.weight >> sep >> n.done;
        n.weight = std::max(0.1, n.weight);
      }
      return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const roadmap_node& n){
      return os << "[" << (n.done ? "x" : " ") << "] #" << n.id << " (parent " << n.parent_id << ") " << n.title << " | weight " << n.weight << "\n";
    }
};

class roadmap{

  private:
    std::string main_goal;
    std::vector<roadmap_node> nodes;

    std::vector<std::size_t> children_of(int pid) const {
      std::vector<std::size_t> ch;
      for(std::size_t i = 0; i < nodes.size(); i++)
        if(nodes[i].get_parent() == pid) ch.push_back(i);
      return ch;
    }

    double subtree_progress(std::size_t idx) const {
      std::vector<std::size_t> order;
      order.push_back(idx);
      for(std::size_t k = 0; k < order.size(); k++)
        for(std::size_t c : children_of(nodes[order[k]].get_id()))
          order.push_back(c);
      std::vector<double> prog(nodes.size(), 0.0);
      for(std::size_t k = order.size(); k > 0; k--){
        std::size_t u = order[k - 1];
        std::vector<std::size_t> ch = children_of(nodes[u].get_id());
        if(ch.empty()) prog[u] = nodes[u].is_done() ? 100.0 : 0.0;
        else{
          double sum_w = 0, done_w = 0;
          for(std::size_t c : ch){
            double w = nodes[c].get_weight();
            sum_w += w;
            done_w += w * (prog[c] / 100.0);
          }
          prog[u] = sum_w > 0 ? (done_w / sum_w) * 100.0 : 0.0;
        }
      }
      return prog[idx];
    }

    double subtree_effort(std::size_t idx) const {
      std::vector<std::size_t> queue;
      queue.push_back(idx);
      double rem = 0;
      for(std::size_t k = 0; k < queue.size(); k++){
        std::size_t u = queue[k];
        std::vector<std::size_t> ch = children_of(nodes[u].get_id());
        if(ch.empty()){
          if(!nodes[u].is_done()) rem += nodes[u].get_weight();
        }else{
          for(std::size_t c : ch) queue.push_back(c);
        }
      }
      return rem;
    }

    void refresh_parents(){
      for(std::size_t pass = 0; pass < nodes.size(); pass++)
        for(std::size_t i = 0; i < nodes.size(); i++){
          std::vector<std::size_t> ch = children_of(nodes[i].get_id());
          if(!ch.empty() && !nodes[i].is_done() && subtree_progress(i) >= 99.9)
            nodes[i].finish();
        }
    }

    std::string find_leaf(int start_pid) const {
      std::vector<std::size_t> st;
      std::vector<std::size_t> roots = children_of(start_pid);
      for(std::size_t i = roots.size(); i > 0; i--) st.push_back(roots[i - 1]);
      while(!st.empty()){
        std::size_t u = st.back();
        st.pop_back();
        if(nodes[u].is_done()) continue;
        std::vector<std::size_t> ch = children_of(nodes[u].get_id());
        if(ch.empty()) return nodes[u].get_title();
        for(std::size_t i = ch.size(); i > 0; i--) st.push_back(ch[i - 1]);
      }
      return "";
    }

    void build_critical_path(int start_pid, std::vector<std::string>& path) const {
      int cur_pid = start_pid;
      for(std::size_t step = 0; step <= nodes.size(); step++){
        std::vector<std::size_t> ch = children_of(cur_pid);
        double best_effort = -1.0;
        std::size_t best_idx = 0;
        for(std::size_t c : ch){
          if(nodes[c].is_done()) continue;
          double eff = subtree_effort(c);
          if(eff > best_effort){
            best_effort = eff;
            best_idx = c;
          }
        }
        if(best_effort <= 0) break;
        path.push_back(nodes[best_idx].get_title());
        cur_pid = nodes[best_idx].get_id();
      }
    }

    void print_branch(std::ostream& os, std::size_t root_idx, const std::string& base_prefix, bool root_last) const {
      struct frame {
        std::size_t idx{0};
        std::string prefix{};
        bool is_last{false};
      };
      std::vector<frame> st;
      st.push_back(frame{root_idx, base_prefix, root_last});
      while(!st.empty()){
        frame cur = st.back();
        st.pop_back();
        os << cur.prefix << (cur.is_last ? "\\-- " : "|-- ") << "[" << (nodes[cur.idx].is_done() ? "x" : " ") << "] " << nodes[cur.idx].get_title() << " (" << subtree_progress(cur.idx) << "%, rem weight " << subtree_effort(cur.idx) << ")\n";
        std::vector<std::size_t> ch = children_of(nodes[cur.idx].get_id());
        std::string next_prefix = cur.prefix + (cur.is_last ? "    " : "|   ");
        for(std::size_t i = ch.size(); i > 0; i--)
          st.push_back(frame{ch[i - 1], next_prefix, i == ch.size()});
      }
    }

  public:
    explicit roadmap(const std::string& mg = "Main Goal", const std::vector<roadmap_node>& no = {}) : main_goal{mg}, nodes{no}{
      refresh_parents();
    }

    bool split_problem(const std::string& parent_title, const std::string& sub_title, double sub_weight){
      int pid = 0, max_id = 0;
      for(const auto& n : nodes){
        max_id = std::max(max_id, n.get_id());
        if(n.get_title() == parent_title) pid = n.get_id();
      }
      if(pid == 0 && !nodes.empty()) return false;
      nodes.push_back(roadmap_node{max_id + 1, pid, sub_title, sub_weight, false});
      return true;
    }

    bool complete_step(const std::string& step_title){
      for(auto& n : nodes)
        if(n.get_title() == step_title && !n.is_done()){
          n.finish();
          refresh_parents();
          return true;
        }
      return false;
    }

    double overall_progress() const {
      std::vector<std::size_t> roots = children_of(0);
      if(roots.empty()) return 0;
      double sum = 0;
      for(std::size_t r : roots) sum += subtree_progress(r);
      return sum / static_cast<double>(roots.size());
    }

    std::string next_step() const {
      std::string step = find_leaf(0);
      return step.empty() ? "All steps completed!" : step;
    }

    void print_critical_path() const {
      std::vector<std::string> path;
      build_critical_path(0, path);
      std::cout << "Critical path (" << path.size() << " steps): ";
      for(std::size_t i = 0; i < path.size(); i++)
        std::cout << (i ? " -> " : "") << path[i];
      std::cout << "\n";
    }

    void render_tree(std::ostream& os = std::cout) const {
      os << "Roadmap [" << main_goal << "] (" << overall_progress() << "%):\n";
      std::vector<std::size_t> roots = children_of(0);
      for(std::size_t i = 0; i < roots.size(); i++)
        print_branch(os, roots[i], "  ", i + 1 == roots.size());
    }

    friend std::ostream& operator<<(std::ostream& os, const roadmap& rm){
      rm.render_tree(os);
      for(const auto& n : rm.nodes) os << "  " << n;
      return os;
    }
};

class habit{

  private:
    std::string title, category_tag;
    int target_days_per_week, current_streak, best_streak;
    date last_completed;

  public:
    explicit habit(const std::string& ti = "unspecified", const std::string& ct = "health", int td = 3, int cs = 0, int bs = 0, const date& lc = date{}) : title{ti}, category_tag{ct}, target_days_per_week{std::clamp(td, 1, 7)}, current_streak{std::max(0, cs)}, best_streak{std::max(cs, bs)}, last_completed{lc}{}

    const std::string& get_title() const { return title; }
    const std::string& get_category() const { return category_tag; }
    int get_streak() const { return current_streak; }
    int get_best_streak() const { return best_streak; }

    void check_in(const date& today){
      int diff = last_completed.days_until(today);
      if(diff == 0) return;
      if(diff == 1 || diff <= 7 / target_days_per_week + 1) current_streak++;
      else current_streak = 1;
      best_streak = std::max(best_streak, current_streak);
      last_completed = today;
    }

    void verify_decay(const date& today){
      int max_gap = 7 / target_days_per_week + 2;
      if(last_completed.days_until(today) > max_gap) current_streak = 0;
    }

    double consistency_score() const {
      if(best_streak <= 0) return 0;
      return std::min(100.0, static_cast<double>(current_streak) * 100.0 / static_cast<double>(best_streak));
    }

    friend std::istream& operator>>(std::istream& is, habit& h){
      char sep = ',';
      std::getline(is, h.title, ',');
      std::getline(is, h.category_tag, ',');
      if(is >> h.target_days_per_week >> sep >> h.current_streak >> sep >> h.best_streak >> sep >> h.last_completed){
        h.target_days_per_week = std::clamp(h.target_days_per_week, 1, 7);
        h.best_streak = std::max(h.current_streak, h.best_streak);
      }
      return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const habit& h){
      return os << h.title << " [" << h.category_tag << "] | " << h.target_days_per_week << "x/week | streak " << h.current_streak << " (best " << h.best_streak << ") | last " << h.last_completed << " | consistency " << h.consistency_score() << "%\n";
    }
};

class habit_tracker{

  private:
    std::vector<habit> habits;

  public:
    explicit habit_tracker(const std::vector<habit>& ha = {}) : habits{ha}{}

    void add_habit(const habit& h){ habits.push_back(h); }

    bool log_habit(const std::string& hab_title, const date& today){
      for(auto& h : habits)
        if(h.get_title() == hab_title){
          h.check_in(today);
          return true;
        }
      return false;
    }

    void update_streaks(const date& today){
      for(auto& h : habits) h.verify_decay(today);
    }

    double average_consistency() const {
      if(habits.empty()) return 0;
      double sum = 0;
      for(const auto& h : habits) sum += h.consistency_score();
      return sum / static_cast<double>(habits.size());
    }

    friend std::ostream& operator<<(std::ostream& os, const habit_tracker& ht){
      os << "Habits (" << ht.habits.size() << " tracked, avg consistency " << ht.average_consistency() << "%):\n";
      for(const auto& h : ht.habits) os << "  " << h;
      return os;
    }
};

class workout_session{

  private:
    std::string title, sport;
    int duration_min, calories, intensity;
    date session_date;

  public:
    explicit workout_session(const std::string& ti = "unspecified", const std::string& sp = "general", int dur = 45, int cal = 300, int in_lvl = 5, const date& sd = date{}) : title{ti}, sport{sp}, duration_min{std::max(5, dur)}, calories{std::max(0, cal)}, intensity{std::clamp(in_lvl, 1, 10)}, session_date{sd}{}

    const std::string& get_title() const { return title; }
    const std::string& get_sport() const { return sport; }
    int get_duration() const { return duration_min; }
    int get_calories() const { return calories; }
    int get_intensity() const { return intensity; }
    const date& get_date() const { return session_date; }

    double training_load() const {
      return static_cast<double>(duration_min * intensity) / 10.0;
    }

    friend std::istream& operator>>(std::istream& is, workout_session& w){
      char sep = ',';
      std::getline(is, w.title, ',');
      std::getline(is, w.sport, ',');
      if(is >> w.duration_min >> sep >> w.calories >> sep >> w.intensity >> sep >> w.session_date){
        w.duration_min = std::max(5, w.duration_min);
        w.calories = std::max(0, w.calories);
        w.intensity = std::clamp(w.intensity, 1, 10);
      }
      return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const workout_session& w){
      return os << w.title << " (" << w.sport << ") | " << w.session_date << " | " << w.duration_min << "m | " << w.calories << " kcal | intensity " << w.intensity << "/10 | load " << w.training_load() << "\n";
    }
};

class fitness_tracker{

  private:
    int weekly_calorie_goal;
    std::vector<workout_session> sessions;

  public:
    explicit fitness_tracker(int wcg = 2500, const std::vector<workout_session>& se = {}) : weekly_calorie_goal{std::max(500, wcg)}, sessions{se}{}

    void add_session(const workout_session& ws){ sessions.push_back(ws); }

    int weekly_calories(const date& today) const {
      int sum = 0;
      for(const auto& s : sessions)
        if(s.get_date().same_week(today)) sum += s.get_calories();
      return sum;
    }

    double weekly_load(const date& today) const {
      double load = 0;
      for(const auto& s : sessions)
        if(s.get_date().same_week(today)) load += s.training_load();
      return load;
    }

    double goal_completion(const date& today) const {
      return std::min(100.0, static_cast<double>(weekly_calories(today)) * 100.0 / static_cast<double>(weekly_calorie_goal));
    }

    void sport_breakdown() const {
      std::map<std::string, int> mins;
      for(const auto& s : sessions) mins[s.get_sport()] += s.get_duration();
      std::cout << "Training minutes by sport:\n";
      for(const auto& p : mins) std::cout << "  " << p.first << ": " << p.second << " min\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const fitness_tracker& ft){
      os << "Fitness (" << ft.sessions.size() << " sessions, weekly target " << ft.weekly_calorie_goal << " kcal):\n";
      for(const auto& s : ft.sessions) os << "  " << s;
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
    if(!line.empty() && line.back() == '\r') line.pop_back();
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
  if(!line.empty() && line.back() == '\r') line.pop_back();
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
    calendar user_calendar;
    roadmap user_roadmap;
    habit_tracker user_habits;
    fitness_tracker user_fitness;

  public:
    explicit lifetracker(const std::string& user = "JhonDoe", const std::string& pass = "password1234", const date& c_d = date{}, const budget& u_b = budget{}, const task_tracker& u_t = task_tracker{}, const calendar& u_c = calendar{}, const roadmap& u_r = roadmap{}, const habit_tracker& u_h = habit_tracker{}, const fitness_tracker& u_f = fitness_tracker{}) : User{user}, Password{pass}, current_date{c_d}, user_budget{u_b}, user_tasks{u_t}, user_calendar{u_c}, user_roadmap{u_r}, user_habits{u_h}, user_fitness{u_f}{}

    bool login(const std::string& u, const std::string& p) const { return u == User && p == Password; }

    void add_transaction(const transaction& t){ user_budget.add_transaction(t); }
    void add_task(const task& t){ user_tasks.add_task(t); }
    void add_event(const calendar_event& e){ user_calendar.add_event(e); }
    void add_habit(const habit& h){ user_habits.add_habit(h); }
    void add_workout(const workout_session& w){ user_fitness.add_session(w); }

    bool check_in_habit(const std::string& hab_title){ return user_habits.log_habit(hab_title, current_date); }

    int sync_deadlines(const std::string& ics_path){
      for(const auto& t : user_tasks.prioritize())
        if(t.has_level(1)){
          int slot = user_calendar.find_free_slot(t.get_deadline(), 60);
          user_calendar.add_event(calendar_event{"Deadline: " + t.get_title(), t.get_deadline(), slot != -1 ? slot : 9, 60, false});
        }
      return user_calendar.sync_google(ics_path);
    }

    double life_score() const {
      double t_score = user_tasks.completion();
      double r_score = user_roadmap.overall_progress();
      double h_score = user_habits.average_consistency();
      double f_score = user_fitness.goal_completion(current_date);
      double b_score = user_budget.get_left() > 0 ? 100.0 : 0.0;
      return (t_score + r_score + h_score + f_score + b_score) / 5.0;
    }

    void next_day(){
      current_date = current_date.tomorrow();
      user_tasks.update_missed(current_date);
      user_tasks.escalate_due_soon(current_date, 2);
      user_habits.update_streaks(current_date);
    }

    void summary() const {
      std::cout << "=== " << User << ", " << current_date << " (" << current_date.weekday_name() << ", Q" << current_date.quarter() << ") ===\n";
      user_budget.period_report(current_date);
      user_budget.category_report(current_date);
      user_budget.spending_split(current_date);
      user_budget.subscriptions_status(current_date);
      user_budget.forecast_balance(3, current_date);
      user_budget.goals_status(current_date);
      std::cout << "Tasks done: " << user_tasks.completion() << "%\n";
      user_tasks.focus_plan(current_date, 3);
      std::vector<task> top = user_tasks.prioritize();
      if(!top.empty()) std::cout << "Next up: " << top.front();
      user_calendar.week_agenda(current_date);
      user_roadmap.render_tree();
      user_roadmap.print_critical_path();
      std::cout << "Next roadmap step: " << user_roadmap.next_step() << "\n";
      std::cout << user_habits;
      user_fitness.sport_breakdown();
      std::cout << "Weekly fitness load: " << user_fitness.weekly_load(current_date) << " (" << user_fitness.goal_completion(current_date) << "% of kcal goal)\n";
      std::cout << "Overall Life Score: " << life_score() << "/100\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const lifetracker& l){
      return os << "User: " << l.User << " | date " << l.current_date << "\n" << l.user_budget << l.user_tasks << l.user_calendar << l.user_roadmap << l.user_habits << l.user_fitness;
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
    std::vector<calendar_event> events = load_all<calendar_event>("data/events.csv");
    std::vector<roadmap_node> nodes = load_all<roadmap_node>("data/roadmap.csv");
    std::vector<habit> habits = load_all<habit>("data/habits.csv");
    std::vector<workout_session> workouts = load_all<workout_session>("data/workouts.csv");
    if(txs.empty() || goals.empty() || tasks.size() < 2 || events.empty() || nodes.empty() || habits.empty() || workouts.empty()){
      std::cerr << "Missing or empty data files\n";
      return 1;
    }

    // date
    std::cout << "Start: " << start << " (" << start.get_day() << "/" << start.get_month() << "/" << start.get_year() << ", " << start.weekday_name() << ", weekend: " << start.is_weekend() << ", Q" << start.quarter() << "), yesterday: " << start.yesterday() << ", tomorrow: " << start.tomorrow() << ", +5 days: " << start.add_days(5) << ", days until tomorrow: " << start.days_until(start.tomorrow()) << ", same day: " << start.same_day(start.tomorrow()) << ", same week: " << start.same_week(start.tomorrow()) << ", same month: " << start.same_month(start.tomorrow()) << ", same year: " << start.same_year(start.tomorrow()) << ", next billing: " << start.next_billing(start) << ", ics: " << start.ics_code() << "\n\n";

    // transactions + goals
    transaction& first_tx = txs.front();
    first_tx.apply_discount(0);
    std::cout << first_tx << first_tx.get_title() << " on " << first_tx.get_date() << " | " << first_tx.category_name() << " | signed: " << first_tx.signed_amount() << " | raw: " << first_tx.raw_amount() << " | sub: " << first_tx.is_membership() << " | win: " << first_tx.is_win() << " | essential: " << first_tx.is_essential() << " | day/week/month/year: " << first_tx.in_day(start) << "/" << first_tx.in_week(start) << "/" << first_tx.in_month(start) << "/" << first_tx.in_year(start) << " | next bill: " << first_tx.next_billing_date(start) << "\n";
    money_goal& first_goal = goals.front();
    first_goal.extend_deadline(10);
    std::cout << first_goal << first_goal.get_title() << " | target: " << first_goal.get_money() << " | due: " << first_goal.get_deadline() << " | days left: " << first_goal.days_left(start) << " | daily: " << first_goal.daily_target(balance, start) << "\n\n";

    // budget
    budget b{balance, txs, goals};
    b.add_goal(money_goal{"New Monitor", 1200, date{1, 5, 2027}});
    b.monthly_report(start);
    b.period_report(start);
    b.category_report(start);
    b.spending_split(start);
    b.subscriptions_status(start);
    b.forecast_balance(6, start);
    b.goals_status(start);
    std::cout << "Subscriptions: " << b.subscriptions_cost() << ", left: " << b.get_left() << "\n" << b;
    budget copy{b};
    budget other;
    other = copy;
    std::cout << "\n";

    // tasks
    task& first_task = tasks.front();
    first_task.check_missed(start);
    first_task.postpone(1);
    std::cout << first_task << first_task.get_title() << " (" << first_task.get_description() << ", level " << first_task.get_urgency() << ", " << first_task.get_progress() << "%, due " << first_task.get_deadline() << ", score " << first_task.urgency_score(start) << ") more urgent than " << tasks.back().get_title() << ": " << first_task.more_urgent(tasks.back()) << ", diff: " << first_task.deadline_diff(tasks.back()) << " days, level 1: " << first_task.has_level(1) << ", done: " << first_task.is_done() << ", missed: " << first_task.is_missed() << "\n\n";

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
    tt.escalate_due_soon(start, 2);
    tt.focus_plan(start, 3);
    std::string title;
    std::cout << "Which task did you finish? ";
    std::getline(std::cin >> std::ws, title);
    if(!title.empty() && title.back() == '\r') title.pop_back();
    std::cout << "\n" << (tt.complete(title) ? "Completed " : "No open task named ") << title << "\n";
    std::cout << tt << tt.completion() << "%\n\n";

    // calendar
    const calendar_event& first_ev = events.front();
    std::cout << "First event: " << first_ev.get_title() << " at " << first_ev.get_start_hour() << ":00 for " << first_ev.get_duration() << "m\n";
    calendar cal{user + "@gmail.com", events};
    std::cout << "Conflicts found: " << cal.detect_conflicts() << "\n";
    std::cout << "Auto-resolved conflicts: " << cal.auto_resolve_conflicts() << "\n";
    std::cout << "Free 60m slot tomorrow: " << cal.find_free_slot(start.tomorrow(), 60) << ":00\n";
    cal.modify_event(events.back().get_title(), start.tomorrow(), 16);
    cal.week_agenda(start);
    cal.sync_google("data/gcal_sync.ics");
    std::cout << cal << "\n";

    // roadmap
    const roadmap_node& first_node = nodes.front();
    std::cout << "Root node #" << first_node.get_id() << " (parent " << first_node.get_parent() << "): " << first_node.get_title() << ", weight " << first_node.get_weight() << ", done: " << first_node.is_done() << "\n";
    roadmap rm{"Launch Life Tracker", nodes};
    rm.split_problem("Smart Task Tracker", "Deadline Delta View", 1.5);
    rm.complete_step("Binary Search Ranking");
    rm.render_tree();
    rm.print_critical_path();
    std::cout << "Overall progress: " << rm.overall_progress() << "%, next step: " << rm.next_step() << "\n" << rm << "\n";

    // habits + fitness
    const habit& first_hab = habits.front();
    std::cout << "First habit: " << first_hab.get_title() << " (" << first_hab.get_category() << "), streak " << first_hab.get_streak() << "/" << first_hab.get_best_streak() << ", score " << first_hab.consistency_score() << "%\n";
    habit_tracker ht{habits};
    ht.log_habit(first_hab.get_title(), start);
    ht.update_streaks(start);
    std::cout << ht << "\n";

    const workout_session& first_ws = workouts.front();
    std::cout << "First workout: " << first_ws.get_title() << " (" << first_ws.get_sport() << ") on " << first_ws.get_date() << ", " << first_ws.get_duration() << "m, " << first_ws.get_calories() << " kcal, intensity " << first_ws.get_intensity() << ", load " << first_ws.training_load() << "\n";
    fitness_tracker ft{2500, workouts};
    ft.sport_breakdown();
    std::cout << "Weekly kcal: " << ft.weekly_calories(start) << ", load: " << ft.weekly_load(start) << ", goal: " << ft.goal_completion(start) << "%\n" << ft << "\n";

    // lifetracker
    lifetracker app{user, pass, start, b, tt, cal, rm, ht, ft};
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
    calendar_event new_ev;
    if(read_item("New event (title,d.m.y,start_hour,duration_min,synced): ", new_ev)) app.add_event(new_ev);
    habit new_hab;
    if(read_item("New habit (title,category,days_per_week,streak,best_streak,d.m.y): ", new_hab)) app.add_habit(new_hab);
    app.check_in_habit(new_hab.get_title());
    workout_session new_ws;
    if(read_item("New workout (title,sport,duration_min,calories,intensity,d.m.y): ", new_ws)) app.add_workout(new_ws);
    app.sync_deadlines("data/gcal_sync.ics");
    int days = 0;
    std::cout << "Days to advance: ";
    std::cin >> days;
    std::cout << "\n";
    for(int i = 0; i < days; i++) app.next_day();
    app.summary();
    std::cout << app << "\n";

    return 0;
}
