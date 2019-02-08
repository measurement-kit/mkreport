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

#include "mkreport.hpp"

TEST_CASE("Basic example of integrating third party test") {
  mk::report::Report report;
  report.probe_asn = "AS30722";
  report.probe_cc = "IT";
  report.software_name = "mkreport-tests";
  report.software_version = "0.0.1";
  report.test_name = "dummy";
  report.test_version = "0.0.1";
  report.test_start_time = mk::report::ooni_date_now();
  report.collector_base_url = "https://collector-sandbox.ooni.io";
  report.ca_bundle_path = "ca-bundle.pem";
  std::vector<std::string> logs;
  auto ok = report.open(logs);
  for (auto &s : logs) std::clog << s << std::endl;
  REQUIRE(ok);
  REQUIRE(!report.id.empty());
  mk::report::Measurement measurement;
  measurement.start();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  measurement.stop();
  measurement.test_keys = R"({"success": true})";
  logs.clear();
  ok = report.submit_measurement(measurement, logs);
  for (auto &s : logs) std::clog << s << std::endl;
  REQUIRE(ok);
  logs.clear();
  ok = report.close(logs);
  for (auto &s : logs) std::clog << s << std::endl;
  REQUIRE(ok);
}

TEST_CASE("Make sure that we can autodiscover a collector") {
  mk::report::Report report;
  report.test_name = "dummy";
  report.test_version = "0.0.1";
  report.ca_bundle_path = "ca-bundle.pem";
  std::vector<std::string> logs;
  auto ok = report.autodiscover_collector(logs);
  for (auto &s : logs) std::clog << s << std::endl;
  REQUIRE(ok);
  std::clog << report.collector_base_url << std::endl;
  std::regex re{R"(^https://[a-z].collector.ooni.io:443$)"};
  std::smatch match;
  REQUIRE(std::regex_match(report.collector_base_url, match, re));
}

TEST_CASE("Make sure that we can autodiscover probe_asn and probe_cc") {
  mk::report::Report report;
  report.ca_bundle_path = "ca-bundle.pem";
  std::vector<std::string> logs;
  auto ok = report.autodiscover_probe_asn_probe_cc(
      "asn.mmdb", "country.mmdb", logs);
  for (auto &s : logs) std::clog << s << std::endl;
  REQUIRE(ok);
  std::clog << report.probe_asn << std::endl;
  {
    std::regex re{R"(^AS[0-9]{1,7}$)"};
    std::smatch match;
    REQUIRE(std::regex_match(report.probe_asn, match, re));
  }
  std::clog << report.probe_cc << std::endl;
  {
    std::regex re{R"(^[A-Z]{2}$)"};
    std::smatch match;
    REQUIRE(std::regex_match(report.probe_cc, match, re));
  }
}

static std::string to_resubmit = R"({
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
    "measurement_start_time": "2019-02-08 09:52:33",
    "options": [],
    "probe_asn": "AS15169",
    "probe_cc": "US",
    "probe_city": null,
    "probe_ip": "127.0.0.1",
    "report_id": "20190208T095233Z_AS15169_O986SVua4krXdAnMx3aGC83INNJAo1GTZII2OwBQx2H4Qx0LKA",
    "software_name": "measurement_kit",
    "software_version": "0.9.0",
    "test_helpers": {},
    "test_keys": {
        "client_resolver": "172.217.33.130",
        "connection": "success"
    },
    "test_name": "tcp_connect",
    "test_runtime": 0.039698123931884766,
    "test_start_time": "2019-02-08 09:52:31",
    "test_version": "0.1.0"
})";

TEST_CASE("Make sure that we can resubmit a measurement") {
  std::vector<std::string> logs;
  std::string id;
  auto ok = mk::report::resubmit_measurement(
        to_resubmit, "ca-bundle.pem", 30, logs, id);
  for (auto &s : logs) std::clog << s << std::endl;
  REQUIRE(ok);
  REQUIRE(!id.empty());
}
