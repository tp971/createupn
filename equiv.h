#ifndef EQUIV_H_INCLUDED
#define EQUIV_H_INCLUDED

#include "expression.h"
#include <vector>

class EquivClass
{
private:
    std::vector<Expression*> exps;
public:
    ~EquivClass();
    std::vector<Expression*> getExpressions();
    void append(Expression* exp);
    void append(EquivClass* cl);
    bool equiv(Expression* exp);
};

class Partition
{
private:
    std::vector<EquivClass*> classes;
public:
    ~Partition();
    std::vector<EquivClass*> getClasses();
    std::vector<EquivClass*> getEquiv(Expression* exp);
    void append(Expression* exp1, Expression* exp2);
};

#endif //EQUIV_H_INCLUDED
