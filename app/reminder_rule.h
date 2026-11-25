#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "lifetracker.h"

// Clasa de baza (abstracta) a ierarhiei de alerte.
// Fiecare regula analizeaza starea aplicatiei (buget, task-uri, obiceiuri...) si produce mesaje de avertizare.
//  - interfata non-virtuala: evaluate() si operator<< sunt publice si NE-virtuale; ele apeleaza
//    functiile virtuale private analyze() si print_details() (pattern NVI)
//  - clone() = "constructor virtual", folosit pentru copierea corecta prin pointer de baza
class reminder_rule {
  private:
    std::string name;
    int priority;  // 1 = critic, 2 = important, 3 = informativ

    static int live_count;  // cate reguli exista in acest moment (toate tipurile)

    // functia virtuala specifica temei: ce probleme vede regula in datele utilizatorului
    virtual std::vector<std::string> analyze(const lifetracker& app) const = 0;
    // afisare virtuala, apelata din operator<< (interfata non-virtuala)
    virtual void print_details(std::ostream& os) const = 0;

  protected:
    reminder_rule(const reminder_rule& o);
    reminder_rule& operator=(const reminder_rule& o) = default;

  public:
    reminder_rule(const std::string& rule_name, int rule_priority);
    virtual ~reminder_rule();

    virtual std::unique_ptr<reminder_rule> clone() const = 0;

    // interfata non-virtuala: adauga prefixul [prioritate] nume la fiecare mesaj produs de analyze()
    std::vector<std::string> evaluate(const lifetracker& app) const;

    int get_priority() const { return priority; }

    static int live_rules();

    friend std::ostream& operator<<(std::ostream& os, const reminder_rule& r);
};
