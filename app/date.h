#pragma once

#include <iostream>
#include <string>

class date{

  private:
    int day, month, year;

    bool leap() const;

    int days_in_month() const;

    int to_days() const;

  public:
    explicit date(int d = 1, int m = 1, int y = 2026);

    int get_day() const { return day; }
    int get_month() const { return month; }
    int get_year() const { return year; }

    int days_until(const date& o) const;

    date tomorrow() const;

    date yesterday() const;

    date add_days(int n) const;

    int day_of_week() const;

    std::string weekday_name() const;

    bool is_weekend() const;

    int quarter() const;

    date next_billing(const date& today) const;

    bool same_day(const date& o) const;
    bool same_week(const date& o) const;
    bool same_month(const date& o) const;
    bool same_year(const date& o) const;
    int ics_code() const;

    friend std::istream& operator>>(std::istream& is, date& d);

    friend std::ostream& operator<<(std::ostream& os, const date& d);
};
