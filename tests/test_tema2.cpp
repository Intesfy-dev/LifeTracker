// Teste pentru Tema 2: ierarhia reminder_rule + reminder_center si ierarhia de exceptii.
// Fara framework: fiecare verificare esuata e afisata, iar programul intoarce 1 daca a esuat ceva.
#include <iostream>
#include <string>
#include <vector>

#include "lifetracker.h"
#include "lt_exceptions.h"
#include "reminder_center.h"
#include "budget_alert.h"
#include "deadline_alert.h"
#include "habit_alert.h"

namespace {
  int failures = 0;

  void check(bool ok, const std::string& what) {
    if(!ok){
      std::cerr << "FAIL: " << what << "\n";
      failures++;
    }
  }

  bool contains(const std::vector<std::string>& msgs, const std::string& part) {
    for(const auto& m : msgs)
      if(m.find(part) != std::string::npos) return true;
    return false;
  }

  // cont cu sold negativ, un task aproape de termen si un obicei pe ultima zi de streak
  lifetracker sample_app() {
    const date today{20, 11, 2026};
    budget b{100, {transaction{"Chirie", category::housing, date{2, 11, 2026}, 800}}};
    task_tracker tt{{task{"Proiect POO", "Tema 2", 1, date{22, 11, 2026}, 30}, task{"Lab SD", "L4", 2, date{20, 12, 2026}, 0}}};
    habit_tracker ht{{habit{"Alergat", "health", 7, 4, 6, date{18, 11, 2026}}}};
    return lifetracker{"test", "pass", today, b, tt, calendar{}, roadmap{}, ht, fitness_tracker{}};
  }

  void test_rules() {
    const lifetracker app = sample_app();
    const reminder_center rc = reminder_center::with_defaults();
    const std::vector<std::string> msgs = rc.run_all(app);

    check(rc.size() == 3, "setul implicit are 3 reguli");
    check(contains(msgs, "sub pragul minim"), "alerta de sold minim");
    check(contains(msgs, "Proiect POO"), "alerta pentru task-ul scadent in 2 zile");
    check(!contains(msgs, "Lab SD"), "fara alerta pentru task-ul cu termen peste o luna");
    check(contains(msgs, "Alergat"), "alerta pentru streak-ul in pericol");
    check(!msgs.empty() && msgs.front().rfind("[CRITIC]", 0) == 0, "mesajele sunt ordonate dupa prioritate");
  }

  void test_copy_and_dynamic_cast() {
    const lifetracker app = sample_app();
    reminder_center original = reminder_center::with_defaults();
    const int before = reminder_rule::live_rules();
    {
      reminder_center copy{original};
      check(reminder_rule::live_rules() == before + 3, "constructorul de copiere cloneaza regulile");
      copy.add_rule(deadline_alert{10});
      check(copy.size() == 4 && original.size() == 3, "copia e independenta de original");
      original = copy;
      check(original.size() == 4, "operator= copiaza toate regulile");
    }
    check(reminder_rule::live_rules() == before + 1, "regulile copiei sunt eliberate la distrugere");
    check(original.adapt_to_forecast(app, 3) == 1, "dynamic_cast gaseste o singura regula de buget");
  }

  void test_exceptions() {
    bool caught = false;
    try {
      const budget_alert bad{-5, 100};
      check(false, "budget_alert cu sold minim negativ trebuie sa arunce");
    } catch(const invalid_value_error&) {
      caught = true;
    }
    check(caught, "invalid_value_error pentru sold minim negativ");

    caught = false;
    try {
      throw data_file_error{"data/lipsa.csv", "nu exista"};
    } catch(const lifetracker_error& e) {
      caught = std::string{e.what()}.find("data/lipsa.csv") != std::string::npos;
    }
    check(caught, "data_file_error e prinsa prin clasa de baza");

    caught = false;
    try {
      throw auth_error{"intrus"};
    } catch(const std::exception& e) {
      caught = std::string{e.what()}.find("intrus") != std::string::npos;
    }
    check(caught, "auth_error e prinsa ca std::exception");

    const int before = reminder_rule::live_rules();
    caught = false;
    try {
      const deadline_alert bad{100};
    } catch(const invalid_value_error&) {
      caught = true;
    }
    check(caught && reminder_rule::live_rules() == before, "o regula respinsa nu ramane numarata");
  }
}

int main() {
  test_rules();
  test_copy_and_dynamic_cast();
  test_exceptions();
  if(failures == 0) std::cout << "Toate testele Tema 2 au trecut\n";
  return failures == 0 ? 0 : 1;
}
