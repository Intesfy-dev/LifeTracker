#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "date.h"

enum class category{ food, transportation, shopping, bills, housing, health, entertainment, education, other, salary, bonus };

class transaction{

  private:
    std::string title;
    category cat;
    date date_transaction;
    double amount;
    bool membership;
    bool win;

    bool is_income() const;

    static std::vector<std::string> names();

  public:
    explicit transaction(const std::string& ti = "unspecified", category c = category::other, const date& z = date{}, double am = 0, bool mem = false, bool w = false);

    const std::string& get_title() const { return title; }
    const date& get_date() const { return date_transaction; }
    std::string category_name() const;

    double signed_amount() const;
    double raw_amount() const { return amount; }
    bool is_membership() const { return membership; }
    bool is_win() const { return win; }

    bool is_essential() const;

    bool in_day(const date& d) const;
    bool in_week(const date& d) const;
    bool in_month(const date& d) const;
    bool in_year(const date& d) const;
    date next_billing_date(const date& today) const;

    void apply_discount(double percent);

    friend std::istream& operator>>(std::istream& is, transaction& t);

    friend std::ostream& operator<<(std::ostream& os, const transaction& t);
};
