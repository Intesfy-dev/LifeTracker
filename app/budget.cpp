#include "budget.h"

#include <algorithm>
#include <map>

void budget::update(const transaction& t) {
  if(t.signed_amount() > 0) total += t.signed_amount();
  else lost -= t.signed_amount();
  left = total - lost;
}

void budget::report_range(const std::string& label, const date& d, int mode) const {
  double earned = 0, spent = 0, biggest = 0;
  for(const auto& t : transactions){
    bool match = (mode == 0 && t.in_day(d)) || (mode == 1 && t.in_week(d)) || (mode == 2 && t.in_month(d)) || (mode == 3 && t.in_year(d));
    if(!match) continue;
    double s = t.signed_amount();
    if(s > 0) earned += s;
    else{
      spent -= s;
      biggest = std::max(biggest, -s);
    }
  }
  std::cout << label << " report: earned " << earned << ", spent " << spent << ", saved " << earned - spent << ", biggest expense " << biggest << "\n";
}

budget::budget(double t, const std::vector<transaction>& tr, const std::vector<money_goal>& go)
  : total{t}, left{t}, lost{0}, transactions{tr}, goals{go} {
  for(const auto& x : transactions) update(x);
}

budget::budget(const budget& o)
  : total{o.total}, left{o.left}, lost{o.lost}, transactions{o.transactions}, goals{o.goals} {
  std::cout << "[budget copy constructor]\n";
}

budget& budget::operator=(const budget& o) {
  if(this != &o){
    total = o.total;
    left = o.left;
    lost = o.lost;
    transactions = o.transactions;
    goals = o.goals;
  }
  std::cout << "[budget operator=]\n";
  return *this;
}

budget::~budget() {
  std::cout << "[budget destructor]\n";
}

void budget::add_transaction(const transaction& t) {
  transactions.push_back(t);
  update(t);
}

void budget::add_goal(const money_goal& g) {
  goals.push_back(g);
}

bool budget::remove_transaction(std::size_t idx) {
  if(idx >= transactions.size()) return false;
  double s = transactions[idx].signed_amount();
  if(s > 0) total -= s;
  else lost += s;
  left = total - lost;
  transactions.erase(transactions.begin() + static_cast<std::ptrdiff_t>(idx));
  return true;
}

bool budget::remove_goal(std::size_t idx) {
  if(idx >= goals.size()) return false;
  goals.erase(goals.begin() + static_cast<std::ptrdiff_t>(idx));
  return true;
}

double budget::subscriptions_cost() const {
  double sum = 0;
  for(const auto& t : transactions)
    if(t.is_membership() && t.signed_amount() < 0) sum -= t.signed_amount();
  return sum;
}

void budget::subscriptions_status(const date& today) const {
  double reserve = 0;
  std::cout << "Subscriptions tracker:\n";
  for(const auto& t : transactions){
    if(!t.is_membership() || t.signed_amount() >= 0) continue;
    double cost = -t.signed_amount();
    date due = t.next_billing_date(today);
    int days = std::max(1, today.days_until(due));
    reserve += cost;
    std::cout << "  " << t.get_title() << ": " << cost << " due on " << due << " (" << days << " days left, set aside " << cost / static_cast<double>(days) << "/day)\n";
  }
  std::cout << "  Total to keep aside for subscriptions: " << reserve << " (available after reserve: " << left - reserve << ")\n";
}

void budget::monthly_report(const date& d) const {
  report_range("Month", d, 2);
}

void budget::period_report(const date& d) const {
  report_range("Day", d, 0);
  report_range("Week", d, 1);
  report_range("Month", d, 2);
  report_range("Year", d, 3);
}

void budget::category_report(const date& d) const {
  std::map<std::string, double> sums;
  for(const auto& t : transactions)
    if(t.in_month(d) && t.signed_amount() < 0) sums[t.category_name()] -= t.signed_amount();
  std::cout << "Spending by category:\n";
  for(const auto& p : sums) std::cout << "  " << p.first << ": " << p.second << "\n";
}

void budget::spending_split(const date& d) const {
  double essential = 0, discretionary = 0, wins = 0;
  for(const auto& t : transactions){
    if(!t.in_month(d)) continue;
    if(t.is_win() && t.signed_amount() > 0) wins += t.signed_amount();
    if(t.signed_amount() >= 0) continue;
    if(t.is_essential()) essential -= t.signed_amount();
    else discretionary -= t.signed_amount();
  }
  double sum = essential + discretionary;
  double ess_pct = sum > 0 ? essential * 100.0 / sum : 0;
  std::cout << "Monthly split: essential " << essential << " (" << ess_pct << "%), discretionary " << discretionary << ", bonus wins " << wins << "\n";
}

double budget::forecast_balance(int months_ahead, const date& d) const {
  double earned = 0, spent = 0;
  for(const auto& t : transactions){
    if(!t.in_month(d)) continue;
    if(t.signed_amount() > 0) earned += t.signed_amount();
    else spent -= t.signed_amount();
  }
  double net_monthly = earned - spent;
  double projected = left + net_monthly * static_cast<double>(months_ahead);
  std::cout << "Forecast in " << months_ahead << " months (net " << net_monthly << "/month): " << projected << "\n";
  return projected;
}

void budget::goals_status(const date& today) const {
  for(const auto& g : goals){
    double target = g.get_money();
    int days = g.days_left(today);
    std::cout << "  " << g;
    if(target <= 0 || left >= target) std::cout << "    reached!\n";
    else if(days <= 0) std::cout << "    deadline passed, missing " << target - left << "\n";
    else std::cout << "    " << left / target * 100 << "% done, save " << (target - left) / (static_cast<double>(days) / 30.0) << " / month (" << g.daily_target(left, today) << " / day)\n";
  }
}

std::ostream& operator<<(std::ostream& os, const budget& b) {
  os << "Budget: total " << b.total << ", left " << b.left << ", lost " << b.lost << "\n";
  for(const auto& t : b.transactions) os << "  " << t;
  for(const auto& g : b.goals) os << "  " << g;
  return os;
}
