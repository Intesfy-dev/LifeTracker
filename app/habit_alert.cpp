#include "habit_alert.h"

#include <sstream>
#include "lt_exceptions.h"

habit_alert::habit_alert(double min_percent)
  : reminder_rule{"Alerta obiceiuri", 3}, min_consistency{min_percent} {
  if(min_percent < 0 || min_percent > 100) throw invalid_value_error{"consistenta minima", min_percent, "trebuie sa fie intre 0 si 100"};
}

std::unique_ptr<reminder_rule> habit_alert::clone() const {
  return std::make_unique<habit_alert>(*this);
}

std::vector<std::string> habit_alert::analyze(const lifetracker& app) const {
  std::vector<std::string> out;
  const date& today = app.get_date();
  for(const habit& h : app.habits_ref().get_habits()){
    std::ostringstream os;
    // aceeasi regula ca habit::check_in: streak-ul continua doar daca pauza e de cel mult 7/target + 1 zile
    const int last_ok_gap = 7 / h.get_target_days() + 1;
    const int gap = h.get_last_completed().days_until(today);
    if(h.get_streak() > 0 && gap == last_ok_gap)
      os << "\"" << h.get_title() << "\": streak-ul de " << h.get_streak() << " se pierde daca nu faci check-in azi";
    else if((h.get_streak() > 0 && gap > last_ok_gap) || (h.get_streak() == 0 && h.get_best_streak() > 0))
      os << "\"" << h.get_title() << "\": streak pierdut (recordul tau e " << h.get_best_streak() << ")";
    else if(h.get_best_streak() > 0 && h.consistency_score() < min_consistency)
      os << "\"" << h.get_title() << "\": consistenta " << h.consistency_score() << "% e sub " << min_consistency << "%";
    else
      continue;
    out.push_back(os.str());
  }
  return out;
}

void habit_alert::print_details(std::ostream& os) const {
  os << "streak-uri in pericol si consistenta sub " << min_consistency << "%";
}
