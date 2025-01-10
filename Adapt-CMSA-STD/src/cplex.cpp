#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        IloModel model(env);
        IloNumVarArray vars(env);
        vars.add(IloNumVar(env, 0.0, 40.0));
        vars.add(IloNumVar(env));
        vars.add(IloNumVar(env));
        model.add(IloMaximize(env, vars[0] + 2 * vars[1] + 3 * vars[2]));
        model.add(-vars[0] + vars[1] + vars[2] <= 20);
        model.add(vars[0] - 3 * vars[1] + vars[2] <= 30);
        IloCplex cplex(model);
        if (!cplex.solve()) {
            env.error() << "无法优化线性规划问题。" << std::endl;
            throw(-1);
        }
        IloNumArray vals(env);
        env.out() << "解决方案状态 = " << cplex.getStatus() << std::endl;
        env.out() << "目标函数值 = " << cplex.getObjValue() << std::endl;
        cplex.getValues(vals, vars);
        env.out() << "变量值 = " << vals << std::endl;
    } catch (IloException& e) {
        std::cerr << "捕获到异常: " << e << std::endl;
    } catch (...) {
        std::cerr << "捕获到未知异常" << std::endl;
    }
    env.end();
    return 0;
}
