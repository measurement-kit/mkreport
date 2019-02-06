// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKREPORT_HPP
#define MEASUREMENT_KIT_MKREPORT_HPP

#include <stdint.h>

#include <string>
#include <vector>

namespace mk {
namespace report {

/// Report represent a report to be submitted to the OONI collector.
class Report {
 public:
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

  /// submit_test_keys wraps @p test_keys into a measurement and submits them
  /// to the configured OONI collector. The @p test_keys parameter MUST be a
  /// serialized JSON containing measurement test keys. The @p test_runtime
  /// argument indicates the runtime of the specific measurement measured
  /// in seconds. This function will return true on success and false on
  /// failure. This functin will also fail immediately if the report ID is
  /// empty or if the collector base URL is empty. In case of failure, inspect
  /// the @p logs to have a sense of what went wrong.
  bool submit_test_keys(std::string test_keys, double test_runtime,
                        std::vector<std::string> &logs) noexcept;

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
  /// from the serialized JSON @p test_keys and the given @p test_runtime. It
  /// will return true on success and false on failure. In the latter case,
  /// the @p logs parameter will contain explanatory logs.
  bool make_content(
      const std::string &test_keys, double test_runtime, std::string &content,
      std::vector<std::string> &logs) noexcept;
};

}  // namespace report
}  // namespace mk

// The implementation can be included inline by defining this preprocessor
// symbol. If you only care about API, you can stop reading here.
#ifdef MKREPORT_INLINE_IMPL

#include <exception>
#include <utility>

#include "json.hpp"
#include "mkcollector.hpp"

namespace mk {
namespace report {

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

bool Report::submit_test_keys(std::string test_keys, double test_runtime,
                              std::vector<std::string> &logs) noexcept {
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
  if (!make_content(test_keys, test_runtime, request.content, logs)) {
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

bool Report::make_content(
    const std::string &test_keys, double test_runtime, std::string &content,
    std::vector<std::string> &logs) noexcept {
  // Step 1: parse test keys and make sure it's a JSON object.
  nlohmann::json tk;
  try {
    tk = nlohmann::json::parse(test_keys);
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
  m["annotations"] = nlohmann::json::object();  // TODO(bassosimone): expose
  m["data_format_version"] = "0.2.0";
  m["id"] = "bdd20d7a-bba5-40dd-a111-9863d7908572";
  m["input"] = nullptr;  // TODO(bassosimone): expose
  m["input_hashes"] = nlohmann::json::array();
  m["measurement_start_time"] = "2018-11-01 15:33:20";  // TODO(bassosimone): fix
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
  m["test_runtime"] = test_runtime;
  m["test_start_time"] = "2018-11-01 15:33:17";  // TODO(bassosimone): fix
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
