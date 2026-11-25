#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "lifetracker.h"
#include "reminder_rule.h"

// Centrul de alerte: tine regulile prin pointeri la clasa de baza (std::unique_ptr<reminder_rule>)
// si le ruleaza polimorfic pe datele utilizatorului.
class reminder_center {
  private:
    std::vector<std::unique_ptr<reminder_rule>> rules;

  public:
    reminder_center() = default;
    reminder_center(const reminder_center& o);             // copiere profunda prin clone()
    reminder_center(reminder_center&& o) noexcept = default;
    reminder_center& operator=(reminder_center o);         // copy and swap
    ~reminder_center() = default;

    friend void swap(reminder_center& a, reminder_center& b) noexcept;

    // setul implicit de reguli pentru un cont nou
    static reminder_center with_defaults();

    void add_rule(const reminder_rule& r);

    std::size_t size() const { return rules.size(); }

    // ruleaza toate regulile (apel virtual prin pointer de baza) si intoarce mesajele ordonate dupa prioritate
    std::vector<std::string> run_all(const lifetracker& app) const;

    // daca prognoza bugetului e negativa, regulile de buget devin mai stricte;
    // dynamic_cast gaseste doar regulile de tip budget_alert; intoarce cate reguli au fost modificate
    int adapt_to_forecast(const lifetracker& app, int months_ahead);

    friend std::ostream& operator<<(std::ostream& os, const reminder_center& rc);
};
