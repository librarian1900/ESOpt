#ifndef OPF_DATA_H
#define OPF_DATA_H

#include <vector>
#include <string>

struct BusData {
    int i;
    int j;
    double pd;
    double gs;
    double qd;
    double bs;
};

struct GenData {
    int i;
    int j;
    double cost1;
    double cost2;
    double cost3;
    int bus;
};

struct ArcData {
    int i;
    double rate_a;
    int bus;
};

struct BranchData {
    int i;
    int j;
    int f_idx;
    int t_idx;
    int f_bus;
    int t_bus;
    double g;
    double b;
    double g_fr;
    double b_fr;
    double g_to;
    double b_to;
    double tr;
    double ti;
    double c1, c2, c3, c4, c5, c6, c7, c8;
    double rate_a_sq;
};

struct OpfData {
    std::vector<BusData> bus;
    std::vector<GenData> gen;
    std::vector<ArcData> arc;
    std::vector<BranchData> branch;
    std::vector<int> ref_buses;
    std::vector<double> vmax;
    std::vector<double> vmin;
    std::vector<double> vmax_sq;
    std::vector<double> vmin_sq;
    std::vector<double> pmax;
    std::vector<double> pmin;
    std::vector<double> qmax;
    std::vector<double> qmin;
    std::vector<double> rate_a;
    std::vector<double> angmax;
    std::vector<double> angmin;
};

OpfData load_opf_data(const std::string& filename);

#endif // OPF_DATA_H
