#include "calendar.h"

#include <cstddef>
#include <fstream>

bool calendar::has_conflict(const calendar_event& ev) const {
  for(const auto& x : events)
    if(x.overlaps(ev)) return true;
  return false;
}

calendar::calendar(const std::string& acc, const std::vector<calendar_event>& ev)
  : google_account{acc}, events{ev} {}

bool calendar::add_event(const calendar_event& e) {
  bool clash = has_conflict(e);
  if(clash) std::cout << "Calendar conflict detected for: " << e.get_title() << "\n";
  events.push_back(e);
  return !clash;
}

bool calendar::remove_event(std::size_t idx) {
  if(idx >= events.size()) return false;
  events.erase(events.begin() + static_cast<std::ptrdiff_t>(idx));
  return true;
}

bool calendar::modify_event(const std::string& ev_title, const date& new_date, int new_hour) {
  for(auto& e : events)
    if(e.get_title() == ev_title){
      e.reschedule(new_date, new_hour);
      return true;
    }
  return false;
}

int calendar::detect_conflicts() const {
  int count = 0;
  for(std::size_t i = 0; i < events.size(); i++)
    for(std::size_t j = i + 1; j < events.size(); j++)
      if(events[i].overlaps(events[j])){
        std::cout << "  Conflict: \"" << events[i].get_title() << "\" overlaps with \"" << events[j].get_title() << "\" on " << events[i].get_date() << "\n";
        count++;
      }
  return count;
}

int calendar::auto_resolve_conflicts() {
  int fixed = 0;
  for(std::size_t i = 0; i < events.size(); i++){
    for(std::size_t j = i + 1; j < events.size(); j++){
      if(!events[i].overlaps(events[j])) continue;
      date d = events[j].get_date();
      int dur = events[j].get_duration();
      for(int h = 8; h <= 21; h++){
        calendar_event probe{events[j].get_title(), d, h, dur, false};
        if(!has_conflict(probe)){
          events[j].reschedule(d, h);
          fixed++;
          break;
        }
      }
    }
  }
  return fixed;
}

int calendar::find_free_slot(const date& d, int slot_minutes) const {
  for(int h = 8; h <= 21; h++){
    calendar_event candidate{"slot", d, h, slot_minutes, false};
    if(!has_conflict(candidate)) return h;
  }
  return -1;
}

void calendar::week_agenda(const date& today) const {
  std::cout << "Week agenda (" << google_account << "):\n";
  for(const auto& e : events){
    int d = today.days_until(e.get_date());
    if(d >= 0 && d < 7) std::cout << "  " << e;
  }
}

int calendar::sync_google(const std::string& ics_path) {
  std::ofstream out{ics_path};
  out << "BEGIN:VCALENDAR\nVERSION:2.0\nX-WR-CALNAME:" << google_account << "\n";
  int pushed = 0;
  for(auto& e : events){
    if(!e.is_synced()){
      e.mark_synced();
      pushed++;
    }
    e.write_ics(out);
  }
  out << "END:VCALENDAR\n";
  std::cout << "Google Calendar sync (" << google_account << "): pushed " << pushed << " updates to " << ics_path << "\n";
  return pushed;
}

std::ostream& operator<<(std::ostream& os, const calendar& c) {
  os << "Calendar (" << c.google_account << ", " << c.events.size() << " events):\n";
  for(const auto& e : c.events) os << "  " << e;
  return os;
}
