#include "fitness_tracker.h"

#include <algorithm>
#include <map>
#include <string>

fitness_tracker::fitness_tracker(int wcg, const std::vector<workout_session>& se)
  : weekly_calorie_goal{std::max(500, wcg)}, sessions{se} {}

void fitness_tracker::add_session(const workout_session& ws) {
  sessions.push_back(ws);
}

bool fitness_tracker::remove_session(std::size_t idx) {
  if(idx >= sessions.size()) return false;
  sessions.erase(sessions.begin() + static_cast<std::ptrdiff_t>(idx));
  return true;
}

int fitness_tracker::weekly_calories(const date& today) const {
  int sum = 0;
  for(const auto& s : sessions)
    if(s.get_date().same_week(today)) sum += s.get_calories();
  return sum;
}

double fitness_tracker::weekly_load(const date& today) const {
  double load = 0;
  for(const auto& s : sessions)
    if(s.get_date().same_week(today)) load += s.training_load();
  return load;
}

double fitness_tracker::goal_completion(const date& today) const {
  return std::min(100.0, static_cast<double>(weekly_calories(today)) * 100.0 / static_cast<double>(weekly_calorie_goal));
}

void fitness_tracker::sport_breakdown() const {
  std::map<std::string, int> mins;
  for(const auto& s : sessions) mins[s.get_sport()] += s.get_duration();
  std::cout << "Training minutes by sport:\n";
  for(const auto& p : mins) std::cout << "  " << p.first << ": " << p.second << " min\n";
}

std::ostream& operator<<(std::ostream& os, const fitness_tracker& ft) {
  os << "Fitness (" << ft.sessions.size() << " sessions, weekly target " << ft.weekly_calorie_goal << " kcal):\n";
  for(const auto& s : ft.sessions) os << "  " << s;
  return os;
}
