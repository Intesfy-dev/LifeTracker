#pragma once

#include <memory>
#include "reminder_rule.h"

// Avertizeaza pentru task-urile nefinalizate cu termenul in urmatoarele `days_ahead` zile sau deja depasit.
class deadline_alert : public reminder_rule {
  private:
    int days_ahead;

    std::vector<std::string> analyze(const lifetracker& app) const override;
    void print_details(std::ostream& os) const override;

  public:
    explicit deadline_alert(int days);

    std::unique_ptr<reminder_rule> clone() const override;
};
