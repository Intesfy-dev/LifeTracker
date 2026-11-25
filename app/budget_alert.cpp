#include "budget_alert.h"

#include <numeric>
#include <sstream>
#include "lt_exceptions.h"

budget_alert::budget_alert(double min_left, double month_limit)
  : reminder_rule{"Alerta buget", 1}, min_balance{min_left}, monthly_limit{month_limit} {
  if(min_left < 0) throw invalid_value_error{"sold minim", min_left, "nu poate fi negativ"};
  if(month_limit <= 0) throw invalid_value_error{"limita lunara", month_limit, "trebuie sa fie pozitiva"};
}

std::unique_ptr<reminder_rule> budget_alert::clone() const {
  return std::make_unique<budget_alert>(*this);
}

void budget_alert::tighten(double factor) {
  if(factor < 1.0) throw invalid_value_error{"factor de crestere", factor, "trebuie sa fie >= 1"};
  min_balance *= factor;
}

std::vector<std::string> budget_alert::analyze(const lifetracker& app) const {
  std::vector<std::string> out;
  const budget& b = app.budget_ref();
  const date& today = app.get_date();

  if(b.get_left() < min_balance){
    std::ostringstream os;
    os << "soldul (" << b.get_left() << " RON) e sub pragul minim de " << min_balance << " RON";
    out.push_back(os.str());
  }

  const std::vector<transaction>& txs = b.get_transactions();
  const double spent = std::accumulate(txs.begin(), txs.end(), 0.0, [&today](double acc, const transaction& t){
    return (t.in_month(today) && t.signed_amount() < 0) ? acc - t.signed_amount() : acc;
  });
  if(spent > monthly_limit){
    std::ostringstream os;
    os << "ai cheltuit " << spent << " RON luna aceasta, peste limita de " << monthly_limit << " RON";
    out.push_back(os.str());
  }

  const double subs = b.subscriptions_cost();
  if(subs > 0 && b.get_left() < subs){
    std::ostringstream os;
    os << "abonamentele (" << subs << " RON) nu mai sunt acoperite de sold";
    out.push_back(os.str());
  }
  return out;
}

void budget_alert::print_details(std::ostream& os) const {
  os << "sold minim " << min_balance << " RON, limita lunara " << monthly_limit << " RON";
}
