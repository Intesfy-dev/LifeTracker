#pragma once

#include <iostream>
#include <string>
#include "date.h"

class money_goal{

  private:
    std::string title;
    double money;
    date goal_deadline;

  public:
    explicit money_goal(const std::string& ti = "unspecified", double m = 0, const date& g = date{});

    const std::string& get_title() const { return title; }
    double get_money() const { return money; }
    const date& get_deadline() const { return goal_deadline; }
    int days_left(const date& today) const;

    double daily_target(double current_saved, const date& today) const;

    void extend_deadline(int extra_days);

    friend std::istream& operator>>(std::istream& is, money_goal& g);

    friend std::ostream& operator<<(std::ostream& os, const money_goal& g);
};
