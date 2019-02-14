// NOMINMAX is a workaround so that windows.h does no define the `min`
// and `max` macros that conflict with the STL.
#ifdef _WIN32
#define NOMINMAX
#endif

#include "mkmock.hpp"

#define MKCURL_INLINE_IMPL
#include "mkcurl.hpp"

#define MKDATA_INLINE_IMPL
#include "mkdata.hpp"

#define MKIPLOOKUP_INLINE_IMPL
#include "mkiplookup.hpp"

#define MKBOUNCER_INLINE_IMPL
#include "mkbouncer.hpp"

#define MKCOLLECTOR_INLINE_IMPL
#include "mkcollector.hpp"

#define MKMMDB_INLINE_IMPL
#include "mkmmdb.hpp"

MKMOCK_DEFINE_HOOK(
    bouncer_response_collectors, std::vector<mk::bouncer::Record>);
MKMOCK_DEFINE_HOOK(iplookup_response_good, bool);
MKMOCK_DEFINE_HOOK(mmdb_asn_lookup, bool);
MKMOCK_DEFINE_HOOK(mmdb_cc_lookup, bool);
MKMOCK_DEFINE_HOOK(report_autodiscover_collector, bool);
MKMOCK_DEFINE_HOOK(report_open, bool);
MKMOCK_DEFINE_HOOK(report_id, std::string);
MKMOCK_DEFINE_HOOK(report_submit_measurement_json, bool);

#define MKREPORT_MOCK
#define MKREPORT_INLINE_IMPL
#include "mkreport.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <chrono>
#include <iostream>
#include <thread>

TEST_CASE("autodiscover_collector_with_bouncer works correctly") {
  SECTION("when the test name is empty") {
    std::vector<std::string> logs;
    mk::report::Report report;
    REQUIRE(!report.autodiscover_collector_with_bouncer("", logs));
  }

  SECTION("when the test version is empty") {
    std::vector<std::string> logs;
    mk::report::Report report;
    report.test_name = "dummy";
    REQUIRE(!report.autodiscover_collector_with_bouncer("", logs));
  }

  SECTION("when the response is not good") {
    std::vector<std::string> logs;
    mk::report::Report report;
    report.test_name = "dummy";
    report.test_version = "0.0.1";
    // Set the URL to nonexistent to trigger a 404
    std::string base_url = "https://bouncer.ooni.io/nonexistent";
    REQUIRE(!report.autodiscover_collector_with_bouncer(base_url, logs));
  }

  SECTION("with no available collectors") {
    MKMOCK_WITH_ENABLED_HOOK(bouncer_response_collectors, {}, {
      std::vector<std::string> logs;
      mk::report::Report report;
      report.test_name = "dummy";
      report.test_version = "0.0.1";
      REQUIRE(!report.autodiscover_collector_with_bouncer("", logs));
    });
  }
}

TEST_CASE("autodiscover_probe_asn_probe_cc works correctly") {
  SECTION("with an error while discovering the IP address") {
    MKMOCK_WITH_ENABLED_HOOK(iplookup_response_good, false, {
      mk::report::Report report;
      std::vector<std::string> logs;
      REQUIRE(!report.autodiscover_probe_asn_probe_cc(
              ".mkbuild/download/asn.mmdb",
              ".mkbuild/download/country.mmdb", logs));
    });
  }

  SECTION("with an invalid ASN DB") {
    mk::report::Report report;
    std::vector<std::string> logs;
    REQUIRE(!report.autodiscover_probe_asn_probe_cc(
          "nonexistent.mmdb", ".mkbuild/download/country.mmdb", logs));
  }

  SECTION("with an error while querying the ASN DB") {
    MKMOCK_WITH_ENABLED_HOOK(mmdb_asn_lookup, false, {
      mk::report::Report report;
      std::vector<std::string> logs;
      REQUIRE(!report.autodiscover_probe_asn_probe_cc(
              ".mkbuild/download/asn.mmdb",
              ".mkbuild/download/country.mmdb", logs));
    });
  }

  SECTION("with an invalid country DB") {
    mk::report::Report report;
    std::vector<std::string> logs;
    REQUIRE(!report.autodiscover_probe_asn_probe_cc(
          ".mkbuild/download/asn.mmdb", "nonexistent.mmdb", logs));
  }

  SECTION("with an error while querying the country DB") {
    MKMOCK_WITH_ENABLED_HOOK(mmdb_cc_lookup, false, {
      mk::report::Report report;
      std::vector<std::string> logs;
      REQUIRE(!report.autodiscover_probe_asn_probe_cc(
              ".mkbuild/download/asn.mmdb",
              ".mkbuild/download/country.mmdb", logs));
    });
  }
}

TEST_CASE("open works correctly") {
  SECTION("when the report is already open") {
    mk::report::Report report;
    report.id = "xx";
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }

  SECTION("when the probe_asn is empty") {
    mk::report::Report report;
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }

  SECTION("when the probe_cc is empty") {
    mk::report::Report report;
    report.probe_asn = "AS30722";
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }

  SECTION("when the software_name is empty") {
    mk::report::Report report;
    report.probe_asn = "AS30722";
    report.probe_cc = "IT";
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }

  SECTION("when the software_version is empty") {
    mk::report::Report report;
    report.probe_asn = "AS30722";
    report.probe_cc = "IT";
    report.software_name = "antanikit";
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }

  SECTION("when the test_name is empty") {
    mk::report::Report report;
    report.probe_asn = "AS30722";
    report.probe_cc = "IT";
    report.software_name = "antanikit";
    report.software_version = "1.0";
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }

  SECTION("when the test_version is empty") {
    mk::report::Report report;
    report.probe_asn = "AS30722";
    report.probe_cc = "IT";
    report.software_name = "antanikit";
    report.software_version = "1.0";
    report.test_name = "foobar";
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }

  SECTION("when the test_start_time is empty") {
    mk::report::Report report;
    report.probe_asn = "AS30722";
    report.probe_cc = "IT";
    report.software_name = "antanikit";
    report.software_version = "1.0";
    report.test_name = "foobar";
    report.test_version = "1.1.1";
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }

  SECTION("when the collector_base_url is empty") {
    mk::report::Report report;
    report.probe_asn = "AS30722";
    report.probe_cc = "IT";
    report.software_name = "antanikit";
    report.software_version = "1.0";
    report.test_name = "foobar";
    report.test_version = "1.1.1";
    report.test_start_time = "2018-12-23 08:30:00";
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }
}

TEST_CASE("submit works correctly") {
  SECTION("when the report is not open") {
    std::vector<std::string> logs;
    mk::report::Report report;
    mk::report::Measurement measurement;
    REQUIRE(!report.submit_measurement(measurement, logs));
  }

  SECTION("when the collector_base_url is empty") {
    std::vector<std::string> logs;
    mk::report::Report report;
    report.id = "xx";
    mk::report::Measurement measurement;
    REQUIRE(!report.submit_measurement(measurement, logs));
  }

  SECTION("when make_content fails") {
    mk::report::Report report;
    report.id = "xx";
    report.collector_base_url = "https://a.collector.ooni.io";
    std::vector<std::string> logs;
    mk::report::Measurement measurement;
    measurement.test_keys = "{";  // invalid JSON
    REQUIRE(!report.submit_measurement(measurement, logs));
  }
}

// clang-format off
const std::vector<uint8_t> binary_data{
  0x57, 0xe5, 0x79, 0xfb, 0xa6, 0xbb, 0x0d, 0xbc, 0xce, 0xbd, 0xa7, 0xa0,
  0xba, 0xa4, 0x78, 0x78, 0x12, 0x59, 0xee, 0x68, 0x39, 0xa4, 0x07, 0x98,
  0xc5, 0x3e, 0xbc, 0x55, 0xcb, 0xfe, 0x34, 0x3c, 0x7e, 0x1b, 0x5a, 0xb3,
  0x22, 0x9d, 0xc1, 0x2d, 0x6e, 0xca, 0x5b, 0xf1, 0x10, 0x25, 0x47, 0x1e,
  0x44, 0xe2, 0x2d, 0x60, 0x08, 0xea, 0xb0, 0x0a, 0xcc, 0x05, 0x48, 0xa0,
  0xf5, 0x78, 0x38, 0xf0, 0xdb, 0x3f, 0x9d, 0x9f, 0x25, 0x6f, 0x89, 0x00,
  0x96, 0x93, 0xaf, 0x43, 0xac, 0x4d, 0xc9, 0xac, 0x13, 0xdb, 0x22, 0xbe,
  0x7a, 0x7d, 0xd9, 0x24, 0xa2, 0x52, 0x69, 0xd8, 0x89, 0xc1, 0xd1, 0x57,
  0xaa, 0x04, 0x2b, 0xa2, 0xd8, 0xb1, 0x19, 0xf6, 0xd5, 0x11, 0x39, 0xbb,
  0x80, 0xcf, 0x86, 0xf9, 0x5f, 0x9d, 0x8c, 0xab, 0xf5, 0xc5, 0x74, 0x24,
  0x3a, 0xa2, 0xd4, 0x40, 0x4e, 0xd7, 0x10, 0x1f
};
// clang-format on

TEST_CASE("make_content works correctly") {
  SECTION("when the JSON is not an object") {
    mk::report::Measurement measurement;
    measurement.test_keys = "[]";  // not an object
    mk::report::Report report;
    std::vector<std::string> logs;
    std::string content;
    REQUIRE(!report.make_content(measurement, content, logs));
  }

  SECTION("when we cannot serialize the complete JSON") {
    mk::report::Measurement measurement;
    mk::report::Report report;
    measurement.test_keys = R"({"success": true})";
    report.probe_asn = std::string{
      (char *)binary_data.data(), binary_data.size()};
    std::vector<std::string> logs;
    std::string content;
    REQUIRE(!report.make_content(measurement, content, logs));
    for (auto &log : logs) std::clog << log << std::endl;
  }
}

TEST_CASE("submit_measurement_internal works correctly") {
  SECTION("when the report ID is empty") {
    mk::collector::UpdateRequest r;
    std::vector<std::string> logs;
    REQUIRE(!mk::report::submit_measurement_internal(
          std::move(r), "", "", "", 30, logs));
  }

  SECTION("when the collector base URL is empty") {
    mk::collector::UpdateRequest r;
    std::vector<std::string> logs;
    REQUIRE(!mk::report::submit_measurement_internal(
          std::move(r), "xx", "", "", 30, logs));
  }
}

static std::string sample_report = R"({
    "annotations": {
        "engine_name": "libmeasurement_kit",
        "engine_version": "0.9.0",
        "engine_version_full": "v0.9.0",
        "platform": "linux"
    },
    "data_format_version": "0.2.0",
    "id": "13bc6fa8-d149-4a64-bac5-d0a444f9ba94",
    "input": "www.kernel.org",
    "input_hashes": [],
    "measurement_start_time": "2018-02-08 09:52:33",
    "options": [],
    "probe_asn": "AS15169",
    "probe_cc": "US",
    "probe_city": null,
    "probe_ip": "127.0.0.1",
    "report_id": "20180208T095233Z_AS15169_O986SVua4krXdAnMx3aGC83INNJAo1GTZII2OwBQx2H4Qx0LKA",
    "software_name": "measurement_kit",
    "software_version": "0.9.0",
    "test_helpers": {},
    "test_keys": {
        "client_resolver": "172.217.33.130",
        "connection": "success"
    },
    "test_name": "tcp_connect",
    "test_runtime": 0.039698123931884766,
    "test_start_time": "2018-02-08 09:52:31",
    "test_version": "0.1.0"
})";

TEST_CASE("resubmit_measurement works correctly") {
  SECTION("when the JSON does not parse") {
    std::vector<std::string> logs;
    std::string id;
    auto ok = mk::report::resubmit_measurement(
        "{", ".mkbuild/download/ca-bundle.pem", 30, logs, id);
    REQUIRE(!ok);
  }

  SECTION("when the JSON does not contain all required fields") {
    std::vector<std::string> logs;
    std::string id;
    auto ok = mk::report::resubmit_measurement(
        "{}", ".mkbuild/download/ca-bundle.pem", 30, logs, id);
    REQUIRE(!ok);
  }

  SECTION("when we cannot autodiscover a suitable collector") {
    MKMOCK_WITH_ENABLED_HOOK(report_autodiscover_collector, false, {
      std::vector<std::string> logs;
      std::string id;
      auto ok = mk::report::resubmit_measurement(
          sample_report, ".mkbuild/download/ca-bundle.pem", 30, logs, id);
      REQUIRE(!ok);
    });
  }

  SECTION("when we cannot open a report") {
    MKMOCK_WITH_ENABLED_HOOK(report_open, false, {
      std::vector<std::string> logs;
      std::string id;
      auto ok = mk::report::resubmit_measurement(
          sample_report, ".mkbuild/download/ca-bundle.pem", 30, logs, id);
      REQUIRE(!ok);
    });
  }

  SECTION("when the report ID is not JSON serializable") {
    std::string bad_id{(char *)binary_data.data(), binary_data.size()};
    MKMOCK_WITH_ENABLED_HOOK(report_id, bad_id, {
      std::vector<std::string> logs;
      std::string id;
      auto ok = mk::report::resubmit_measurement(
          sample_report, ".mkbuild/download/ca-bundle.pem", 30, logs, id);
      REQUIRE(!ok);
    });
  }

  SECTION("when we cannot submit a measurement") {
    MKMOCK_WITH_ENABLED_HOOK(report_submit_measurement_json, false, {
      std::vector<std::string> logs;
      std::string id;
      auto ok = mk::report::resubmit_measurement(
          sample_report, ".mkbuild/download/ca-bundle.pem", 30, logs, id);
      REQUIRE(!ok);
    });
  }
}
