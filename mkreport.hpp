// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKREPORT_HPP
#define MEASUREMENT_KIT_MKREPORT_HPP

#include <stdint.h>

#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace mk {
namespace report {

/// ooni_date_now formats the current date and time according to the
/// format that is expected by the OONI collector.
std::string ooni_date_now() noexcept;

/// monotonic_seconds_now returns the current time in seconds according
/// to the C++11 monotonic clock.
double monotonic_seconds_now() noexcept;

/// Measurement contains info about a measurement. The code that will
/// actually run a measurement is not here. You should (1) save the
/// measurement input (an empty string is fine), (2) call start() before
/// starting the measurements, (3) run the measurement and store its
/// JSON result in test_keys, and finally (4) call stop(). You can then
/// pass the Measurement object to Report for submission.
class Measurement {
 public:
  /// input is the measurement input.
  std::string input;

  /// start_time is the time when the measurement started.
  std::string start_time;

  /// test_keys contains the measurement test keys as a serialized
  /// JSON object. Measurement specific code will produce this.
  std::string test_keys;

  /// runtime is the time for which the measurement run.
  double runtime = 0.0;

  /// start records the measurement start_time and the moment in
  /// which the test started. That will be useful later to compute
  /// the test runtime. The moment in which the test started is
  /// computed using a monotonic clock.
  void start() noexcept;

  /// stop computes the test_runtime variable. The delta time
  /// is computed using a monotonic clock.
  void stop() noexcept;

 private:
  double beginning_ = 0.0;
};

/// Report represent a report to be submitted to the OONI collector.
class Report {
 public:
  /// annotation contains optional results annotations.
  std::map<std::string, std::string> annotations;

  /// probe_asn is the probe ASN.
  std::string probe_asn;

  /// probe_cc is the probe country code.
  std::string probe_cc;

  /// software_name is the name of the application.
  std::string software_name;

  /// software_version is the version of the application.
  std::string software_version;

  /// test_name is the nettest name.
  std::string test_name;

  /// test_version is the nettest version.
  std::string test_version;

  /// test_start_time is the time when the test started.
  std::string test_start_time;

  /// bouncer_base_url is the OONI bouncer base URL.
  std::string bouncer_base_url;

  /// collector_base_url is the OONI collector base URL.
  std::string collector_base_url;

  /// ca_bundle_path is the path to the CA bundle (required on mobile).
  std::string ca_bundle_path;

  /// asn_db_path is the path to MaxMind's ASN mmdb database.
  std::string asn_db_path;

  /// country_db_path is the path to MaxMind's country mmdb database.
  std::string country_db_path;

  /// id is the identifier of the report.
  std::string id;

  /// timeout is the timeout for HTTP requests (in seconds).
  int64_t timeout = 30;

  /// Report creates a new empty report.
  Report() noexcept;

  /// The copy constructor will copy the internal state variables.
  Report(const Report &) noexcept = default;

  /// The copy assignment will copy the internal state variables.
  Report &operator=(const Report &) noexcept = default;

  /// The move constructor will move the internal state variables.
  Report(Report &&) noexcept = default;

  /// The move assignment will move the internal state variables.
  Report &operator=(Report &&) noexcept = delete;

  /// open opens a report with the configured OONI collector. This function
  /// will return true on success and false on failure. In case of failure, you
  /// SHOULD inspect the @p logs, which will explain what went wrong. The
  /// algorithm is the following: open will fail if the report ID is nonempty
  /// as well as if some mandatory field is missing; otherwise it will attempt
  /// to open the report, write the report ID, and return whether it succeded
  /// in opening the report or not.
  bool open(std::vector<std::string> &logs) noexcept;

  /// submit_measurement turns @p measurement into a JSON object and submits it
  /// to the configured OONI collector. This function will return true on
  /// success and false on failure. This function will also fail immediately if
  /// the report ID is empty or if the collector base URL is empty. In case of
  /// failure, inspect the @p logs to have a sense of what went wrong.
  bool submit_measurement(
      Measurement measurement, std::vector<std::string> &logs) noexcept;

  /// close closes the report with the OONI collector. This function will
  /// return true on success and false on failure. In case of failure, you
  /// SHOULD inspect the @p logs, which will explain what went wrong. The
  /// algorithm is the following: if there is no report ID, then this
  /// function will return false; otherwise, if there is no collector base
  /// URL, this function will return false; otherwise, it will attempt to
  /// close the report, clear the report ID, and return whether it succeded
  /// in closing the report or not.
  bool close(std::vector<std::string> &logs) noexcept;

  /// ~Report cleans the allocated resources. Notably, if a report is open
  /// and it has not been closed, this function will also close it.
  ~Report() noexcept;

  /// make_content crates a serialized JSON measurement object in @p content
  /// from the @p measurement. This function will return true on success, and
  /// false on failure. In the latter case, the @p logs parameter will
  /// contain explanatory logs.
  bool make_content(const Measurement &measurement, std::string &content,
                    std::vector<std::string> &logs) noexcept;
};

}  // namespace report
}  // namespace mk

// The implementation can be included inline by defining this preprocessor
// symbol. If you only care about API, you can stop reading here.
#ifdef MKREPORT_INLINE_IMPL

#include <exception>
#include <utility>

#include "date.h"
#include "json.hpp"
#include "mkcollector.hpp"

namespace mk {
namespace report {

std::string ooni_date_now() noexcept {
  // Implementation note: to avoid using the C standard library that has
  // given us many headaches on Windows because of parameter validation we
  // go for a fully C++11 solution based on <chrono> and on the C++11
  // HowardHinnant/date library, which will be available as part of the
  // C++ standard library starting from C++20.
  //
  // Explanation of the algorithm:
  //
  // 1. get the current system time
  // 2. round the time point obtained in the previous step to an integral
  //    number of seconds since the EPOCH used by the system clock
  // 3. create a system clock time point from the integral number of seconds
  // 4. convert the previous result to string using HowardInnant/date
  // 5. if there is a decimal component (there should be one given how the
  //    library we use works) remove it, because OONI doesn't like it
  //
  // (There was another way to deal with fractionary seconds, i.e. using '%OS',
  //  but this solution seems better to me because it's less obscure.)
  using namespace std::chrono;
  constexpr auto fmt = "%Y-%m-%d %H:%M:%S";
  auto sys_point = system_clock::now();                                    // 1
  auto as_seconds = duration_cast<seconds>(sys_point.time_since_epoch());  // 2
  auto back_as_sys_point = system_clock::time_point(as_seconds);           // 3
  auto s = date::format(fmt, back_as_sys_point);                           // 4
  if (s.find(".") != std::string::npos) s = s.substr(0, s.find("."));      // 5
  return s;
}

double monotonic_seconds_now() noexcept {
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch());
  return now.count();
}

void Measurement::start() noexcept {
  start_time = ooni_date_now();
  beginning_ = monotonic_seconds_now();
}

void Measurement::stop() noexcept {
  runtime = monotonic_seconds_now() - beginning_;
}

Report::Report() noexcept {}

bool Report::open(std::vector<std::string> &logs) noexcept {
  if (!id.empty()) {
    logs.push_back("A report is already open");
    return false;
  }
  mk::collector::OpenRequest request;
  if (probe_asn.empty()) {
    logs.push_back("Please, initialize the probe_asn");
    return false;
  }
  request.probe_asn = probe_asn;
  if (probe_cc.empty()) {
    logs.push_back("Please, initialize the probe_cc.");
    return false;
  }
  request.probe_cc = probe_cc;
  if (software_name.empty()) {
    logs.push_back("Please, initialize the software_name");
    return false;
  }
  request.software_name = software_name;
  if (software_version.empty()) {
    logs.push_back("Please, initialize the software_version");
    return false;
  }
  request.software_version = software_version;
  if (test_name.empty()) {
    logs.push_back("Please, initialize the test_name");
    return false;
  }
  request.test_name = test_name;
  if (test_version.empty()) {
    logs.push_back("Please, initialize the test_version");
    return false;
  }
  request.test_version = test_version;
  if (test_start_time.empty()) {
    logs.push_back("Please, initialize the test_start_time");
    return false;
  }
  request.test_start_time = test_start_time;
  if (collector_base_url.empty()) {
    logs.push_back("Please, initialize the collector_base_url");
    return false;
  }
  request.base_url = collector_base_url;
  request.ca_bundle_path = ca_bundle_path;
  request.timeout = timeout;
  mk::collector::OpenResponse response = mk::collector::open(request);
  std::swap(logs, response.logs);
  std::swap(id, response.report_id);
  return response.good;
}

bool Report::submit_measurement(
    Measurement measurement, std::vector<std::string> &logs) noexcept {
  if (id.empty()) {
    logs.push_back("No configured report ID.");
    return false;
  }
  mk::collector::UpdateRequest request;
  request.report_id = id;
  if (collector_base_url.empty()) {
    logs.push_back("No configured collector_base_url.");
    return false;
  }
  request.base_url = collector_base_url;
  if (!make_content(measurement, request.content, logs)) {
    return false;
  }
  request.ca_bundle_path = ca_bundle_path;
  request.timeout = timeout;
  mk::collector::UpdateResponse response = mk::collector::update(request);
  std::swap(logs, response.logs);
  return response.good;
}

bool Report::close(std::vector<std::string> &logs) noexcept {
  if (id.empty()) {
    logs.push_back("No configured report ID.");
    return false;
  }
  mk::collector::CloseRequest request;
  request.report_id = id;
  if (collector_base_url.empty()) {
    logs.push_back("No configured collector_base_url.");
    return false;
  }
  request.base_url = collector_base_url;
  request.ca_bundle_path = ca_bundle_path;
  request.timeout = timeout;
  mk::collector::CloseResponse response = mk::collector::close(request);
  std::swap(logs, response.logs);
  id = "";  // Clear the report ID as this report is now closed
  return response.good;
}

Report::~Report() noexcept {
  std::vector<std::string> logs;
  (void)close(logs);
}

bool Report::make_content(const Measurement &measurement, std::string &content,
                          std::vector<std::string> &logs) noexcept {
  // Step 1: parse test keys and make sure it's a JSON object.
  nlohmann::json tk;
  try {
    tk = nlohmann::json::parse(measurement.test_keys);
  } catch (const std::exception &exc) {
    logs.push_back(exc.what());
    return false;
  }
  if (!tk.is_object()) {
    logs.push_back("Test keys is not a JSON object");
    return false;
  }
  // Step 2: fill the measurement JSON object
  nlohmann::json m;
  m["annotations"] = annotations;
  m["data_format_version"] = "0.2.0";
  m["id"] = "bdd20d7a-bba5-40dd-a111-9863d7908572";
  m["input"] = measurement.input;
  m["input_hashes"] = nlohmann::json::array();
  m["measurement_start_time"] = measurement.start_time;
  m["options"] = nlohmann::json::array();
  m["probe_asn"] = probe_asn;
  m["probe_cc"] = probe_cc;
  m["probe_city"] = nullptr;
  m["report_id"] = id;
  m["software_name"] = software_name;
  m["software_version"] = software_version;
  m["test_helpers"] = nlohmann::json::object();
  m["test_keys"] = tk;
  m["test_keys"]["client_resolver"] = "91.80.37.104";  // TODO(bassosimone): fix
  m["test_name"] = test_name;
  m["test_runtime"] = measurement.runtime;
  m["test_start_time"] = test_start_time;
  m["test_version"] = test_version;
  // Step 3: dump the measurement message
  try {
    content = m.dump();
  } catch (const std::exception &exc) {
    logs.push_back(exc.what());
    return false;
  }
  return true;
}

}  // namespace report
}  // namespace mk
#endif  // MKREPORT_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKREPORT_HPP
