#include "model_base.h"
#include "model_ipopt.h"
#include "variable_expression.h"
#include "opf_data.h"
#include "opf_solver.h"
#include "timing_stats.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

// Include nlohmann/json library (single header)
// Make sure to download nlohmann/json.hpp to the same directory or include path
#include "nlohmann/json.hpp"

// #define LIBHSL_ENABLED

using json = nlohmann::json;
using namespace std;

const double PI = 3.14159265358979323846;

OpfData load_opf_data(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open JSON file: " + filename);
    }

    json data;
    file >> data;

    OpfData opf;

    // Load buses
    for (const auto& b : data["bus"]) {
        opf.bus.push_back({
            b["i"],
            b["j"],
            b["pd"],
            b["gs"],
            b["qd"],
            b["bs"]
        });
    }

    // Load generators
    for (const auto& g : data["gen"]) {
        opf.gen.push_back({
            g["i"],
            g["j"],
            g["cost1"],
            g["cost2"],
            g["cost3"],
            g["bus"]
        });
    }

    // Load arcs
    for (const auto& a : data["arc"]) {
        opf.arc.push_back({
            a["i"],
            a["rate_a"],
            a["bus"]
        });
    }

    // Load branches
    for (const auto& b : data["branch"]) {
        opf.branch.push_back({
            b["i"],
            b["j"],
            b["f_idx"],
            b["t_idx"],
            b["f_bus"],
            b["t_bus"],
            b["g"],
            b["b"],
            b["g_fr"],
            b["b_fr"],
            b["g_to"],
            b["b_to"],
            b["tr"],
            b["ti"],
            b["c1"], b["c2"], b["c3"], b["c4"],
            b["c5"], b["c6"], b["c7"], b["c8"],
            b["rate_a_sq"]
        });
    }

    // Load ref buses
    for (const auto& r : data["ref_buses"]) {
        opf.ref_buses.push_back(r);
    }

    // Load vectors
    opf.vmax = data["vmax"].get<vector<double>>();
    opf.vmin = data["vmin"].get<vector<double>>();
    opf.vmax_sq = data["vmax_sq"].get<vector<double>>();
    opf.vmin_sq = data["vmin_sq"].get<vector<double>>();
    opf.pmax = data["pmax"].get<vector<double>>();
    opf.pmin = data["pmin"].get<vector<double>>();
    opf.qmax = data["qmax"].get<vector<double>>();
    opf.qmin = data["qmin"].get<vector<double>>();
    opf.rate_a = data["rate_a"].get<vector<double>>();
    opf.angmax = data["angmax"].get<vector<double>>();
    opf.angmin = data["angmin"].get<vector<double>>();

    return opf;
}

void solve_opf(IpoptModel& model, const OpfData& data) {
    size_t Nbus = data.bus.size();
    vector<VariableIndex> va(Nbus);
    for (size_t i = 0; i < Nbus; ++i) {
        va[i] = model.add_variable(-1e20, 1e20, VariableDomain::Continuous);
    }

    vector<VariableIndex> vm(Nbus);
    for (size_t i = 0; i < Nbus; ++i) {
        vm[i] = model.add_variable(data.vmin[i], data.vmax[i], VariableDomain::Continuous);
        model.set_variable_start(vm[i], 1.0);
    }

    size_t Ngen = data.gen.size();
    vector<VariableIndex> pg(Ngen);
    vector<VariableIndex> qg(Ngen);
    for (size_t i = 0; i < Ngen; ++i) {
        pg[i] = model.add_variable(data.pmin[i], data.pmax[i], VariableDomain::Continuous);
        qg[i] = model.add_variable(data.qmin[i], data.qmax[i], VariableDomain::Continuous);
    }

    size_t Narc = data.arc.size();
    vector<VariableIndex> p(Narc);
    vector<VariableIndex> q(Narc);
    for (size_t i = 0; i < Narc; ++i) {
        p[i] = model.add_variable(-data.rate_a[i], data.rate_a[i], VariableDomain::Continuous);
        q[i] = model.add_variable(-data.rate_a[i], data.rate_a[i], VariableDomain::Continuous);
    }

    // Objective function
    ExprBuilder cost;
    for (const auto& g : data.gen) {
        size_t i = g.i - 1;
        cost += g.cost1 * pg[i] * pg[i] + g.cost2 * pg[i] + g.cost3;
    }
    model.set_objective(cost, ObjectiveSense::Minimize);

    // Reference bus constraints
    for (int i : data.ref_buses) {
        model.add_linear_constraint(1.0 * va[i - 1], ConstraintSense::Equal, 0.0);
    }

    // Branch flow constraints (nonlinear)
    for (const auto& b : data.branch) {
        ExpressionGraphContextGuard guard;

        int f_idx = b.f_idx - 1;
        int t_idx = b.t_idx - 1;
        int f_bus = b.f_bus - 1;
        int t_bus = b.t_bus - 1;

        auto p_from = p[f_idx];
        auto q_from = q[f_idx];
        auto p_to = p[t_idx];
        auto q_to = q[t_idx];
        auto vm_from = vm[f_bus];
        auto vm_to = vm[t_bus];
        auto va_from = va[f_bus];
        auto va_to = va[t_bus];

        double c1 = b.c1, c2 = b.c2, c3 = b.c3, c4 = b.c4;
        double c5 = b.c5, c6 = b.c6, c7 = b.c7, c8 = b.c8;

        auto va_delta = va_from - va_to;
        auto sin_ft = sin(va_delta);
        auto cos_ft = cos(va_delta);
        auto sin_tf = -sin_ft;
        auto cos_tf = cos_ft;

        auto pij = p_from - c5 * vm_from * vm_from
                  - c3 * vm_from * vm_to * cos_ft
                  - c4 * vm_from * vm_to * sin_ft;
        model.add_nl_constraint(pij, ConstraintSense::Equal, 0.0);

        auto qij = q_from + c6 * vm_from * vm_from
                  + c4 * vm_from * vm_to * cos_ft
                  - c3 * vm_from * vm_to * sin_ft;
        model.add_nl_constraint(qij, ConstraintSense::Equal, 0.0);

        auto pji = p_to - c7 * vm_to * vm_to
                  - c1 * vm_from * vm_to * cos_tf
                  - c2 * vm_from * vm_to * sin_tf;
        model.add_nl_constraint(pji, ConstraintSense::Equal, 0.0);

        auto qji = q_to + c8 * vm_to * vm_to
                  + c2 * vm_from * vm_to * cos_tf
                  - c1 * vm_from * vm_to * sin_tf;
        model.add_nl_constraint(qji, ConstraintSense::Equal, 0.0);
    }

    // Angle difference constraints and apparent power constraints
    for (size_t i = 0; i < data.branch.size(); ++i) {
        const auto& b = data.branch[i];
        int f_bus = b.f_bus - 1;
        int t_bus = b.t_bus - 1;
        int f_idx = b.f_idx - 1;
        int t_idx = b.t_idx - 1;

        model.add_linear_constraint(va[f_bus] - va[t_bus],
                                    make_tuple(data.angmin[i], data.angmax[i]));

        model.add_quadratic_constraint(p[f_idx] * p[f_idx] + q[f_idx] * q[f_idx],
                                       ConstraintSense::LessEqual, b.rate_a_sq);
        model.add_quadratic_constraint(p[t_idx] * p[t_idx] + q[t_idx] * q[t_idx],
                                       ConstraintSense::LessEqual, b.rate_a_sq);
    }

    // Bus balance constraints
    vector<ExprBuilder> p_balance_expr(Nbus);
    vector<ExprBuilder> q_balance_expr(Nbus);

    for (size_t i = 0; i < Nbus; ++i) {
        const auto& bus = data.bus[i];
        p_balance_expr[i] += bus.pd + bus.gs * vm[i] * vm[i];
        q_balance_expr[i] += bus.qd - bus.bs * vm[i] * vm[i];
    }

    for (const auto& a : data.arc) {
        int bus_idx = a.bus - 1;
        int i = a.i - 1;
        p_balance_expr[bus_idx] += p[i];
        q_balance_expr[bus_idx] += q[i];
    }

    for (const auto& g : data.gen) {
        int bus_idx = g.bus - 1;
        int i = g.i - 1;
        p_balance_expr[bus_idx] -= pg[i];
        q_balance_expr[bus_idx] -= qg[i];
    }

    for (size_t i = 0; i < Nbus; ++i) {
        model.add_quadratic_constraint(p_balance_expr[i], ConstraintSense::Equal, 0.0);
        model.add_quadratic_constraint(q_balance_expr[i], ConstraintSense::Equal, 0.0);
    }

    // Solve
    model.optimize();
}

// acopf_main - returns benchmark results without writing logs
//   case_path: full path to the JSON case file
BenchmarkResult esopt_main(const string& logdir,
                           const string& case_path) {
    BenchmarkResult result;

    auto sep_pos = case_path.find_last_of("/\\");
    string basename = (sep_pos == string::npos)
                          ? case_path
                          : case_path.substr(sep_pos + 1);
    auto dot_pos = basename.find_last_of('.');
    string log_name = (dot_pos == string::npos) ? basename : basename.substr(0, dot_pos);
    result.log_path = logdir + "/" + log_name + ".log";
    result.code_gen_time = 0.0;

    string json_path = case_path;
    while (!json_path.empty() && (json_path.back() == '/' || json_path.back() == '\\')) {
        json_path.pop_back();
    }
    cout << "Loading data from: " << json_path << endl;
    OpfData data = load_opf_data(json_path);

    cout << "Creating IPOPT model..." << endl;
    // Directly create and configure model in this function
    IpoptModel model;
    model.set_parameter("print_timing_statistics", "yes");
#ifdef LIBHSL_ENABLED
    model.set_parameter("hsllib", "libhsl.dll");
    model.set_parameter("linear_solver", "ma27");
#else
    model.set_parameter("linear_solver", "mumps");
#endif
    model.set_parameter("max_iter", 800);
    // Set output file to log solver messages
    model.set_parameter("output_file", result.log_path.c_str());

    TimingStats::get_instance().reset();
    TimingStats::get_instance().set_enabled(true);

    auto start = chrono::high_resolution_clock::now();
    solve_opf(model, data);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    result.total_time = duration.count();
    result.code_gen_time = TimingStats::get_instance().get_total_time();

    result.termination_status = model.get_model_attribute(ModelAttributeInt::TerminationStatus);
    result.objective_value = model.get_model_attribute(ModelAttributeDouble::ObjectiveValue);

    cout << "Termination status: " << result.termination_status << endl;
    cout << "Objective value: " << result.objective_value << endl;
    cout << "Total time: " << result.total_time << " seconds" << endl;

    return result;
}

// Standalone main function for direct execution
int main(int argc, char* argv[]) {
    // IPOPT shared library location comes from $IPOPT_LIB.
    // On Linux it is typically /usr/lib/libipopt.so.1; on Windows it is
    // typically <IPOPT_HOME>\bin\ipopt-3.dll. Hard-coded here as a fallback
    // so the path lookup stays single-variable.
    const char* env = std::getenv("IPOPT_LIB");
    std::string lib_ipopt = (env && std::string(env).size() > 0)
                                ? std::string(env)
#if defined(_WIN32)
                                : std::string("ipopt-3.dll");
#else
                                : std::string("libipopt.so.1");
#endif
    cout << "Loading IPOPT library: " << lib_ipopt << endl;
    ipopt::load_library(lib_ipopt);

    // Parse arguments
    string case_path = "./data/json/pglib_opf_case5_pjm.json";

    if (argc >= 2) {
        case_path = argv[1];
    }
    BenchmarkResult result = esopt_main(".", case_path);
    return 0;
}
