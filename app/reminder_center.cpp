#include "reminder_center.h"

#include <algorithm>
#include <set>
#include <utility>
#include "budget_alert.h"
#include "deadline_alert.h"
#include "habit_alert.h"

reminder_center::reminder_center(const reminder_center& o) {
  rules.reserve(o.rules.size());
  for(const auto& r : o.rules) rules.push_back(r->clone());
}

reminder_center& reminder_center::operator=(reminder_center o) {
  swap(*this, o);
  return *this;
}

void swap(reminder_center& a, reminder_center& b) noexcept {
  using std::swap;
  swap(a.rules, b.rules);
}

reminder_center reminder_center::with_defaults() {
  reminder_center rc;
  rc.add_rule(budget_alert{200, 2500});
  rc.add_rule(deadline_alert{3});
  rc.add_rule(habit_alert{50});
  return rc;
}

void reminder_center::add_rule(const reminder_rule& r) {
  rules.push_back(r.clone());
}

std::vector<std::string> reminder_center::run_all(const lifetracker& app) const {
  std::vector<std::pair<int, std::string>> found;
  for(const auto& r : rules)
    for(const std::string& msg : r->evaluate(app))
      found.emplace_back(r->get_priority(), msg);

  std::stable_sort(found.begin(), found.end(), [](const auto& a, const auto& b){ return a.first < b.first; });

  // doua reguli care se suprapun (ex: deadline la 3 si la 7 zile) nu repeta acelasi mesaj
  std::vector<std::string> out;
  std::set<std::string> seen;
  for(const auto& p : found)
    if(seen.insert(p.second).second) out.push_back(p.second);
  return out;
}

int reminder_center::adapt_to_forecast(const lifetracker& app, int months_ahead) {
  const budget& b = app.budget_ref();
  const date& today = app.get_date();
  double net = 0;
  for(const transaction& t : b.get_transactions())
    if(t.in_month(today)) net += t.signed_amount();
  const double projected = b.get_left() + net * months_ahead;
  if(projected >= 0) return 0;

  int changed = 0;
  for(auto& r : rules)
    if(auto* ba = dynamic_cast<budget_alert*>(r.get())){
      ba->tighten(1.5);
      changed++;
    }
  return changed;
}

std::ostream& operator<<(std::ostream& os, const reminder_center& rc) {
  os << "Centru de alerte (" << rc.rules.size() << " reguli):\n";
  for(const auto& r : rc.rules) os << "  " << *r;
  return os;
}
