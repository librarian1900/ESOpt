#pragma once
#include <string>

#include "variable_expression.h"
#include "constraint.h"
#include "attribute.h"
#include "nlexpr.hpp"


class MODEL_API ModelBase
{
  public:
	virtual ~ModelBase() = default;

	// add variable
	virtual VariableIndex add_variable(double lb, double ub, VariableDomain domain = VariableDomain::Continuous,
	                                   const char *name = nullptr) = 0;

	// add linear constraint
	virtual ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f, ConstraintSense sense, double rhs,
	                                              const char *name = nullptr) = 0;

	virtual ConstraintIndex add_linear_constraint(const ScalarAffineFunction &f,
                                          const std::tuple<double, double> &interval,
                                          const char *name = nullptr) = 0;
	// add quadratic constraint
	virtual ConstraintIndex add_quadratic_constraint(const ScalarQuadraticFunction &f,
                                             ConstraintSense sense, double rhs,
                                             const char *name = nullptr) = 0;
	// add nonlinear constraint
	virtual NLConstraintIndex add_nl_constraint(const ExpressionHandle &expr,
												ConstraintSense sense, double rhs,
                                        		const char *name = nullptr) = 0;

	// set objective
	virtual void set_objective(const ScalarAffineFunction &expr, ObjectiveSense sense = ObjectiveSense::Minimize) = 0;

	// set nonlinear objective
	virtual void add_nl_objective(const ExpressionHandle &expr) = 0;
	// solve the optimization model
	virtual void optimize() = 0;

	// set variable bounds
	virtual void set_variable_bounds(const VariableIndex &variable, double lb, double ub) = 0;
	// get variable value
	virtual double get_variable_value(const VariableIndex &variable) = 0;

	// get and set attributes
	virtual bool supports_model_attribute(ModelAttributeDouble attribute, bool changeable) = 0;
	virtual bool supports_model_attribute(ModelAttributeInt attribute, bool changeable) = 0;
	virtual bool supports_model_attribute(ModelAttributeString attribute, bool changeable) = 0;
	virtual double get_model_attribute(ModelAttributeDouble attribute) = 0;
	virtual int get_model_attribute(ModelAttributeInt attribute) = 0;
	virtual std::string get_model_attribute(ModelAttributeString attribute) = 0;
	virtual void set_model_attribute(ModelAttributeDouble attribute, double value) = 0;
	virtual void set_model_attribute(ModelAttributeInt attribute, int value) = 0;
	virtual void set_model_attribute(ModelAttributeString attribute, const char *value) = 0;

	// The variable / constraint attribute APIs follow the same pattern; to be added
	virtual bool supports_variable_attribute(VariableAttributeDouble attribute, bool changeable) = 0;
	virtual bool supports_variable_attribute(VariableAttributeInt attribute, bool changeable) = 0;
	virtual bool supports_variable_attribute(VariableAttributeString attribute, bool changeable) = 0;
	virtual double get_variable_attribute(const VariableIndex& variable, VariableAttributeDouble attribute) = 0;
	virtual int get_variable_attribute(const VariableIndex& variable, VariableAttributeInt attribute) = 0;
	virtual std::string get_variable_attribute(const VariableIndex& variable, VariableAttributeString attribute) = 0;
	virtual void set_variable_attribute(const VariableIndex& variable, VariableAttributeDouble attribute, double value) = 0;
	virtual void set_variable_attribute(const VariableIndex& variable, VariableAttributeInt attribute, int value) = 0;
	virtual void set_variable_attribute(const VariableIndex& variable, VariableAttributeString attribute, const char *value) = 0;

	virtual bool supports_constraint_attribute(ConstraintAttributeDouble attribute, bool changeable) = 0;
	virtual bool supports_constraint_attribute(ConstraintAttributeInt attribute, bool changeable) = 0;
	virtual bool supports_constraint_attribute(ConstraintAttributeString attribute, bool changeable) = 0;
	virtual double get_constraint_attribute(const ConstraintIndex& constraint, ConstraintAttributeDouble attribute) = 0;
	virtual int get_constraint_attribute(const ConstraintIndex& constraint, ConstraintAttributeInt attribute) = 0;
	virtual std::string get_constraint_attribute(const ConstraintIndex& constraint, ConstraintAttributeString attribute) = 0;
	virtual void set_constraint_attribute(const ConstraintIndex& constraint, ConstraintAttributeDouble attribute, double value) = 0;
	virtual void set_constraint_attribute(const ConstraintIndex& constraint, ConstraintAttributeInt attribute, int value) = 0;
	virtual void set_constraint_attribute(const ConstraintIndex& constraint, ConstraintAttributeString attribute, const char *value) = 0;
	
	// set solver parameters
	virtual void set_parameter(const char *name, double value) = 0;
	virtual void set_parameter(const char *name, int value) = 0;
	virtual void set_parameter(const char *name, const char *value) = 0;
};