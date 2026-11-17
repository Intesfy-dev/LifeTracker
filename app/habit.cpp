#include "habit.h"

#include <algorithm>

habit::habit(const std::string& ti, const std::string& ct, int td, int cs, int bs, const date& lc)
  : title{ti}, category_tag{ct}, target_days_per_week{std::clamp(td, 1, 7)},
    current_streak{std::max(0, cs)}, best_streak{std::max(cs, bs)}, last_completed{lc} {}

void habit::check_in(const date& today) {
  int diff = last_completed.days_until(today);
  if(diff == 0) return;
  prev_streak = current_streak;
  prev_best = best_streak;
  prev_last = last_completed;
  has_prev = true;
  if(diff == 1 || diff <= 7 / target_days_per_week + 1) current_streak++;
  else current_streak = 1;
  best_streak = std::max(best_streak, current_streak);
  last_completed = today;
}

bool habit::undo_check_in(const date& today) {
  if(last_completed.days_until(today) != 0) return false;
  if(has_prev){
    current_streak = prev_streak;
    best_streak = prev_best;
    last_completed = prev_last;
    has_prev = false;
    return true;
  }
  // dupa o repornire a aplicatiei nu mai stim starea exacta dinainte: o reconstruim
  if(current_streak <= 0) return false;
  if(best_streak == current_streak) best_streak--;
  current_streak--;
  last_completed = current_streak > 0 ? today.yesterday() : today.add_days(-30);
  return true;
}

void habit::verify_decay(const date& today) {
  int max_gap = 7 / target_days_per_week + 2;
  if(last_completed.days_until(today) > max_gap) current_streak = 0;
}

double habit::consistency_score() const {
  if(best_streak <= 0) return 0;
  return std::min(100.0, static_cast<double>(current_streak) * 100.0 / static_cast<double>(best_streak));
}

std::istream& operator>>(std::istream& is, habit& h) {
  char sep = ',';
  std::getline(is, h.title, ',');
  std::getline(is, h.category_tag, ',');
  if(is >> h.target_days_per_week >> sep >> h.current_streak >> sep >> h.best_streak >> sep >> h.last_completed){
    h.target_days_per_week = std::clamp(h.target_days_per_week, 1, 7);
    h.best_streak = std::max(h.current_streak, h.best_streak);
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const habit& h) {
  return os << h.title << " [" << h.category_tag << "] | " << h.target_days_per_week << "x/week | streak " << h.current_streak << " (best " << h.best_streak << ") | last " << h.last_completed << " | consistency " << h.consistency_score() << "%\n";
}
