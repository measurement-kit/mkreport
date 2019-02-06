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

/// OpenRequest is a request to open a report with a report.
class OpenRequest {
 public:
  /// probe_asn is the probe ASN. Autocomputed if not set.
  std::string probe_asn;

  /// probe_cc is the probe country code. Autocomputed if not set.
  std::string probe_cc;

  /// software_name is the name of the application. If not set,
  /// we will refuse to attempt creating a report.
  std::string software_name;

  /// software_version is the version of the application. If not set,
  /// we will refuse to attempt creating a report.
  std::string software_version;

  /// test_name is the nettest name. If not set, we will refuse to
  /// attempt creating a report.
  std::string test_name;

  /// test_version is the nettest version. If not set, we'll refuse to
  /// attempt creating a report.
  std::string test_version;

  /// test_start_time is the time when the test started. If not set, we
  /// will set it to the current time and date.
  std::string test_start_time;

  /// bouncer_base_url is the OONI bouncer base URL. If this is not
  /// set, we will use a reasonable default.
  std::string bouncer_base_url;

  /// collector_base_url is the OONI collector base URL. If this is not
  /// set, we will use the OONI bouncer to discover a collector.
  std::string collector_base_url;

  /// ca_bundle_path is the path to the CA bundle (required on mobile).
  std::string ca_bundle_path;

  /// asn_db_path is the path to MaxMind's ASN mmdb database. This is needed
  /// if we need to autodiscover the probe_asn.
  std::string asn_db_path;

  /// country_db_path is the path to MaxMind's country mmdb database. This is
  /// needed if we need to autodiscover the probe_cc.
  std::string country_db_path;

  /// report_id is the identifier of the report. If this is not set, we'll
  /// attempt to open a report to get a new report ID. Otherwise, we'll use
  /// the report ID that you provided us.
  std::string report_id;

  /// timeout is the timeout for HTTP requests (in seconds).
  int64_t timeout = 30;

  /// Report creates a new empty report. All the fields will have the
  /// initial value that you have seen above.
  Report() noexcept;

  /// The copy constructor is explicitly deleted.
  Report(const Report &) noexcept = delete;

  /// The copy assignment is explicitly deleted.
  Report &operator=(const Report &) noexcept = delete;

  /// The move constructor will move the internal state.
  Report(Report &&) noexcept = default;

  /// The move assignment will move the internal state.
  Report &operator=(Report &&) noexcept = delete;

  // TODO(bassosimone): implement
  bool open(std::vector<std::string> &logs) noexcept;

  /// submit_measurement submits @p measurement to the OONI collector. The
  /// @p measurement parameter MUST be a serialized JSON containing an object
  /// consistent with the OONI measurement specification. This function will
  /// return true on success and false on failure. In case of failure, you
  /// SHOULD inspect the @p logs, which will explain what went wrong.
  bool submit_measurement(
      std::string measurement, std::vector<std::string> &logs) noexcept;

  // TODO(bassosimone): document
  bool close(std::vector<std::string> &logs) noexcept;

  // TODO(bassosimone): document
  ~Report() noexcept;
};

// The implementation can be included inline by defining this preprocessor
// symbol. If you only care about API, you can stop reading here.
#ifdef MKREPORT_INLINE_IMPL

// TODO(bassosimone): implement

#endif  // MKREPORT_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKREPORT_HPP
