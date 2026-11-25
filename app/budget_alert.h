#pragma once

#include <memory>
#include "reminder_rule.h"

// Avertizeaza cand soldul scade sub un prag minim sau cand cheltuielile lunii depasesc o limita.
class budget_alert : public reminder_rule {
  private:
    double min_balance;
    double monthly_limit;

    std::vector<std::string> analyze(const lifetracker& app) const override;
    void print_details(std::ostream& os) const override;

  public:
    budget_alert(double min_left, double month_limit);

    std::unique_ptr<reminder_rule> clone() const override;

    // creste pragul minim de sold (ex: factor 1.5 = +50%), folosit cand prognoza bugetului e negativa
    void tighten(double factor);
};
