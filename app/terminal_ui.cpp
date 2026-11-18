#include "terminal_ui.h"
#include "io_utils.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace ansi {
  const std::string bg_dark   = "\033[48;2;28;30;29m";
  const std::string bg_card   = "\033[48;2;35;37;36m";
  const std::string fg_white  = "\033[38;2;244;244;246m";
  const std::string fg_neon   = "\033[38;2;0;230;91m";
  const std::string fg_muted  = "\033[38;2;142;146;144m";
  const std::string bold      = "\033[1m";
  const std::string reset     = "\033[0m";
}

terminal_ui::terminal_ui(account_manager& m) : mgr{m} {}

namespace {
  // Cere un numar de ordine (1..n) pentru stergere; intoarce indexul 0-based sau -1 daca e anulat/invalid
  int ask_index(const std::string& what, std::size_t n) {
    if (n == 0) {
      std::cout << "Nu exista nimic de sters la " << what << ".\n";
      return -1;
    }
    std::cout << "Numarul de ordine (1-" << n << ", 0 = anuleaza) pentru " << what << ": ";
    int k = 0;
    std::cin >> k;
    std::cin.ignore(1000, '\n');
    if (k < 1 || k > static_cast<int>(n)) {
      std::cout << "Anulat.\n";
      return -1;
    }
    return k - 1;
  }
}

void terminal_ui::clear_screen() {
  std::cout << ansi::bg_dark << ansi::fg_white << "\033[2J\033[H";
}

void terminal_ui::print_banner(const std::string& subtitle) {
  clear_screen();
  std::cout << ansi::bg_card << ansi::fg_white << ansi::bold
            << "╭──────────────────────────────────────────────────────────────────────────────╮\n"
            << "│  " << ansi::fg_neon << "▍" << ansi::fg_white << "  ⏻  L I F E   T R A C K E R   O S   —   M A T T E   N E O N   U I        │\n"
            << "│  " << ansi::fg_neon << "▍" << ansi::fg_muted << "  " << std::left << std::setw(73) << subtitle << ansi::fg_white << "│\n"
            << "╰──────────────────────────────────────────────────────────────────────────────╯"
            << ansi::reset << ansi::bg_dark << ansi::fg_white << "\n\n";
}

void terminal_ui::pause_prompt() {
  std::cout << "\n" << ansi::fg_muted << "Apasa [ENTER] pentru a continua..." << ansi::reset << ansi::bg_dark << ansi::fg_white;
  std::string dummy;
  std::getline(std::cin, dummy);
}

int terminal_ui::run() {
  while (true) {
    print_banner("AUTENTIFICARE & CONTROL CONTURI (DARK NEUMORPHIC TUI)");
    std::cout << ansi::fg_neon << "  ▍ [1] " << ansi::fg_white << "Autentificare in cont existent (Login)\n"
              << ansi::fg_neon << "  ▍ [2] " << ansi::fg_white << "Creare cont nou (Sign Up)\n"
              << ansi::fg_neon << "  ▍ [3] " << ansi::fg_white << "Lista conturi inregistrate\n"
              << ansi::fg_neon << "  ▍ [0] " << ansi::fg_muted << "Iesire din aplicatie (OFF)\n\n"
              << ansi::fg_neon << "  ⏻ Alege optiunea: " << ansi::fg_white;

    int opt = -1;
    if (!(std::cin >> opt)) {
      if (std::cin.eof()) return 0;
      std::cin.clear();
      std::cin.ignore(1000, '\n');
      continue;
    }
    std::cin.ignore(1000, '\n');

    if (opt == 0) {
      std::cout << ansi::reset << "\nLa revedere!\n";
      return 0;
    } else if (opt == 1) {
      std::string u, p;
      std::cout << "\nUtilizator: ";
      std::getline(std::cin, u);
      std::cout << "Parola: ";
      std::getline(std::cin, p);

      account_record rec;
      if (mgr.authenticate(u, p, rec)) {
        lifetracker app = mgr.load_user_app(rec);
        run_dashboard(app, rec.initial_balance);
      } else {
        std::cout << "\n✕ Date de autentificare incorecte!\n";
        pause_prompt();
      }
    } else if (opt == 2) {
      std::string u, p, date_str;
      double bal = 0;
      char seed_ch = 'd';
      std::cout << "\nNume utilizator nou: ";
      std::getline(std::cin, u);
      std::cout << "Parola: ";
      std::getline(std::cin, p);
      std::cout << "Data de inceput (z.l.a, ex: 17.11.2026): ";
      std::getline(std::cin, date_str);
      std::cout << "Sold initial buget (RON): ";
      std::cin >> bal;
      std::cout << "Populam contul cu date demo initiale? [ON=d / OFF=n]: ";
      std::cin >> seed_ch;
      std::cin.ignore(1000, '\n');

      date d{17, 11, 2026};
      parse_line(date_str, d);
      bool seed = (seed_ch == 'd' || seed_ch == 'D' || seed_ch == 'y' || seed_ch == 'Y');

      if (mgr.register_account(u, p, d, bal, seed)) {
        std::cout << "\n✓ Contul '" << u << "' a fost activat (ON)! Te conectam automat...\n";
        pause_prompt();
        account_record rec{u, p, d, bal};
        lifetracker app = mgr.load_user_app(rec);
        run_dashboard(app, rec.initial_balance);
      } else {
        std::cout << "\n✕ Eroare: Contul exista deja sau datele sunt invalide!\n";
        pause_prompt();
      }
    } else if (opt == 3) {
      std::cout << "\nConturi disponibile:\n";
      for (const auto& a : mgr.list_accounts()) {
        std::cout << "  " << ansi::fg_neon << "▍ " << ansi::fg_white << a.username
                  << ansi::fg_muted << " (Data: " << a.start_date << ", Sold initial: " << a.initial_balance << " RON)\n" << ansi::fg_white;
      }
      pause_prompt();
    }
  }
}

void terminal_ui::run_dashboard(lifetracker& app, double initial_balance) {
  while (true) {
    std::ostringstream sub;
    sub << "CONT: " << app.get_user() << "  |  DATA: " << app.get_date()
        << " (" << app.get_date().weekday_name() << ")  |  LIFE SCORE: "
        << std::fixed << std::setprecision(0) << app.life_score() << "%";
    print_banner(sub.str());

    std::cout << ansi::bg_card
              << "  " << ansi::fg_neon << "▍" << ansi::fg_muted << " Buget: " << ansi::fg_neon << std::fixed << std::setprecision(0) << app.budget_ref().get_left() << " RON"
              << "   " << ansi::fg_neon << "▍" << ansi::fg_muted << " Tasks: " << ansi::fg_white << app.tasks_ref().completion() << "%"
              << "   " << ansi::fg_neon << "▍" << ansi::fg_muted << " Roadmap: " << ansi::fg_neon << app.roadmap_ref().overall_progress() << "%"
              << "   " << ansi::fg_neon << "▍" << ansi::fg_muted << " Habits: " << ansi::fg_white << app.habits_ref().average_consistency() << "%"
              << ansi::reset << ansi::bg_dark << ansi::fg_white << "\n\n";

    std::cout << ansi::fg_neon << "  ▍ [1] " << ansi::fg_white << "Raport Complet Dashboard (Summary)\n"
              << ansi::fg_neon << "  ▍ [2] " << ansi::fg_white << "Buget: Adauga Tranzactie / Obiectiv Financiar & Rapoarte\n"
              << ansi::fg_neon << "  ▍ [3] " << ansi::fg_white << "Tasks: Adauga / Finalizeaza / Comparatie Binara (Rank Level)\n"
              << ansi::fg_neon << "  ▍ [4] " << ansi::fg_white << "Calendar: Adauga Eveniment / Rezolva Conflicte / Sync Google (.ics)\n"
              << ansi::fg_neon << "  ▍ [5] " << ansi::fg_white << "Roadmap: Afiseaza Arbore / Split Sub-problema / Completeaza Pas\n"
              << ansi::fg_neon << "  ▍ [6] " << ansi::fg_white << "Habits & Fitness: Check-in Obicei / Adauga Antrenament\n"
              << ansi::fg_neon << "  ▍ [7] " << ansi::fg_white << "Avanseaza la Ziua Urmatoare (+1 Day)\n"
              << ansi::fg_neon << "  ▍ [0] " << ansi::fg_muted << "Salveaza & Deconectare (OFF)\n\n"
              << ansi::fg_neon << "  ⏻ Comanda: " << ansi::fg_white;

    int cmd = -1;
    if (!(std::cin >> cmd)) {
      if (std::cin.eof()) break;
      std::cin.clear();
      std::cin.ignore(1000, '\n');
      continue;
    }
    std::cin.ignore(1000, '\n');

    if (cmd == 0) {
      mgr.save_user_app(app, initial_balance);
      break;
    } else if (cmd == 1) {
      std::cout << "\n";
      app.summary();
      pause_prompt();
    } else if (cmd == 2) {
      std::cout << "\n[1] Adauga Tranzactie  [2] Adauga Obiectiv (Money Goal)  [3] Vezi Raport Buget  [4] Sterge Tranzactie  [5] Sterge Obiectiv: ";
      int bopt = 0;
      std::cin >> bopt;
      std::cin.ignore(1000, '\n');
      if (bopt == 1) {
        transaction tx;
        if (read_item("Format (titlu,categorie,z.l.a,suma,abonament 0/1,win 0/1): ", tx)) {
          app.add_transaction(tx);
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Tranzactie adaugata!\n";
        }
      } else if (bopt == 2) {
        money_goal mg;
        if (read_item("Format (titlu,suma_tinta,z.l.a): ", mg)) {
          app.budget_ref().add_goal(mg);
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Obiectiv financiar adaugat!\n";
        }
      } else if (bopt == 4 || bopt == 5) {
        if (bopt == 4) {
          const auto& txs = app.budget_ref().get_transactions();
          for (std::size_t i = 0; i < txs.size(); ++i) std::cout << "  " << i + 1 << ". " << txs[i];
          int k = ask_index("tranzactie", txs.size());
          if (k >= 0 && app.budget_ref().remove_transaction(static_cast<std::size_t>(k))) {
            mgr.save_user_app(app, initial_balance);
            std::cout << "✓ Tranzactie stearsa, soldul a fost recalculat.\n";
          }
        } else {
          const auto& gs = app.budget_ref().get_goals();
          for (std::size_t i = 0; i < gs.size(); ++i) std::cout << "  " << i + 1 << ". " << gs[i];
          int k = ask_index("obiectiv", gs.size());
          if (k >= 0 && app.budget_ref().remove_goal(static_cast<std::size_t>(k))) {
            mgr.save_user_app(app, initial_balance);
            std::cout << "✓ Obiectiv sters.\n";
          }
        }
      } else {
        std::cout << "\n" << app.budget_ref() << "\n";
        app.budget_ref().period_report(app.get_date());
        app.budget_ref().category_report(app.get_date());
        app.budget_ref().spending_split(app.get_date());
        app.budget_ref().subscriptions_status(app.get_date());
        app.budget_ref().goals_status(app.get_date());
      }
      pause_prompt();
    } else if (cmd == 3) {
      std::cout << "\n[1] Adauga Task  [2] Finalizeaza Task  [3] Rank Level (Comparatii Binare)  [4] Focus Plan  [5] Redeschide Task  [6] Sterge Task: ";
      int topt = 0;
      std::cin >> topt;
      std::cin.ignore(1000, '\n');
      if (topt == 1) {
        task t;
        if (read_item("Format (titlu,descriere,nivel 1-3,z.l.a,progres): ", t)) {
          app.add_task(t);
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Task adaugat!\n";
        }
      } else if (topt == 2) {
        std::string title;
        std::cout << "Titlul task-ului finalizat: ";
        std::getline(std::cin, title);
        if (app.tasks_ref().complete(title)) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Task marcat ca finalizat!\n";
        } else {
          std::cout << "✕ Task-ul nu a fost gasit.\n";
        }
      } else if (topt == 5) {
        std::string title;
        std::cout << "Titlul task-ului de redeschis (anuleaza Finalizat): ";
        std::getline(std::cin, title);
        if (app.tasks_ref().reopen(title, app.get_date())) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Task redeschis!\n";
        } else {
          std::cout << "✕ Nu exista un task finalizat cu acest titlu.\n";
        }
      } else if (topt == 6) {
        const auto& ts = app.tasks_ref().get_tasks();
        for (std::size_t i = 0; i < ts.size(); ++i) std::cout << "  " << i + 1 << ". " << ts[i];
        int k = ask_index("task", ts.size());
        if (k >= 0) {
          const std::string title = ts[static_cast<std::size_t>(k)].get_title();
          const date dl = ts[static_cast<std::size_t>(k)].get_deadline();
          if (app.tasks_ref().remove_task(title, dl)) {
            mgr.save_user_app(app, initial_balance);
            std::cout << "✓ Task sters.\n";
          }
        }
      } else if (topt == 3) {
        int lvl = 1;
        std::cout << "Nivelul de prioritizat (1-3): ";
        std::cin >> lvl;
        std::cin.ignore(1000, '\n');
        app.tasks_ref().rank_level(lvl);
        mgr.save_user_app(app, initial_balance);
      } else {
        std::cout << "\n" << app.tasks_ref() << "\n";
        app.tasks_ref().focus_plan(app.get_date(), 5);
      }
      pause_prompt();
    } else if (cmd == 4) {
      std::cout << "\n[1] Adauga Eveniment  [2] Auto-Resolve Conflicte  [3] Sync Deadlines & Google (.ics)  [4] Sterge Eveniment: ";
      int copt = 0;
      std::cin >> copt;
      std::cin.ignore(1000, '\n');
      if (copt == 1) {
        calendar_event ev;
        if (read_item("Format (titlu,z.l.a,ora_start,durata_min,synced 0/1): ", ev)) {
          app.add_event(ev);
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Eveniment adaugat!\n";
        }
      } else if (copt == 2) {
        int resolved = app.calendar_ref().auto_resolve_conflicts();
        mgr.save_user_app(app, initial_balance);
        std::cout << "✓ Conflicte rezolvate automat: " << resolved << "\n";
      } else if (copt == 4) {
        const auto& es = app.calendar_ref().get_events();
        for (std::size_t i = 0; i < es.size(); ++i) std::cout << "  " << i + 1 << ". " << es[i];
        int k = ask_index("eveniment", es.size());
        if (k >= 0 && app.calendar_ref().remove_event(static_cast<std::size_t>(k))) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Eveniment sters.\n";
        }
      } else {
        app.sync_deadlines("data/gcal_sync.ics");
        mgr.save_user_app(app, initial_balance);
      }
      pause_prompt();
    } else if (cmd == 5) {
      std::cout << "\n";
      app.roadmap_ref().render_tree();
      app.roadmap_ref().print_critical_path();
      std::cout << "\n[1] Split Sub-problema  [2] Completeaza Pas  [3] Redeschide Pas  [4] Sterge Nod  [0] Inapoi: ";
      int ropt = 0;
      std::cin >> ropt;
      std::cin.ignore(1000, '\n');
      if (ropt == 1) {
        std::string parent, sub_title;
        double w = 1.0;
        std::cout << "Titlu nod parinte: ";
        std::getline(std::cin, parent);
        std::cout << "Titlu sub-problema noua: ";
        std::getline(std::cin, sub_title);
        std::cout << "Pondere (weight, ex 1.5): ";
        std::cin >> w;
        std::cin.ignore(1000, '\n');
        if (app.roadmap_ref().split_problem(parent, sub_title, w)) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Sub-problema adaugata in arbore!\n";
        } else {
          std::cout << "✕ Nodul parinte nu a fost gasit.\n";
        }
      } else if (ropt == 3) {
        std::string step;
        std::cout << "Titlu pas de redeschis: ";
        std::getline(std::cin, step);
        if (app.roadmap_ref().reopen_step(step)) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Pas redeschis (si sub-pasii lui, daca are)!\n";
        } else {
          std::cout << "✕ Nu exista un pas finalizat cu acest titlu.\n";
        }
      } else if (ropt == 4) {
        std::string step;
        std::cout << "Titlu nod de sters (se sterg si sub-nodurile): ";
        std::getline(std::cin, step);
        int id = 0;
        for (const auto& n : app.roadmap_ref().get_nodes())
          if (n.get_title() == step) { id = n.get_id(); break; }
        int removed = id ? app.roadmap_ref().remove_node(id) : 0;
        if (removed > 0) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Noduri sterse: " << removed << "\n";
        } else {
          std::cout << "✕ Nodul nu a fost gasit.\n";
        }
      } else if (ropt == 2) {
        std::string step;
        std::cout << "Titlu pas finalizat: ";
        std::getline(std::cin, step);
        if (app.roadmap_ref().complete_step(step)) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Pas completat!\n";
        } else {
          std::cout << "✕ Pasul nu a fost gasit sau este deja gata.\n";
        }
      }
      pause_prompt();
    } else if (cmd == 6) {
      std::cout << "\n[1] Check-in Obicei Azi  [2] Adauga Obicei Nou  [3] Adauga Antrenament  [4] Anuleaza Check-in Azi  [5] Sterge Obicei  [6] Sterge Antrenament: ";
      int hopt = 0;
      std::cin >> hopt;
      std::cin.ignore(1000, '\n');
      if (hopt == 1) {
        std::cout << app.habits_ref() << "\nTitlul obiceiului: ";
        std::string htitle;
        std::getline(std::cin, htitle);
        if (app.check_in_habit(htitle)) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Streak actualizat!\n";
        } else {
          std::cout << "✕ Obiceiul nu a fost gasit.\n";
        }
      } else if (hopt == 2) {
        habit h;
        if (read_item("Format (titlu,categorie,zile_pe_sapt,streak,best_streak,z.l.a): ", h)) {
          app.add_habit(h);
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Obicei adaugat!\n";
        }
      } else if (hopt == 4) {
        std::cout << app.habits_ref() << "\nTitlul obiceiului: ";
        std::string htitle;
        std::getline(std::cin, htitle);
        if (app.undo_check_in_habit(htitle)) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Check-in anulat!\n";
        } else {
          std::cout << "✕ Obiceiul nu exista sau nu are check-in la data curenta.\n";
        }
      } else if (hopt == 5) {
        const auto& hs = app.habits_ref().get_habits();
        for (std::size_t i = 0; i < hs.size(); ++i) std::cout << "  " << i + 1 << ". " << hs[i];
        int k = ask_index("obicei", hs.size());
        if (k >= 0 && app.habits_ref().remove_habit(static_cast<std::size_t>(k))) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Obicei sters.\n";
        }
      } else if (hopt == 6) {
        const auto& ws = app.fitness_ref().get_sessions();
        for (std::size_t i = 0; i < ws.size(); ++i) std::cout << "  " << i + 1 << ". " << ws[i];
        int k = ask_index("antrenament", ws.size());
        if (k >= 0 && app.fitness_ref().remove_session(static_cast<std::size_t>(k))) {
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Antrenament sters.\n";
        }
      } else if (hopt == 3) {
        workout_session ws;
        if (read_item("Format (titlu,sport,durata_min,calorii,intensitate 1-10,z.l.a): ", ws)) {
          app.add_workout(ws);
          mgr.save_user_app(app, initial_balance);
          std::cout << "✓ Antrenament inregistrat!\n";
        }
      }
      pause_prompt();
    } else if (cmd == 7) {
      app.next_day();
      mgr.save_user_app(app, initial_balance);
      std::cout << "\n» Data curenta a avansat la " << app.get_date() << "!\n";
      pause_prompt();
    }
  }
}
