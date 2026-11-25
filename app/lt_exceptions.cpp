#include "lt_exceptions.h"

#include <sstream>

namespace {
  std::string describe_value(const std::string& field, double value, const std::string& rule) {
    std::ostringstream os;
    os << "Valoare invalida pentru '" << field << "': " << value << " (" << rule << ")";
    return os.str();
  }
}

lifetracker_error::lifetracker_error(const std::string& what_arg)
  : std::runtime_error{what_arg} {}

invalid_value_error::invalid_value_error(const std::string& field, double value, const std::string& rule)
  : lifetracker_error{describe_value(field, value, rule)} {}

data_file_error::data_file_error(const std::string& file_path, const std::string& reason)
  : lifetracker_error{"Eroare fisier de date '" + file_path + "': " + reason}, path{file_path} {}

auth_error::auth_error(const std::string& username)
  : lifetracker_error{"Autentificare esuata pentru utilizatorul '" + username + "'"} {}
