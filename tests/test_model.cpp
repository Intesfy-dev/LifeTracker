// Teste pentru logica de domeniu (fara UI, fara citire de la tastatura).
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "lifetracker.h"

namespace {

int failures = 0;

void check(bool cond, const std::string& what) {
  if (!cond) {
    ++failures;
    std::cerr << "FAIL: " << what << "\n";
  }
}

bool all_done(const roadmap& r) {
  for (const auto& n : r.get_nodes())
    if (!n.is_done()) return false;
  return true;
}

void test_roadmap() {
  roadmap r;
  r.add_root_tree("A", 2);
  r.split_problem("A", "a1", 1);
  r.split_problem("A", "a2", 1);
  r.split_problem("a1", "a1x", 1);
  check(r.overall_progress() == 0, "roadmap porneste de la 0%");

  check(r.complete_step("a1x"), "complete a1x");
  check(r.complete_step("a2"), "complete a2");
  check(all_done(r), "parintii se finalizeaza cand toti copiii sunt gata");

  check(r.reopen_step("a2"), "redeschide a2");
  check(r.overall_progress() < 99.9, "progresul scade dupa redeschidere");
  for (const auto& n : r.get_nodes()) {
    if (n.get_title() == "a2" || n.get_title() == "A") check(!n.is_done(), "a2 si radacina devin nefinalizate");
    if (n.get_title() == "a1x" || n.get_title() == "a1") check(n.is_done(), "a1/a1x raman finalizate");
  }

  check(r.complete_step("A"), "finalizarea unui parinte finalizeaza sub-arborele");
  check(all_done(r), "tot arborele finalizat");
  check(r.reopen_step("a1"), "redeschide a1 (cu sub-nodul)");
  for (const auto& n : r.get_nodes()) {
    if (n.get_title() == "a2") check(n.is_done(), "a2 ramane finalizat");
    else check(!n.is_done(), "restul devin nefinalizate");
  }

  int id = 0;
  for (const auto& n : r.get_nodes())
    if (n.get_title() == "a1") id = n.get_id();
  check(r.subtree_count(id) == 2, "sub-arborele lui a1 are 2 noduri");
  check(r.remove_node(id) == 2, "stergerea scoate nodul si sub-nodul");
  check(r.get_nodes().size() == 2, "raman 2 noduri");
  check(all_done(r), "dupa stergere, parintele se finalizeaza daca restul e gata");
  check(r.remove_node(9999) == 0, "id inexistent nu sterge nimic");
}

void test_tasks() {
  date d{10, 11, 2026};
  task_tracker tt;
  tt.add_task(task{"t", "x", 2, d, 40});
  check(tt.complete("t"), "complete task");
  check(tt.get_tasks()[0].get_progress() == 100, "progres 100 dupa finalizare");
  check(tt.reopen("t", d), "redeschide task");
  check(!tt.get_tasks()[0].is_done() && tt.get_tasks()[0].get_progress() == 40, "progresul anterior se restaureaza");
  check(tt.get_done() == 0 && tt.get_tbd() == 1, "contoarele se recalculeaza");
  check(!tt.reopen("t", d), "nu poti redeschide un task nefinalizat");
  check(tt.remove_task("t", d), "sterge task");
  check(tt.get_total() == 0, "lista goala dupa stergere");
}

void test_habits_and_fitness() {
  date d{10, 11, 2026};
  habit_tracker ht;
  ht.add_habit(habit{"h", "c", 5, 3, 3, d.yesterday()});
  ht.log_habit("h", d);
  check(ht.get_habits()[0].get_streak() == 4, "check-in creste streak-ul");
  check(ht.undo_log("h", d), "anuleaza check-in");
  check(ht.get_habits()[0].get_streak() == 3 && ht.get_habits()[0].get_best_streak() == 3, "streak restaurat");
  check(!ht.undo_log("h", d), "nu se poate anula de doua ori");

  habit reloaded{"g", "c", 5, 4, 4, d};
  check(reloaded.undo_check_in(d), "anulare dupa reincarcare");
  check(reloaded.get_streak() == 3 && reloaded.get_best_streak() == 3, "streak reconstruit");
  check(ht.remove_habit(0) && ht.get_habits().empty(), "sterge obicei");

  fitness_tracker ft;
  ft.add_session(workout_session{"w", "s", 30, 200, 5, d});
  ft.add_session(workout_session{"w2", "s", 30, 100, 5, d});
  check(ft.remove_session(0), "sterge antrenament");
  check(ft.get_sessions().size() == 1 && ft.get_sessions()[0].get_title() == "w2", "ramane antrenamentul corect");
  check(!ft.remove_session(5), "index invalid");
}

void test_budget_and_calendar() {
  date d{10, 11, 2026};
  budget b{1000};
  b.add_transaction(transaction{"salary", category::salary, d, 500, false, false});
  b.add_transaction(transaction{"food", category::food, d, 120, false, false});
  double before = b.get_left();
  check(b.remove_transaction(1), "sterge cheltuiala");
  check(b.get_left() == before + 120, "soldul se reface dupa stergerea cheltuielii");
  check(b.remove_transaction(0), "sterge venit");
  check(b.get_left() == 1000 && b.get_total() == 1000 && b.get_lost() == 0, "soldul revine la valoarea initiala");
  b.add_goal(money_goal{"g", 100, d});
  check(b.remove_goal(0) && b.get_goals().empty(), "sterge obiectiv");

  calendar c;
  c.add_event(calendar_event{"e", d, 9, 60, false});
  check(c.remove_event(0) && c.get_events().empty(), "sterge eveniment");
  check(!c.remove_event(0), "index invalid");
}

} // namespace

int main() {
  // logica claselor mai scrie informatii pe cout; le ascundem ca sa ramana doar rezultatul testelor
  std::ostringstream sink;
  std::streambuf* old = std::cout.rdbuf(sink.rdbuf());
  test_roadmap();
  test_tasks();
  test_habits_and_fitness();
  test_budget_and_calendar();
  std::cout.rdbuf(old);

  if (failures != 0) {
    std::cerr << failures << " test(e) esuate\n";
    return EXIT_FAILURE;
  }
  std::cout << "Toate testele au trecut\n";
  return EXIT_SUCCESS;
}
