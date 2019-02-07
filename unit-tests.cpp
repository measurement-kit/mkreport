#include <chrono>
#include <iostream>
#include <thread>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

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

#define MKREPORT_INLINE_IMPL
#include "mkreport.hpp"

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
}

TEST_CASE("autodiscover_probe_asn_probe_cc works correctly") {
  SECTION("with an invalid ASN DB") {
    mk::report::Report report;
    std::vector<std::string> logs;
    REQUIRE(!report.autodiscover_probe_asn_probe_cc(
          "nonexistent.mmdb", "country.mmdb", logs));
  }

  SECTION("with an invalid country DB") {
    mk::report::Report report;
    std::vector<std::string> logs;
    REQUIRE(!report.autodiscover_probe_asn_probe_cc(
          "asn.mmdb", "nonexistent.mmdb", logs));
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
    std::vector<uint8_t> v{0xf0, 0x90, 0x28, 0xbc};
    mk::report::Report report;
    report.probe_asn = std::string{(char *)v.data(), v.size()};
    std::vector<std::string> logs;
    REQUIRE(!report.open(logs));
  }
}
