#include "deadline_alert.h"

#include <sstream>
#include "lt_exceptions.h"

namespace {
  std::string days_text(int n) {
    return n == 1 ? std::string{"1 zi"} : std::to_string(n) + " zile";
  }
}

deadline_alert::deadline_alert(int days)
  : reminder_rule{"Alerta deadline", 2}, days_ahead{days} {
  if(days < 0 || days > 60) throw invalid_value_error{"zile inainte de termen", static_cast<double>(days), "trebuie sa fie intre 0 si 60"};
}

std::unique_ptr<reminder_rule> deadline_alert::clone() const {
  return std::make_unique<deadline_alert>(*this);
}

std::vector<std::string> deadline_alert::analyze(const lifetracker& app) const {
  std::vector<std::string> out;
  const date& today = app.get_date();
  for(const task& t : app.tasks_ref().prioritize()){
    if(t.is_done()) continue;
    const int left = today.days_until(t.get_deadline());
    std::ostringstream os;
    if(left < 0)
      os << "\"" << t.get_title() << "\" a depasit termenul cu " << days_text(-left) << " (" << t.get_progress() << "% gata)";
    else if(left <= days_ahead)
      os << "\"" << t.get_title() << "\" e scadent " << (left == 0 ? std::string{"azi"} : "in " + days_text(left)) << " (" << t.get_progress() << "% gata)";
    else
      continue;
    out.push_back(os.str());
  }
  return out;
}

void deadline_alert::print_details(std::ostream& os) const {
  os << "task-uri scadente in urmatoarele " << days_ahead << " zile";
}
