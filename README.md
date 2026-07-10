# ESOpt SDK

## 1. ESOpt Introduction

ESOpt is a high-performance nonlinear programming (NLP) solver SDK
for C++ applications, distributed as a dynamic library:

- **Core library**: [`libOptInterface.dll`](lib/libOptInterface.dll) (Windows) /
  [`libOptInterface.so`](lib/libOptInterface.so) (Linux). It wraps the IPOPT kernel and exposes an
  object-oriented modeling API on top.
- **Bundled sample**: the [`example/`](example/) subdirectory ships an AC
  optimal power flow (AC-OPF) benchmark program
  [`acopf_test`](example/src/acopf_test.cpp), used for functional demo and
  performance comparison; it is also the fastest onboarding reference for new
  users.

ESOpt targets users familiar with C++ and basic mathematical optimization who
want to integrate a modern NLP kernel into their own engineering / simulation
code.

---

## 2. Usage

The following sections give the complete steps, from environment prep to
running the sample, under both **Windows** and **Linux** deployment shapes.

Case data is generated from https://github.com/metab0t/opf_benchmark.
Please refer to the original repository for details on the data.

### 2.1 Windows (MinGW)

#### 2.1.1 Build & runtime dependencies

Confirm the following are installed before building; if a missing package is
reported during linking / compilation, look it up in this table.

| Dependency | Version / Source | Required |
|---|---|---|
| MinGW-w64 GCC | 15.2.0 posix-seh-ucrt-rt_v13 ([official prebuilt 7z](https://github.com/niXman/mingw-builds-binaries/releases/download/15.2.0-rt_v13-rev0/x86_64-15.2.0-release-posix-seh-ucrt-rt_v13-rev0.7z)) | ✅ |
| CMake | ≥ 3.10 | ✅ |
| `make` | Bundled with MinGW (`mingw32-make`) | ✅ |
| IPOPT prebuilt package | [≥ 3.14.19](https://github.com/coin-or/Ipopt/releases/download/releases%2F3.14.19/Ipopt-3.14.19-win64-msvs2022-md.zip). After unpack to any local directory as <IPOPT root>, **must** set `$env:IPOPT_LIB = <IPOPT root>\bin\ipopt-3.dll` to point at the DLL directly and ensure `<IPOPT root>\bin` is in the system `%PATH%` | ✅ |  
| TinyCC | [≥ 0.9.28rc](https://github.com/TinyCC/tinycc/commit/98765e5ebc04ea464195fa80ea5e4bbdc70a29cc); required for codegen. After build && install **must** set `$env:LIBTINYCC_DIR` | ✅ |
| HSL (MA27) | Obtain an academic license separately | ⚠️ Optional; Default setting is `mumps` |

#### 2.1.2 Build example

```powershell
cd <repo>\example
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

Output: `<repo>\example\build\bin\acopf_test.exe`, with `libOptInterface.dll`
auto-copied next to it. Cache folder `POICache`.

The `POICache` cache directory is created by CMake and is essential for test case execution. If this directory is not present, manual creation or a fresh CMake run is required.

#### 2.1.3 Runtime example

Before running, only the `IPOPT_LIB` and `LIBTINYCC_DIR` environment variables need to be
configured (see the section on build & runtime dependencies).

```powershell
$env:IPOPT_LIB    = "<IPOPT root>\bin\ipopt-3.dll"
$env:LIBTINYCC_DIR = "<TinyCC root>"

cd <repo>\example\build\bin
.\acopf_test.exe ..\..\data\json\pglib_opf_case5_pjm.json
```

Expected output (measured on `pglib_opf_case5_pjm`):

```
Loading IPOPT library: <IPOPT root>\bin\ipopt-3.dll
Loading data from: ..\..\data\json\pglib_opf_case5_pjm.json
Creating IPOPT model...
...
EXIT: Optimal Solution Found.
Termination status: 0
Objective value: 17551.9
```

Other cases you can run: `pglib_opf_case1354_pegase.json` /
`pglib_opf_case2000_goc.json`.

---

### 2.2 Linux (GCC)

> Applies to: `x86_64` Linux on Ubuntu 22.04+ / Debian 12+ family.
> For other distributions, replace `apt-get` with the equivalent `dnf` /
> `pacman` commands.

#### 2.2.1 Build & runtime dependencies

```bash
sudo apt-get install -y g++ cmake coinor-libipopt-dev libtcc-dev
```

| Dependency | Source | Required |
|---|---|---|
| `g++` ≥ 11 | `apt-get install g++` | ✅ |
| `cmake` ≥ 3.16 | `apt-get install cmake` | ✅ |
| IPOPT runtime | `apt-get coinor-libipopt-dev` | ✅ |
| TinyCC | `apt-get libtcc-dev` | ✅ |
| HSL (MA27) | Obtain an academic license separately | ⚠️ Optional; Default setting is `mumps` |

#### 2.2.2 Build example

```bash
cd <repo>/example
rm -rf build
cmake -S . -B build -G "Unix Makefiles"
cmake --build build -j"$(nproc)"
```

Output: `<repo>/example/build/bin/acopf_test`.

#### 2.2.3 Run example

```bash
cd <repo>/example
./build/bin/acopf_test data/json/pglib_opf_case5_pjm.json
```

Expected output (IPOPT 3.11.x):

```
Loading IPOPT library: /usr/lib/libipopt.so.1
Loading data from: ../../data/json/pglib_opf_case5_pjm.json
Creating IPOPT model...
...
EXIT: Optimal Solution Found.
Termination status: 0
Objective value: 17551.9
Total time: ... seconds
```

Other cases you can run: `pglib_opf_case1354_pegase.json` /
`pglib_opf_case2000_goc.json`.

---

## 3. Issues & Discussion

- **Issue / Bug reports**: open one in the GitHub Issues tab, ideally including
  - OS / compiler version (`g++ --version`)
  - Output of the `message(STATUS ...)` lines from `CMakeLists.txt`
  - Full output of `acopf_test.exe ... 2>&1 | tee run.log`
- **Discussion / Experience sharing**:
  - GitHub Discussions "Q&A" and "General" boards
  - Contact: `libr4rian@outlook.com`

