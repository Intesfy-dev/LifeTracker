#include "transaction.h"

#include <algorithm>
#include <cstddef>

bool transaction::is_income() const {
  return cat == category::salary || cat == category::bonus;
}

std::vector<std::string> transaction::names() {
  return {"food", "transportation", "shopping", "bills", "housing", "health", "entertainment", "education", "other", "salary", "bonus"};
}

transaction::transaction(const std::string& ti, category c, const date& z, double am, bool mem, bool w)
  : title{ti}, cat{c}, date_transaction{z}, amount{am}, membership{mem}, win{w} {}

std::string transaction::category_name() const {
  return names()[static_cast<std::size_t>(cat)];
}

double transaction::signed_amount() const {
  return is_income() ? amount : -amount;
}

bool transaction::is_essential() const {
  return cat == category::food || cat == category::transportation || cat == category::bills || cat == category::housing || cat == category::health || cat == category::education;
}

bool transaction::in_day(const date& d) const {
  return date_transaction.same_day(d);
}

bool transaction::in_week(const date& d) const {
  return date_transaction.same_week(d);
}

bool transaction::in_month(const date& d) const {
  return date_transaction.same_month(d);
}

bool transaction::in_year(const date& d) const {
  return date_transaction.same_year(d);
}

date transaction::next_billing_date(const date& today) const {
  return date_transaction.next_billing(today);
}

void transaction::apply_discount(double percent) {
  double p = std::clamp(percent, 0.0, 100.0);
  amount = amount * (1.0 - p / 100.0);
}

std::istream& operator>>(std::istream& is, transaction& t) {
  std::string cat_name;
  char sep = ',';
  std::getline(is, t.title, ',');
  std::getline(is, cat_name, ',');
  is >> t.date_transaction >> sep >> t.amount >> sep >> t.membership >> sep >> t.win;
  const std::vector<std::string> all = transaction::names();
  auto it = std::find(all.begin(), all.end(), cat_name);
  t.cat = it == all.end() ? category::other : static_cast<category>(it - all.begin());
  return is;
}

std::ostream& operator<<(std::ostream& os, const transaction& t) {
  os << t.title << " | " << t.category_name() << " | " << t.date_transaction << " | " << t.signed_amount();
  if(t.membership) os << " | subscription";
  if(t.win) os << " | win";
  return os << "\n";
}
