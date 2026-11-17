#pragma once

#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include "date.h"
#include "habit.h"

class habit_tracker{

  private:
    std::vector<habit> habits;

  public:
    explicit habit_tracker(const std::vector<habit>& ha = {});

    const std::vector<habit>& get_habits() const { return habits; }

    void add_habit(const habit& h);

    bool log_habit(const std::string& hab_title, const date& today);

    bool undo_log(const std::string& hab_title, const date& today);

    bool remove_habit(std::size_t idx);

    void update_streaks(const date& today);

    double average_consistency() const;

    friend std::ostream& operator<<(std::ostream& os, const habit_tracker& ht);
};
