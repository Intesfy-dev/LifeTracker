#include "date.h"

#include <algorithm>
#include <cstddef>
#include <vector>

bool date::leap() const {
  return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int date::days_in_month() const {
  static const std::vector<int> dm = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return (month == 2 && leap()) ? 29 : dm[static_cast<std::size_t>(month - 1)];
}

int date::to_days() const {
  int y = year - 1, days = y * 365 + y / 4 - y / 100 + y / 400;
  for(int m = 1; m < month; m++)
    days += date{1, m, year}.days_in_month();
  return days + day;
}

date::date(int d, int m, int y)
  : day{d}, month{m}, year{y} {
  if(month < 1 || month > 12) month = 1;
  if(day < 1 || day > days_in_month()) day = 1;
}

int date::days_until(const date& o) const {
  return o.to_days() - to_days();
}

date date::tomorrow() const {
  if(day < days_in_month()) return date{day + 1, month, year};
  if(month < 12) return date{1, month + 1, year};
  return date{1, 1, year + 1};
}

date date::yesterday() const {
  if(day > 1) return date{day - 1, month, year};
  if(month > 1){
    int pm = month - 1;
    return date{date{1, pm, year}.days_in_month(), pm, year};
  }
  return date{31, 12, year - 1};
}

date date::add_days(int n) const {
  date cur = *this;
  for(int i = 0; i < n; i++) cur = cur.tomorrow();
  for(int i = 0; i > n; i--) cur = cur.yesterday();
  return cur;
}

int date::day_of_week() const {
  int d = to_days() % 7;
  return d < 0 ? d + 7 : d;
}

std::string date::weekday_name() const {
  static const std::vector<std::string> wn = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  return wn[static_cast<std::size_t>(day_of_week())];
}

bool date::is_weekend() const {
  int dw = day_of_week();
  return dw == 0 || dw == 6;
}

int date::quarter() const {
  return (month - 1) / 3 + 1;
}

date date::next_billing(const date& today) const {
  int max_d = date{1, today.month, today.year}.days_in_month();
  date due{std::min(day, max_d), today.month, today.year};
  if(today.days_until(due) > 0) return due;
  int nm = today.month == 12 ? 1 : today.month + 1;
  int ny = today.month == 12 ? today.year + 1 : today.year;
  return date{std::min(day, date{1, nm, ny}.days_in_month()), nm, ny};
}

bool date::same_day(const date& o) const {
  return day == o.day && month == o.month && year == o.year;
}

bool date::same_week(const date& o) const {
  int diff = days_until(o); return diff >= -6 && diff <= 6;
}

bool date::same_month(const date& o) const {
  return month == o.month && year == o.year;
}

bool date::same_year(const date& o) const {
  return year == o.year;
}

int date::ics_code() const {
  return year * 10000 + month * 100 + day;
}

std::istream& operator>>(std::istream& is, date& d) {
  int dd = 1, mm = 1, yy = 2026;
  char sep = '.';
  if(is >> dd >> sep >> mm >> sep >> yy) d = date{dd, mm, yy};
  return is;
}

std::ostream& operator<<(std::ostream& os, const date& d) {
  return os << d.day << "." << d.month << "." << d.year;
}
