#include "workout_session.h"

#include <algorithm>

workout_session::workout_session(const std::string& ti, const std::string& sp, int dur, int cal, int in_lvl, const date& sd)
  : title{ti}, sport{sp}, duration_min{std::max(5, dur)}, calories{std::max(0, cal)},
    intensity{std::clamp(in_lvl, 1, 10)}, session_date{sd} {}

double workout_session::training_load() const {
  return static_cast<double>(duration_min * intensity) / 10.0;
}

std::istream& operator>>(std::istream& is, workout_session& w) {
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

std::ostream& operator<<(std::ostream& os, const workout_session& w) {
  return os << w.title << " (" << w.sport << ") | " << w.session_date << " | " << w.duration_min << "m | " << w.calories << " kcal | intensity " << w.intensity << "/10 | load " << w.training_load() << "\n";
}
