#pragma once

#include <stdexcept>
#include <string>

// Ierarhia de exceptii a aplicatiei (independenta de ierarhia reminder_rule).
// Baza: lifetracker_error, derivata din std::runtime_error (deci si din std::exception).
class lifetracker_error : public std::runtime_error {
  public:
    explicit lifetracker_error(const std::string& what_arg);
};

// Valori de configurare invalide (praguri negative, intervale imposibile etc.)
class invalid_value_error : public lifetracker_error {
  public:
    invalid_value_error(const std::string& field, double value, const std::string& rule);
};

// Fisiere de date lipsa, goale sau care nu pot fi citite
class data_file_error : public lifetracker_error {
  private:
    std::string path;

  public:
    data_file_error(const std::string& file_path, const std::string& reason);

    const std::string& get_path() const { return path; }
};

// Autentificare esuata (utilizator sau parola gresite)
class auth_error : public lifetracker_error {
  public:
    explicit auth_error(const std::string& username);
};
