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
