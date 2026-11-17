#pragma once

#include <iostream>
#include <cstddef>
#include <vector>
#include "date.h"
#include "workout_session.h"

class fitness_tracker{

  private:
    int weekly_calorie_goal;
    std::vector<workout_session> sessions;

  public:
    explicit fitness_tracker(int wcg = 2500, const std::vector<workout_session>& se = {});

    int get_weekly_goal() const { return weekly_calorie_goal; }
    const std::vector<workout_session>& get_sessions() const { return sessions; }

    void add_session(const workout_session& ws);

    bool remove_session(std::size_t idx);

    int weekly_calories(const date& today) const;

    double weekly_load(const date& today) const;

    double goal_completion(const date& today) const;

    void sport_breakdown() const;

    friend std::ostream& operator<<(std::ostream& os, const fitness_tracker& ft);
};
