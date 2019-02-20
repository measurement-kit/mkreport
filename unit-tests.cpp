#include "mkreport.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "json.hpp"

// clang-format off
const uint8_t binary_input[] = {
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

TEST_CASE("we can prepare a report and serialize measurements") {
  const auto measurement = []() {
    auto begin = mk::report::monotonic_seconds_now();
    mk::report::Report report;
    report.annotations["unit_tests_run"] = "true";
    report.probe_asn = "AS30722";
    report.probe_cc = "IT";
    report.software_name = "neubot";
    report.software_version = "1.2.345";
    report.test_helpers["backend"] = "https://1.2.3.4:5678/";
    report.test_name = "simple";
    report.test_start_time = mk::report::ooni_date_now();
    report.test_version = "0.1.2";
    mk::report::Measurement measurement;
    measurement.input = "http://www.example.com/";
    measurement.report = report;
    measurement.start_time = mk::report::ooni_date_now();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    measurement.runtime = mk::report::monotonic_seconds_now() - begin;
    measurement.test_keys = R"({"success": true})";
    return measurement;
  }();

  SECTION("when there are no errors") {
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(measurement, json_str, logs);
    REQUIRE(ok);
    REQUIRE(!json_str.empty());
    REQUIRE(logs.empty());
    auto doc = nlohmann::json::parse(json_str);
    REQUIRE(doc.at("annotations").at("unit_tests_run") == "true");
    REQUIRE(doc.at("probe_asn") == "AS30722");
    REQUIRE(doc.at("probe_cc") == "IT");
    REQUIRE(doc.at("software_name") == "neubot");
    REQUIRE(doc.at("software_version") == "1.2.345");
    REQUIRE(doc.at("test_helpers").at("backend") == "https://1.2.3.4:5678/");
    REQUIRE(doc.at("test_name") == "simple");
    REQUIRE(!doc.at("test_start_time").empty());
    REQUIRE(doc.at("test_version") == "0.1.2");
    REQUIRE(doc.at("input") == "http://www.example.com/");
    REQUIRE(!doc.at("measurement_start_time").empty());
    REQUIRE(doc.at("test_runtime") > 0.3);
    REQUIRE(doc.at("test_keys").at("success") == true);
    std::clog << doc.dump(2) << std::endl;
  }

  SECTION("when probe_asn is empty") {
    auto m = measurement;
    m.report.probe_asn = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when probe_cc is empty") {
    auto m = measurement;
    m.report.probe_cc = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when software_name is empty") {
    auto m = measurement;
    m.report.software_name = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when software_version is empty") {
    auto m = measurement;
    m.report.software_version = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when test_name is empty") {
    auto m = measurement;
    m.report.test_name = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when test_start_time is empty") {
    auto m = measurement;
    m.report.test_start_time = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when test_version is empty") {
    auto m = measurement;
    m.report.test_version = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when measurement_start_time is empty") {
    auto m = measurement;
    m.start_time = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when test_keys is empty") {
    auto m = measurement;
    m.test_keys = "";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when test_keys is not a valid JSON") {
    auto m = measurement;
    m.test_keys = "{";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when test_keys is not an object") {
    auto m = measurement;
    m.test_keys = "[]";
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }

  SECTION("when we cannot dump a JSON") {
    auto m = measurement;
    m.report.software_name = std::string{
        (char *)binary_input, sizeof(binary_input)};
    std::string json_str;
    std::string logs;
    auto ok = mk::report::dump(m, json_str, logs);
    REQUIRE(!ok);
  }
}
