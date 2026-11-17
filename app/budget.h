#pragma once

#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include "date.h"
#include "transaction.h"
#include "money_goal.h"

class budget{

  private:
    double total, left, lost;
    std::vector<transaction> transactions;
    std::vector<money_goal> goals;

    void update(const transaction& t);

    void report_range(const std::string& label, const date& d, int mode) const;

  public:
    explicit budget(double t = 0, const std::vector<transaction>& tr = {}, const std::vector<money_goal>& go = {});

    budget(const budget& o);

    budget& operator=(const budget& o);

    ~budget();

    double get_total() const { return total; }
    double get_left() const { return left; }
    double get_lost() const { return lost; }
    const std::vector<transaction>& get_transactions() const { return transactions; }
    const std::vector<money_goal>& get_goals() const { return goals; }

    void add_transaction(const transaction& t);

    void add_goal(const money_goal& g);

    bool remove_transaction(std::size_t idx);

    bool remove_goal(std::size_t idx);

    double subscriptions_cost() const;

    void subscriptions_status(const date& today) const;

    void monthly_report(const date& d) const;

    void period_report(const date& d) const;

    void category_report(const date& d) const;

    void spending_split(const date& d) const;

    double forecast_balance(int months_ahead, const date& d) const;

    void goals_status(const date& today) const;

    friend std::ostream& operator<<(std::ostream& os, const budget& b);
};
