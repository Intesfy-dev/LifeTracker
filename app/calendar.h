#pragma once

#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include "date.h"
#include "calendar_event.h"

class calendar{

  private:
    std::string google_account;
    std::vector<calendar_event> events;

    bool has_conflict(const calendar_event& ev) const;

  public:
    explicit calendar(const std::string& acc = "user@gmail.com", const std::vector<calendar_event>& ev = {});

    const std::vector<calendar_event>& get_events() const { return events; }

    bool add_event(const calendar_event& e);

    bool remove_event(std::size_t idx);

    bool modify_event(const std::string& ev_title, const date& new_date, int new_hour);

    int detect_conflicts() const;

    int auto_resolve_conflicts();

    int find_free_slot(const date& d, int slot_minutes) const;

    void week_agenda(const date& today) const;

    int sync_google(const std::string& ics_path);

    friend std::ostream& operator<<(std::ostream& os, const calendar& c);
};
