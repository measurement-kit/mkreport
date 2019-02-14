# Measurement Kit OONI report library

[![GitHub license](https://img.shields.io/github/license/measurement-kit/mkreport.svg)](https://raw.githubusercontent.com/measurement-kit/mkreport/master/LICENSE) [![Github Releases](https://img.shields.io/github/release/measurement-kit/mkreport.svg)](https://github.com/measurement-kit/mkreport/releases) [![Build Status](https://img.shields.io/travis/measurement-kit/mkreport/master.svg?label=travis)](https://travis-ci.org/measurement-kit/mkreport) [![codecov](https://codecov.io/gh/measurement-kit/mkreport/branch/master/graph/badge.svg)](https://codecov.io/gh/measurement-kit/mkreport) [![Build status](https://img.shields.io/appveyor/ci/bassosimone/mkreport/master.svg?label=appveyor)](https://ci.appveyor.com/project/bassosimone/mkreport/branch/master)

This library allows you to:

1. autodiscover probe ASN, probe CC, collector, etc.;

2. open reports, submit measurements, and close reports;

3. resubmit unsubmitted reports.

The possibility of managing the lifecycle of a report allows us to have
external nettests, such as Psiphon.

The main use case of this library is be vendored into MK sources and
built along with MK.

Therefore, we don't guarantee a stable API.

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
