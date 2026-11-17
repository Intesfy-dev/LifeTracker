#include "task_tracker.h"

#include <algorithm>
#include <cstddef>

bool task_tracker::higher(const task& a, const task& b) {
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

void task_tracker::recount() {
  total = static_cast<int>(tasks.size());
  done = tbd = missed = 0;
  for(const auto& t : tasks){
    if(t.is_done()) done++;
    else if(t.is_missed()) missed++;
    else tbd++;
  }
}

task_tracker::task_tracker(const std::vector<task>& ta)
  : total{0}, done{0}, tbd{0}, missed{0}, tasks{ta} {
  recount();
}

void task_tracker::add_task(const task& t) {
  tasks.push_back(t);
  recount();
}

bool task_tracker::complete(const std::string& task_title) {
  for(auto& t : tasks)
    if(t.get_title() == task_title && !t.is_done()){
      t.finish();
      recount();
      return true;
    }
  return false;
}

bool task_tracker::reopen(const std::string& task_title, const date& today) {
  for(auto& t : tasks)
    if(t.get_title() == task_title && t.is_done()){
      t.reopen();
      t.check_missed(today);
      recount();
      return true;
    }
  return false;
}

bool task_tracker::remove_task(const std::string& task_title, const date& deadline) {
  for(auto it = tasks.begin(); it != tasks.end(); ++it)
    if(it->get_title() == task_title && it->get_deadline().same_day(deadline)){
      tasks.erase(it);
      recount();
      return true;
    }
  return false;
}

void task_tracker::update_missed(const date& today) {
  for(auto& t : tasks) t.check_missed(today);
  recount();
}

int task_tracker::escalate_due_soon(const date& today, int threshold_days) {
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

void task_tracker::focus_plan(const date& today, int max_items) const {
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

double task_tracker::completion() const {
  return total ? done * 100.0 / total : 0;
}

void task_tracker::rank_level(int level) {
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

std::vector<task> task_tracker::prioritize() const {
  std::vector<task> sorted = tasks;
  std::stable_sort(sorted.begin(), sorted.end(), [](const task& a, const task& b){ return a.more_urgent(b); });
  return sorted;
}

std::ostream& operator<<(std::ostream& os, const task_tracker& tt) {
  os << "Tasks: " << tt.total << " total, " << tt.done << " done, " << tt.tbd << " to do, " << tt.missed << " missed\n";
  for(const auto& t : tt.tasks) os << "  " << t;
  return os;
}
