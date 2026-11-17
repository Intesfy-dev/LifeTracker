#include "task.h"

#include <algorithm>

int task::days_left(const date& today) const {
  return today.days_until(task_deadline);
}

task::task(const std::string& ti, const std::string& desc, int u, const date& t_d, double p, bool d, bool m)
  : title{ti}, description{desc}, urgency{std::clamp(u, 1, max_level)}, task_deadline{t_d},
    progress{p}, done{d}, missed{m} {}

bool task::has_level(int l) const {
  return urgency == l && !done;
}

int task::deadline_diff(const task& o) const {
  return task_deadline.days_until(o.task_deadline);
}

double task::urgency_score(const date& today) const {
  if(done) return 0;
  int d = std::max(1, days_left(today));
  double base = static_cast<double>(max_level + 1 - urgency) * 100.0;
  double remaining = (100.0 - progress) / 100.0;
  return (base * remaining) / static_cast<double>(d);
}

void task::finish() {
  if(!done) prev_progress = progress;
  done = true;
  missed = false;
  progress = 100;
}

void task::reopen() {
  if(!done) return;
  done = false;
  progress = prev_progress < 100 ? prev_progress : 0;
}

void task::set_progress(double p) {
  progress = std::clamp(p, 0.0, 100.0);
  if(progress >= 100) finish();
}

void task::postpone(int extra_days) {
  if(extra_days > 0){
    task_deadline = task_deadline.add_days(extra_days);
    missed = false;
  }
}

void task::escalate() {
  if(urgency > 1) urgency--;
}

void task::check_missed(const date& today) {
  if(!done && days_left(today) < 0) missed = true;
}

bool task::more_urgent(const task& o) const {
  if(done != o.done) return !done;
  return urgency < o.urgency;
}

std::istream& operator>>(std::istream& is, task& t) {
  char sep = ',';
  double p = 0;
  std::getline(is, t.title, ',');
  std::getline(is, t.description, ',');
  if(is >> t.urgency >> sep >> t.task_deadline >> sep >> p){
    t.urgency = std::clamp(t.urgency, 1, task::max_level);
    t.done = t.missed = false;
    t.set_progress(p);
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const task& t) {
  os << t.title << " (" << t.description << ") | level " << t.urgency << " | due " << t.task_deadline << " | " << t.progress << "% | " << (t.done ? "done" : t.missed ? "missed" : "to do");
  return os << "\n";
}
