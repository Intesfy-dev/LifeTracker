#pragma once

#include <iostream>
#include <string>
#include "date.h"

class habit{

  private:
    std::string title, category_tag;
    int target_days_per_week, current_streak, best_streak;
    date last_completed;
    int prev_streak{0}, prev_best{0};
    date prev_last{};
    bool has_prev{false};

  public:
    explicit habit(const std::string& ti = "unspecified", const std::string& ct = "health", int td = 3, int cs = 0, int bs = 0, const date& lc = date{});

    const std::string& get_title() const { return title; }
    const std::string& get_category() const { return category_tag; }
    int get_target_days() const { return target_days_per_week; }
    int get_streak() const { return current_streak; }
    int get_best_streak() const { return best_streak; }
    const date& get_last_completed() const { return last_completed; }

    void check_in(const date& today);

    bool undo_check_in(const date& today);

    void verify_decay(const date& today);

    double consistency_score() const;

    friend std::istream& operator>>(std::istream& is, habit& h);

    friend std::ostream& operator<<(std::ostream& os, const habit& h);
};
