#include "money_goal.h"

money_goal::money_goal(const std::string& ti, double m, const date& g)
  : title{ti}, money{m}, goal_deadline{g} {}

int money_goal::days_left(const date& today) const {
  return today.days_until(goal_deadline);
}

double money_goal::daily_target(double current_saved, const date& today) const {
  int d = days_left(today);
  if(d <= 0 || current_saved >= money) return 0;
  return (money - current_saved) / static_cast<double>(d);
}

void money_goal::extend_deadline(int extra_days) {
  if(extra_days > 0) goal_deadline = goal_deadline.add_days(extra_days);
}

std::istream& operator>>(std::istream& is, money_goal& g) {
  char sep = ',';
  std::getline(is, g.title, ',');
  return is >> g.money >> sep >> g.goal_deadline;
}

std::ostream& operator<<(std::ostream& os, const money_goal& g) {
  return os << g.title << " | target " << g.money << " | deadline " << g.goal_deadline << "\n";
}
