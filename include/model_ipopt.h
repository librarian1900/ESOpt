#pragma once
#include "model_base.h"
#include "dylib.h"
#include "IpStdCInterface.h"
#include "variable_expression.h"
#include "constraint.h"
#include "nleval.hpp"
#include "nlexpr.hpp"
#include "tcc_interface.hpp"
#include "cppad_interface.hpp"
#include "codegen_c.hpp"

#include <map>
#include <vector>
#include <memory>


// For storing ExpressionGraph instances
#include <unordered_map>

struct IpoptProblemInfo;
using IpoptProblem = IpoptProblemInfo *;
using UserDataPtr = void *;
using ipnumber = double;
using ipindex = int;
using Bool = bool;
using IndexStyleEnum = int;
using SolverReturn = int;

using Eval_F_CB = Bool (*)(ipindex, ipnumber *, Bool, ipnumber *, UserDataPtr);
using Eval_G_CB = Bool (*)(ipindex, ipnumber *, Bool, ipindex, ipnumber *, UserDataPtr);
using Eval_Grad_F_CB = Bool (*)(ipindex, ipnumber *, Bool, ipnumber *, UserDataPtr);
using Eval_Jac_G_CB = Bool (*)(ipindex, ipnumber *, Bool, ipindex, ipindex, ipindex *, ipindex *, ipnumber *, UserDataPtr);
using Eval_H_CB = Bool (*)(ipindex, ipnumber *, Bool, ipnumber, ipindex, ipnumber *, Bool, ipindex, ipindex *, ipindex *, ipnumber *, UserDataPtr);


#define APILIST \
    B(CreateIpoptProblem); \
    B(FreeIpoptProblem);   \
    B(AddIpoptStrOption);  \
    B(AddIpoptNumOption);  \
    B(AddIpoptIntOption);  \
    B(IpoptSolve);

namespace ipopt
{
#define B DYLIB_EXTERN_DECLARE
APILIST
#undef B

bool is_library_loaded();
bool MODEL_API load_library(const std::string &path);
} // namespace ipopt

struct IpoptfreeproblemT
{
    void operator()(IpoptProblemInfo *model) const
    {
        ipopt::FreeIpoptProblem(model);
    };
};

struct IpoptResult
{
    bool is_valid = false;
    // store results
    std::vector<double> x, g, mult_g, mult_x_L, mult_x_U;
    double obj_val;
};

class MODEL_API IpoptModel : public ModelBase
{
public:
    IpoptModel();
    ~IpoptModel();

    void init();
    void close();

    VariableIndex add_variable(double lb, double ub, VariableDomain domain = VariableDomain::Continuous,
                               const char *name = nullptr) override;
    double get_variable_lb(const VariableIndex &variable);
    double get_variable_ub(const VariableIndex &variable);
    void set_variable_lb(const VariableIndex &variable, double lb);
    void set_variable_ub(const VariableIndex &variable, double ub);
    void set_variable_bounds(const VariableIndex &variable, double lb, double ub) override;

    double get_variable_start(const VariableIndex &variable);
    void set_variable_start(const VariableIndex &variable, double start);

    std::string get_variable_name(const VariableIndex &variable);
    void set_variable_name(const VariableIndex &variable, const std::string &name);

    double get_variable_value(const VariableIndex &variable) override;

    std::string pprint_variable(const VariableIndex &variable);

    double get_obj_value();
    int _constraint_internal_index(const ConstraintIndex &constraint);
    double get_constraint_primal(const ConstraintIndex &constraint);
    double get_constraint_dual(const ConstraintIndex &constraint);

    ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f, ConstraintSense sense, double rhs,
                                          const char *name = nullptr) override;
    ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f,
                                          const std::tuple<double, double> &interval,
                                          const char *name = nullptr) override;

    ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
                                             ConstraintSense sense, double rhs,
                                             const char *name = nullptr) override;
    ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
                                             const std::tuple<double, double> &interval,
                                             const char *name = nullptr);

    NLConstraintIndex add_nl_constraint(const NLExpr &expr, ConstraintSense sense, double rhs,
                                        const char *name = nullptr);
    NLConstraintIndex add_nl_constraint(const NLExpr &expr, const std::tuple<double, double> &interval,
                                        const char *name = nullptr);

    NLConstraintIndex add_nl_constraint(const ExpressionHandle &expr, ConstraintSense sense, double rhs,
                                        const char *name = nullptr) override;
    NLConstraintIndex add_nl_constraint(const ExpressionHandle &expr, const std::tuple<double, double> &interval,
                                        const char *name = nullptr);
    void set_objective(const ScalarAffineFunction &expr, ObjectiveSense sense) override;
    void set_objective(const ScalarQuadraticFunction &expr, ObjectiveSense sense);
    void set_objective(const ExprBuilder &expr, ObjectiveSense sense);

    void _set_linear_objective(const ScalarAffineFunction &expr);
    void _set_quadratic_objective(const ScalarQuadraticFunction &expr);

    // struct GraphInstancesInfo
    // {
    //     // hash of this graph instance
    //     std::vector<uint64_t> hashes;
    //     // index of this graph instance
    //     std::vector<int> instance_indices;

    //     size_t n_instances_since_last_aggregation;
    // } nl_constraint_info, nl_objective_info;

    // // length = n_graph_instances
    // struct GraphInstancesGroupInfo
    // {
    //     // which group it belongs to
    //     std::vector<int> group_indices;
    //     // the number in that group
    //     std::vector<int> group_orders;
    // } nl_constraint_group_info, nl_objective_group_info;

    // // graph groups
    // struct
    // {
    //     Hashmap<uint64_t, int> hash_to_group;
    //     size_t n_group = 0;
    //     std::vector<int> representative_graph_indices;
    //     std::vector<std::vector<int>> instance_indices;
    //     std::vector<AutodiffSymbolicStructure> autodiff_structures;
    //     std::vector<ConstraintAutodiffEvaluator> autodiff_evaluators;

    //     // where to store the hessian matrix, each group length = n_instance * local_hessian_nnz
    //     std::vector<std::vector<int>> hessian_indices;
    // } nl_constraint_groups;

    // struct
    // {
    //     Hashmap<uint64_t, int> hash_to_group;
    //     size_t n_group = 0;
    //     std::vector<int> representative_graph_indices;
    //     std::vector<std::vector<int>> instance_indices;
    //     std::vector<AutodiffSymbolicStructure> autodiff_structures;
    //     std::vector<ObjectiveAutodiffEvaluator> autodiff_evaluators;

    //     // where to store the gradient vector, each group length = n_instance * local_jacobian_nnz
    //     std::vector<std::vector<int>> gradient_indices;
    //     // where to store the hessian matrix, each group length = n_instance * local_hessian_nnz
    //     std::vector<std::vector<int>> hessian_indices;
    // } nl_objective_groups;

    void add_nl_objective(const ExpressionHandle &expr) override;
    void add_nl_objective(const NLExpr &expr);

    // void clear_nl_objective();

    void analyze_structure();
    void optimize() override;

    // load current solution as initial guess
    void load_current_solution();

    // set options
    void set_raw_option_int(const std::string &name, int value);
    void set_raw_option_double(const std::string &name, double value);
    void set_raw_option_string(const std::string &name, const std::string &value);

    /* Members */

    size_t n_variables = 0;

    size_t n_nl_constraints = 0;
    /*
     * record the constraint indices mapping from the monotonic one (the order of adding
     * constraint) to the reordered one (linear, quadratic, NL group 0 -> con0, con1 ,..., conN0, NL
     * group1 -> con0, con1,..., conN1)
     */

    IpoptResult m_result;
    bool m_is_dirty = true;
    enum ApplicationReturnStatus m_status;

    std::unique_ptr<IpoptProblemInfo, IpoptfreeproblemT> m_problem = nullptr;

    // ModelBase interface methods
    bool supports_model_attribute(ModelAttributeDouble attribute, bool changeable) override;
    bool supports_model_attribute(ModelAttributeInt attribute, bool changeable) override;
    bool supports_model_attribute(ModelAttributeString attribute, bool changeable) override;
    double get_model_attribute(ModelAttributeDouble attribute) override;
    int get_model_attribute(ModelAttributeInt attribute) override;
    std::string get_model_attribute(ModelAttributeString attribute) override;
    void set_model_attribute(ModelAttributeDouble attribute, double value) override;
    void set_model_attribute(ModelAttributeInt attribute, int value) override;
    void set_model_attribute(ModelAttributeString attribute, const char *value) override;

    bool supports_variable_attribute(VariableAttributeDouble attribute, bool changeable) override;
    bool supports_variable_attribute(VariableAttributeInt attribute, bool changeable) override;
    bool supports_variable_attribute(VariableAttributeString attribute, bool changeable) override;
    double get_variable_attribute(const VariableIndex &variable, VariableAttributeDouble attribute) override;
    int get_variable_attribute(const VariableIndex &variable, VariableAttributeInt attribute) override;
    std::string get_variable_attribute(const VariableIndex &variable, VariableAttributeString attribute) override;
    void set_variable_attribute(const VariableIndex &variable, VariableAttributeDouble attribute, double value) override;
    void set_variable_attribute(const VariableIndex &variable, VariableAttributeInt attribute, int value) override;
    void set_variable_attribute(const VariableIndex &variable, VariableAttributeString attribute, const char *value) override;

    bool supports_constraint_attribute(ConstraintAttributeDouble attribute, bool changeable) override;
    bool supports_constraint_attribute(ConstraintAttributeInt attribute, bool changeable) override;
    bool supports_constraint_attribute(ConstraintAttributeString attribute, bool changeable) override;
    double get_constraint_attribute(const ConstraintIndex &constraint, ConstraintAttributeDouble attribute) override;
    int get_constraint_attribute(const ConstraintIndex &constraint, ConstraintAttributeInt attribute) override;
    std::string get_constraint_attribute(const ConstraintIndex &constraint, ConstraintAttributeString attribute) override;
    void set_constraint_attribute(const ConstraintIndex &constraint, ConstraintAttributeDouble attribute, double value) override;
    void set_constraint_attribute(const ConstraintIndex &constraint, ConstraintAttributeInt attribute, int value) override;
    void set_constraint_attribute(const ConstraintIndex &constraint, ConstraintAttributeString attribute, const char *value) override;

    void set_parameter(const char *name, double value) override;
    void set_parameter(const char *name, int value) override;
    void set_parameter(const char *name, const char *value) override;
    void set_parameter();

    ObjectiveSense get_obj_sense();
    void set_obj_sense(ObjectiveSense sense);
    int _variable_index(const VariableIndex &variable);
    int _constraint_index(const ConstraintIndex &constraint);

private:
    TCCInstance tccInst;
    class Impl;
    std::unique_ptr<Impl> pImpl;

    int add_graph_index();
    void finalize_graph_instance(size_t graph_index, const ExpressionGraph &graph);
    int aggregate_nl_constraint_groups();
    int get_nl_constraint_group_representative(int group_index) const;
    int aggregate_nl_objective_groups();
    int get_nl_objective_group_representative(int group_index) const;

    void assign_nl_constraint_group_autodiff_structure(int group_index,
                                                       const AutodiffSymbolicStructure &structure);
    void assign_nl_constraint_group_autodiff_evaluator(
        int group_index, const ConstraintAutodiffEvaluator &evaluator);
    void assign_nl_objective_group_autodiff_structure(int group_index,
                                                      const AutodiffSymbolicStructure &structure);
    void assign_nl_objective_group_autodiff_evaluator(int group_index,
                                                      const ObjectiveAutodiffEvaluator &evaluator);

    ConstraintIndex add_single_nl_constraint(size_t graph_index, const ExpressionGraph &graph,
                                             double lb, double ub);
    void _find_similar_graphs();
    void _compile_evaluators();
    void _codegen_c();
    void _codegen_llvm();

    void _compute_objective_graph_hashes(std::vector<uint64_t> &hashes);
    void _compute_constraint_graph_hashes(std::vector<uint64_t> &hashes);


    friend bool eval_f(ipindex n, ipnumber *x, bool new_x, ipnumber *obj_value, UserDataPtr user_data);
    friend bool eval_grad_f(ipindex n, ipnumber *x, bool new_x, ipnumber *grad_f, UserDataPtr user_data);
    friend bool eval_g(ipindex n, ipnumber *x, bool new_x, ipindex m, ipnumber *g, UserDataPtr user_data);
    friend bool eval_jac_g(ipindex n, ipnumber *x, bool new_x, ipindex m, ipindex nele_jac,
                       ipindex *iRow, ipindex *jCol, ipnumber *values, UserDataPtr user_data);
    friend bool eval_h(ipindex n, ipnumber *x, bool new_x, ipnumber obj_factor, ipindex m,
                   ipnumber *lambda, bool new_lambda, ipindex nele_hess, ipindex *iRow,
                   ipindex *jCol, ipnumber *values, UserDataPtr user_data);

// private:
//     static Bool eval_f_cb(ipindex n, ipnumber *x, Bool new_x, ipnumber *obj_value, UserDataPtr user_data);
//     static Bool eval_grad_f_cb(ipindex n, ipnumber *x, Bool new_x, ipnumber *grad_f, UserDataPtr user_data);
//     static Bool eval_g_cb(ipindex n, ipnumber *x, Bool new_x, ipindex m, ipnumber *g, UserDataPtr user_data);
//     static Bool eval_jac_g_cb(ipindex n, ipnumber *x, Bool new_x, ipindex m, ipindex nele_jac,
//                               ipindex *iRow, ipindex *jCol, ipnumber *values, UserDataPtr user_data);
//     static bool eval_h_cb(ipindex n, ipnumber *x, bool new_x, ipnumber obj_factor,
//                           ipindex m, ipnumber *lambda, bool new_lambda,
//                           ipindex nele_hess, ipindex *iRow, ipindex *jCol,
//                           ipnumber *values, UserDataPtr user_data);
};
