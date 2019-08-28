# Measurement Kit OONI report library

⚠️⚠️⚠️⚠️⚠️⚠️⚠️: This repository is unused and has been archived.

[![GitHub license](https://img.shields.io/github/license/measurement-kit/mkreport.svg)](https://raw.githubusercontent.com/measurement-kit/mkreport/master/LICENSE) [![Github Releases](https://img.shields.io/github/release/measurement-kit/mkreport.svg)](https://github.com/measurement-kit/mkreport/releases) [![Build Status](https://img.shields.io/travis/measurement-kit/mkreport/master.svg?label=travis)](https://travis-ci.org/measurement-kit/mkreport) [![codecov](https://codecov.io/gh/measurement-kit/mkreport/branch/master/graph/badge.svg)](https://codecov.io/gh/measurement-kit/mkreport) [![Build status](https://img.shields.io/appveyor/ci/bassosimone/mkreport/master.svg?label=appveyor)](https://ci.appveyor.com/project/bassosimone/mkreport/branch/master)

This library allows you to generate measurement results as serialized JSON
documents. In turn, this allows you to generate OONI compliant reports
for tests that are not included in Measurement Kit. The first test that
we'll write using this library will be a Psiphon test.

The main use case of this library is be vendored into MK sources and
built along with MK. Therefore, we don't guarantee a stable API.

## Regenerating build files

Possibly edit `MKBuild.yaml`, then run:

```
go get -v github.com/measurement-kit/mkbuild
mkbuild
```

## Building

```
mkdir build
cd build
cmake -GNinja ..
cmake --build .
ctest -a -j8 --output-on-failure
```

## Testing with docker

```
./docker.sh <build-type>
```
