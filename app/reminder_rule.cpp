#include "reminder_rule.h"

#include <algorithm>
#include <cstddef>
#include "lt_exceptions.h"

int reminder_rule::live_count = 0;

reminder_rule::reminder_rule(const std::string& rule_name, int rule_priority)
  : name{rule_name}, priority{rule_priority} {
  if(rule_name.empty()) throw invalid_value_error{"nume regula", 0, "numele nu poate fi gol"};
  if(rule_priority < 1 || rule_priority > 3) throw invalid_value_error{"prioritate", static_cast<double>(rule_priority), "trebuie sa fie intre 1 si 3"};
  live_count++;
}

reminder_rule::reminder_rule(const reminder_rule& o)
  : name{o.name}, priority{o.priority} {
  live_count++;
}

reminder_rule::~reminder_rule() {
  live_count--;
}

std::vector<std::string> reminder_rule::evaluate(const lifetracker& app) const {
  static const std::vector<std::string> labels = {"CRITIC", "IMPORTANT", "INFO"};
  std::vector<std::string> out = analyze(app);
  const std::string prefix = "[" + labels[static_cast<std::size_t>(priority - 1)] + "] " + name + ": ";
  std::transform(out.begin(), out.end(), out.begin(), [&prefix](const std::string& msg){ return prefix + msg; });
  return out;
}

int reminder_rule::live_rules() {
  return live_count;
}

std::ostream& operator<<(std::ostream& os, const reminder_rule& r) {
  os << r.name << " (prioritate " << r.priority << ") - ";
  r.print_details(os);
  return os << "\n";
}
