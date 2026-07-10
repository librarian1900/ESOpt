#ifndef OPF_SOLVER_H
#define OPF_SOLVER_H

#include <string>
#include "opf_data.h"
#include "model_ipopt.h"

// Benchmark result structure
struct BenchmarkResult {
    double total_time;
    double code_gen_time;
    double objective_value;
    int termination_status;
    std::string log_path;
};

// Load OPF data from JSON file
OpfData load_opf_data(const std::string& filename);

// Solve OPF in polar coordinates
void solve_opf(IpoptModel& model, const OpfData& data);

// acopf_main - returns benchmark results
//   case_path: full path to a JSON case file (e.g. "./data/json/case5.json")
BenchmarkResult acopf_main(const std::string& logdir,
                           const std::string& case_path);

#endif // OPF_SOLVER_H
