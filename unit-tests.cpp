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
              "asn.mmdb", "country.mmdb", logs));
    });
  }

  SECTION("with an invalid ASN DB") {
    mk::report::Report report;
    std::vector<std::string> logs;
    REQUIRE(!report.autodiscover_probe_asn_probe_cc(
          "nonexistent.mmdb", "country.mmdb", logs));
  }

  SECTION("with an error while querying the ASN DB") {
    MKMOCK_WITH_ENABLED_HOOK(mmdb_asn_lookup, false, {
      mk::report::Report report;
      std::vector<std::string> logs;
      REQUIRE(!report.autodiscover_probe_asn_probe_cc(
              "asn.mmdb", "country.mmdb", logs));
    });
  }

  SECTION("with an invalid country DB") {
    mk::report::Report report;
    std::vector<std::string> logs;
    REQUIRE(!report.autodiscover_probe_asn_probe_cc(
          "asn.mmdb", "nonexistent.mmdb", logs));
  }

  SECTION("with an error while querying the country DB") {
    MKMOCK_WITH_ENABLED_HOOK(mmdb_cc_lookup, false, {
      mk::report::Report report;
      std::vector<std::string> logs;
      REQUIRE(!report.autodiscover_probe_asn_probe_cc(
              "asn.mmdb", "country.mmdb", logs));
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
