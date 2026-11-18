#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef __unix__
#include <unistd.h>
#endif

#include "io_utils.h"
#include "date.h"
#include "transaction.h"
#include "money_goal.h"
#include "task.h"
#include "budget.h"
#include "task_tracker.h"
#include "calendar_event.h"
#include "calendar.h"
#include "roadmap_node.h"
#include "roadmap.h"
#include "habit.h"
#include "habit_tracker.h"
#include "workout_session.h"
#include "fitness_tracker.h"
#include "lifetracker.h"
#include "account_manager.h"
#include "terminal_ui.h"
#include "gui_window.h"

#ifdef USE_QT_GUI
#include <QApplication>
#endif

static int run_legacy_demo() {
    std::cout << std::boolalpha;

    std::ifstream user_file{"data/user.txt"};
    std::string user, pass;
    double balance = 0;
    char sep = ',';
    date start;
    std::getline(user_file, user, ',');
    std::getline(user_file, pass, ',');
    user_file >> start >> sep >> balance;
    if(!user_file){
      std::cerr << "Cannot read data/user.txt\n";
      return 1;
    }

    std::vector<transaction> txs = load_all<transaction>("data/transactions.csv");
    std::vector<money_goal> goals = load_all<money_goal>("data/goals.csv");
    std::vector<task> tasks = load_all<task>("data/tasks.csv");
    std::vector<calendar_event> events = load_all<calendar_event>("data/events.csv");
    std::vector<roadmap_node> nodes = load_all<roadmap_node>("data/roadmap.csv");
    std::vector<habit> habits = load_all<habit>("data/habits.csv");
    std::vector<workout_session> workouts = load_all<workout_session>("data/workouts.csv");
    if(txs.empty() || goals.empty() || tasks.size() < 2 || events.empty() || nodes.empty() || habits.empty() || workouts.empty()){
      std::cerr << "Missing or empty data files\n";
      return 1;
    }

    // date
    std::cout << "Start: " << start << " (" << start.get_day() << "/" << start.get_month() << "/" << start.get_year() << ", " << start.weekday_name() << ", weekend: " << start.is_weekend() << ", Q" << start.quarter() << "), yesterday: " << start.yesterday() << ", tomorrow: " << start.tomorrow() << ", +5 days: " << start.add_days(5) << ", days until tomorrow: " << start.days_until(start.tomorrow()) << ", same day: " << start.same_day(start.tomorrow()) << ", same week: " << start.same_week(start.tomorrow()) << ", same month: " << start.same_month(start.tomorrow()) << ", same year: " << start.same_year(start.tomorrow()) << ", next billing: " << start.next_billing(start) << ", ics: " << start.ics_code() << "\n\n";

    // transactions + goals
    transaction& first_tx = txs.front();
    first_tx.apply_discount(0);
    std::cout << first_tx << first_tx.get_title() << " on " << first_tx.get_date() << " | " << first_tx.category_name() << " | signed: " << first_tx.signed_amount() << " | raw: " << first_tx.raw_amount() << " | sub: " << first_tx.is_membership() << " | win: " << first_tx.is_win() << " | essential: " << first_tx.is_essential() << " | day/week/month/year: " << first_tx.in_day(start) << "/" << first_tx.in_week(start) << "/" << first_tx.in_month(start) << "/" << first_tx.in_year(start) << " | next bill: " << first_tx.next_billing_date(start) << "\n";
    money_goal& first_goal = goals.front();
    first_goal.extend_deadline(10);
    std::cout << first_goal << first_goal.get_title() << " | target: " << first_goal.get_money() << " | due: " << first_goal.get_deadline() << " | days left: " << first_goal.days_left(start) << " | daily: " << first_goal.daily_target(balance, start) << "\n\n";

    // budget
    budget b{balance, txs, goals};
    b.add_goal(money_goal{"New Monitor", 1200, date{1, 5, 2027}});
    b.monthly_report(start);
    b.period_report(start);
    b.category_report(start);
    b.spending_split(start);
    b.subscriptions_status(start);
    b.forecast_balance(6, start);
    b.goals_status(start);
    std::cout << "Subscriptions: " << b.subscriptions_cost() << ", left: " << b.get_left() << "\n" << b;
    budget copy{b};
    budget other;
    other = copy;
    std::cout << "\n";

    // tasks
    task& first_task = tasks.front();
    first_task.check_missed(start);
    first_task.postpone(1);
    std::cout << first_task << first_task.get_title() << " (" << first_task.get_description() << ", level " << first_task.get_urgency() << ", " << first_task.get_progress() << "%, due " << first_task.get_deadline() << ", score " << first_task.urgency_score(start) << ") more urgent than " << tasks.back().get_title() << ": " << first_task.more_urgent(tasks.back()) << ", diff: " << first_task.deadline_diff(tasks.back()) << " days, level 1: " << first_task.has_level(1) << ", done: " << first_task.is_done() << ", missed: " << first_task.is_missed() << "\n\n";

    // task tracker: ranking
    task_tracker tt{tasks};
    int level = 0;
    std::cout << "Which level do you want to rank? (1 = most urgent, " << task::levels() << " = least urgent): ";
    std::cin >> level;
    std::cout << "\n";
    tt.rank_level(level);
    std::cout << "Priority list:\n";
    for(const auto& t : tt.prioritize()) std::cout << "  " << t;
    tt.update_missed(start);
    tt.escalate_due_soon(start, 2);
    tt.focus_plan(start, 3);
    std::string title;
    std::cout << "Which task did you finish? ";
    std::getline(std::cin >> std::ws, title);
    if(!title.empty() && title.back() == '\r') title.pop_back();
    std::cout << "\n" << (tt.complete(title) ? "Completed " : "No open task named ") << title << "\n";
    std::cout << tt << tt.completion() << "%\n\n";

    // calendar
    const calendar_event& first_ev = events.front();
    std::cout << "First event: " << first_ev.get_title() << " at " << first_ev.get_start_hour() << ":00 for " << first_ev.get_duration() << "m\n";
    calendar cal{user + "@gmail.com", events};
    std::cout << "Conflicts found: " << cal.detect_conflicts() << "\n";
    std::cout << "Auto-resolved conflicts: " << cal.auto_resolve_conflicts() << "\n";
    std::cout << "Free 60m slot tomorrow: " << cal.find_free_slot(start.tomorrow(), 60) << ":00\n";
    cal.modify_event(events.back().get_title(), start.tomorrow(), 16);
    cal.week_agenda(start);
    cal.sync_google("data/gcal_sync.ics");
    std::cout << cal << "\n";

    // roadmap
    const roadmap_node& first_node = nodes.front();
    std::cout << "Root node #" << first_node.get_id() << " (parent " << first_node.get_parent() << "): " << first_node.get_title() << ", weight " << first_node.get_weight() << ", done: " << first_node.is_done() << "\n";
    roadmap rm{"Launch Life Tracker", nodes};
    rm.split_problem("Smart Task Tracker", "Deadline Delta View", 1.5);
    rm.complete_step("Binary Search Ranking");
    rm.render_tree();
    rm.print_critical_path();
    std::cout << "Overall progress: " << rm.overall_progress() << "%, next step: " << rm.next_step() << "\n" << rm << "\n";

    // habits + fitness
    const habit& first_hab = habits.front();
    std::cout << "First habit: " << first_hab.get_title() << " (" << first_hab.get_category() << "), streak " << first_hab.get_streak() << "/" << first_hab.get_best_streak() << ", score " << first_hab.consistency_score() << "%\n";
    habit_tracker ht{habits};
    ht.log_habit(first_hab.get_title(), start);
    ht.update_streaks(start);
    std::cout << ht << "\n";

    const workout_session& first_ws = workouts.front();
    std::cout << "First workout: " << first_ws.get_title() << " (" << first_ws.get_sport() << ") on " << first_ws.get_date() << ", " << first_ws.get_duration() << "m, " << first_ws.get_calories() << " kcal, intensity " << first_ws.get_intensity() << ", load " << first_ws.training_load() << "\n";
    fitness_tracker ft{2500, workouts};
    ft.sport_breakdown();
    std::cout << "Weekly kcal: " << ft.weekly_calories(start) << ", load: " << ft.weekly_load(start) << ", goal: " << ft.goal_completion(start) << "%\n" << ft << "\n";

    // lifetracker
    lifetracker app{user, pass, start, b, tt, cal, rm, ht, ft};
    std::string typed_user, typed_pass;
    std::cout << "Login (user password): ";
    std::cin >> typed_user >> typed_pass;
    std::cout << "\n";
    if(!app.login(typed_user, typed_pass)){
      std::cout << "Wrong credentials\n";
      return 1;
    }

    transaction new_tx;
    if(read_item("New transaction (title,category,d.m.y,amount,subscription,win): ", new_tx)) app.add_transaction(new_tx);
    task new_task;
    if(read_item("New task (title,description,level,d.m.y,progress): ", new_task)) app.add_task(new_task);
    calendar_event new_ev;
    if(read_item("New event (title,d.m.y,start_hour,duration_min,synced): ", new_ev)) app.add_event(new_ev);
    habit new_hab;
    if(read_item("New habit (title,category,days_per_week,streak,best_streak,d.m.y): ", new_hab)) app.add_habit(new_hab);
    app.check_in_habit(new_hab.get_title());
    workout_session new_ws;
    if(read_item("New workout (title,sport,duration_min,calories,intensity,d.m.y): ", new_ws)) app.add_workout(new_ws);
    app.sync_deadlines("data/gcal_sync.ics");
    int days = 0;
    std::cout << "Days to advance: ";
    std::cin >> days;
    std::cout << "\n";
    for(int i = 0; i < days; i++) app.next_day();
    app.summary();
    std::cout << app << "\n";

    return 0;
}

namespace {
// true daca stdin este redirectionat dintr-un fisier/pipe (ex: ./oop_main < tastatura.txt)
bool stdin_is_redirected() {
#ifdef __unix__
    return !isatty(STDIN_FILENO);
#else
    return false;
#endif
}
} // namespace

int main(int argc, char* const argv[]) {
#ifdef LT_ROOT_DIR
    std::error_code ec;
    if (!std::filesystem::exists("data/user.txt", ec) && std::filesystem::exists(LT_ROOT_DIR, ec)) {
        std::filesystem::current_path(LT_ROOT_DIR, ec);
    }
#endif

    bool force_tui = false;
    bool force_cli = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--tui" || arg == "-t") force_tui = true;
        if (arg == "--cli" || arg == "--demo" || arg == "--batch") force_cli = true;
    }

    // Daca stdin este redirectionat din fisier (ex: ./scripts/run_valgrind.sh < tastatura.txt), rulam scenariul automat CLI
    if (stdin_is_redirected() && !force_tui) {
        force_cli = true;
    }

    if (force_cli) {
        return run_legacy_demo();
    }

    account_manager mgr{"data"};

#ifdef USE_QT_GUI
    if (!force_tui) {
        // QApplication cere char** modificabil, asa ca ii dam o copie a argumentelor
        int qt_argc = argc;
        std::vector<std::string> qt_args(argv, argv + argc);
        std::vector<char*> qt_argv;
        for (auto& a : qt_args) qt_argv.push_back(a.data());
        qt_argv.push_back(nullptr);
        QApplication qt_app{qt_argc, qt_argv.data()};
        gui_window win{mgr};
        win.show();
        return qt_app.exec();
    }
#endif

    terminal_ui tui{mgr};
    return tui.run();
}
