#include "expression.h"

using namespace std;

Expression::Expression(ExpType type, int var, Expression* a, Expression* b)
    :type(type), var(var)
{
    subexp[0] = a;
    subexp[1] = b;
    clean = (type != TRUE) && (type != FALSE);
    size = 1;
    if(subexp[0])
    {
        if(!subexp[0]->clean)
            clean = false;
        size += subexp[0]->size;
    }
    if(subexp[1])
    {
        if(!subexp[1]->clean)
            clean = false;
        size += subexp[1]->size;
    }
}

Expression::Expression(Expression* exp)
{
    type = exp->type;
    var = exp->var;
    subexp[0] = 0;
    subexp[1] = 0;
    clean = (type != TRUE) && (type != FALSE);
    size = 1;

    if(exp->subexp[0])
    {
        subexp[0] = new Expression(exp->subexp[0]);
        if(!subexp[0]->clean)
            clean = false;
        size += subexp[0]->size;
    }
    if(exp->subexp[1])
    {
        subexp[1] = new Expression(exp->subexp[1]);
        if(!subexp[1]->clean)
            clean = false;
        size += subexp[1]->size;
    }
}

Expression::~Expression()
{
    if(subexp[0])
        delete subexp[0];
    if(subexp[1])
        delete subexp[1];
}

Expression* Expression::trueExp()
{ return new Expression(TRUE, 0, 0, 0); }

Expression* Expression::falseExp()
{ return new Expression(FALSE, 0, 0, 0); }

Expression* Expression::varExp(int var)
{ return new Expression(VAR, var, 0, 0); }

Expression* Expression::opExp(ExpType op, Expression* a, Expression* b)
{
    if(!a)
        return 0;
    if(op == TRUE || op == FALSE || op == VAR)
        return 0;

    if(op == NOT)
        return new Expression(op, 0, a, 0);
    else if(b)
        return new Expression(op, 0, a, b);

    return 0;
}

ExpType Expression::getType()
{ return type; }

int Expression::getVar()
{ return var; }

Expression* Expression::getExp0()
{ return subexp[0]; }

Expression* Expression::getExp1()
{ return subexp[1]; }

bool Expression::isClean()
{ return clean; }

int Expression::getSize()
{ return size; }

void Expression::print(std::ostream& output, map<int, string>& names)
{
    switch(type)
    {
    case TRUE:
        output << "TRUE";
        break;
    case FALSE:
        output << "FALSE";
        break;
    case VAR:
        output << names[var];
        break;
    case NOT:
        subexp[0]->print(output, names);
        output << " !";
        break;
    case AND:
    case OR:
    case IMPLIES:
    case EQUIV:
        subexp[0]->print(output, names);
        output << " ";
        subexp[1]->print(output, names);
        switch(type)
        {
        case AND: output << " &&"; break;
        case OR: output << " ||"; break;
        case IMPLIES: output << " =>"; break;
        case EQUIV: output << " <=>"; break;
        }
        break;
    };
}

void Expression::printExp(std::ostream& output)
{
    switch(type)
    {
    case TRUE:
        output << "TRUE";
        break;
    case FALSE:
        output << "FALSE";
        break;
    case VAR:
        output << "$" << var;
        break;
    case NOT:
        if(subexp[0]->type == TRUE || subexp[0]->type == FALSE || subexp[0]->type == VAR)
        {
            output << "!";
            subexp[0]->printExp(output);
        }
        else
        {
            output << "!(";
            subexp[0]->printExp(output);
            output << ")";
        }
        break;
    case AND:
    case OR:
    case IMPLIES:
    case EQUIV:
        if(subexp[0]->type == TRUE || subexp[0]->type == FALSE || subexp[0]->type == VAR || subexp[0]->type == NOT)
            subexp[0]->printExp(output);
        else
        {
            output << "(";
            subexp[0]->printExp(output);
            output << ")";
        }
        switch(type)
        {
        case AND: output << " && "; break;
        case OR: output << " || "; break;
        case IMPLIES: output << " => "; break;
        case EQUIV: output << " <=> "; break;
        }
        if(subexp[1]->type == TRUE || subexp[1]->type == FALSE || subexp[1]->type == VAR || subexp[1]->type == NOT)
            subexp[1]->printExp(output);
        else
        {
            output << "(";
            subexp[1]->printExp(output);
            output << ")";
        }
        break;
    };
}

bool Expression::equals(Expression* exp)
{
    if(type != exp->type)
        return false;

    switch(type)
    {
    case TRUE:
    case FALSE:
        return true;
    case VAR:
        return var == exp->var;
    case NOT:
        return subexp[0]->equals(exp->subexp[0]);
    case AND:
    case OR:
    case IMPLIES:
    case EQUIV:
        return
            subexp[0]->equals(exp->subexp[0]) &&
            subexp[1]->equals(exp->subexp[1]);
    }
}

bool Expression::match(Expression* exp, map<int, Expression*>* remap_ptr)
{
    map<int, Expression*> remap_;
    if(!remap_ptr)
        remap_ptr = &remap_;

    if(type == VAR)
    {
        if(remap_ptr->find(var) == remap_ptr->end())
        {
            (*remap_ptr)[var] = exp;
            return true;
        }
        else
            return exp->equals((*remap_ptr)[var]);
    }

    if(type != exp->type)
        return false;

    switch(type)
    {
    case TRUE:
    case FALSE:
        return true;
    case NOT:
        return subexp[0]->match(exp->subexp[0], remap_ptr);
    case AND:
    case OR:
    case IMPLIES:
    case EQUIV:
        return
            subexp[0]->match(exp->subexp[0], remap_ptr) &&
            subexp[1]->match(exp->subexp[1], remap_ptr);
    }
}
