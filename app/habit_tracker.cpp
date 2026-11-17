#include "habit_tracker.h"

#include <cstddef>

habit_tracker::habit_tracker(const std::vector<habit>& ha)
  : habits{ha} {}

void habit_tracker::add_habit(const habit& h) {
  habits.push_back(h);
}

bool habit_tracker::log_habit(const std::string& hab_title, const date& today) {
  for(auto& h : habits)
    if(h.get_title() == hab_title){
      h.check_in(today);
      return true;
    }
  return false;
}

bool habit_tracker::undo_log(const std::string& hab_title, const date& today) {
  for(auto& h : habits)
    if(h.get_title() == hab_title) return h.undo_check_in(today);
  return false;
}

bool habit_tracker::remove_habit(std::size_t idx) {
  if(idx >= habits.size()) return false;
  habits.erase(habits.begin() + static_cast<std::ptrdiff_t>(idx));
  return true;
}

void habit_tracker::update_streaks(const date& today) {
  for(auto& h : habits) h.verify_decay(today);
}

double habit_tracker::average_consistency() const {
  if(habits.empty()) return 0;
  double sum = 0;
  for(const auto& h : habits) sum += h.consistency_score();
  return sum / static_cast<double>(habits.size());
}

std::ostream& operator<<(std::ostream& os, const habit_tracker& ht) {
  os << "Habits (" << ht.habits.size() << " tracked, avg consistency " << ht.average_consistency() << "%):\n";
  for(const auto& h : ht.habits) os << "  " << h;
  return os;
}
