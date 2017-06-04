#include "equiv.h"

#include <iostream>
using namespace std;

EquivClass::~EquivClass()
{
    for(int i = 0; i < exps.size(); i++)
        delete exps[i];
}

vector<Expression*> EquivClass::getExpressions()
{ return exps; }

void EquivClass::append(Expression* exp)
{
    for(int i = 0; i < exps.size(); i++)
        if(exps[i]->equals(exp))
            return;
    exps.push_back(new Expression(exp));
}

void EquivClass::append(EquivClass* cl)
{
    for(int i = 0; i < cl->exps.size(); i++)
        append(cl->exps[i]);
}

bool EquivClass::equiv(Expression* exp)
{
    for(int i = 0; i < exps.size(); i++)
        if(exps[i]->equals(exp))
            return true;
    return false;
}

Partition::~Partition()
{
    for(int i = 0; i < classes.size(); i++)
        delete classes[i];
}

vector<EquivClass*> Partition::getClasses()
{ return classes; }

vector<EquivClass*> Partition::getEquiv(Expression* exp)
{
    vector<EquivClass*> res;
    for(int i = 0; i < classes.size(); i++)
    {
        vector<Expression*> exps = classes[i]->getExpressions();
        for(int j = 0; j < exps.size(); j++)
            if(exps[j]->match(exp))
            {
                res.push_back(classes[i]);
                break;
            }
    }
    return res;
}

void Partition::append(Expression* exp1, Expression* exp2)
{
    if(exp1->equals(exp2))
        return;
    std::vector<EquivClass*> all;
    std::vector<int> all_i;
    for(int i = 0; i < classes.size(); i++)
        if(classes[i]->equiv(exp1) || classes[i]->equiv(exp2))
        {
            all.push_back(classes[i]);
            all_i.push_back(i);
        }
    if(!all.size())
    {
        EquivClass* cl = new EquivClass();;
        cl->append(exp1);
        cl->append(exp2);
        classes.push_back(cl);
    }
    else
    {
        all[0]->append(exp1);
        all[0]->append(exp2);
        for(int i = 1; i < all.size(); i++)
            all[0]->append(all[i]);
        for(int i = all_i.size() - 1; i > 0; i--)
        {
            delete classes[all_i[i]];
            classes.erase(classes.begin() + all_i[i]);
        }
    }
}
