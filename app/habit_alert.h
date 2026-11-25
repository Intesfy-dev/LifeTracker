#pragma once

#include <memory>
#include "reminder_rule.h"

// Avertizeaza cand un obicei e pe cale sa-si piarda streak-ul sau cand consistenta scade sub un minim.
class habit_alert : public reminder_rule {
  private:
    double min_consistency;  // procent, 0..100

    std::vector<std::string> analyze(const lifetracker& app) const override;
    void print_details(std::ostream& os) const override;

  public:
    explicit habit_alert(double min_percent);

    std::unique_ptr<reminder_rule> clone() const override;
};
