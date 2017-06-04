#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>
#include "expression.h"
#include "equiv.h"

using namespace std;

#define ETRUE               (Expression::trueExp())
#define EFALSE              (Expression::falseExp())
#define EVAR(a)             (Expression::varExp(a))
#define ENOT(a)             (Expression::opExp(NOT,a))
#define EAND(a,b)           (Expression::opExp(AND,a,b))
#define EOR(a,b)            (Expression::opExp(OR,a,b))
#define EIMPLIES(a,b)       (Expression::opExp(IMPLIES,a,b))
#define EEQUIV(a,b)         (Expression::opExp(EQUIV,a,b))

#define PADDEQUIV(p,a,b)    {Expression* e1 = (a); Expression* e2 = (b); (p)->append(e1, e2); delete e1; delete e2;}

Expression* rebuildExp(Partition* equivs, Expression* exp, map<int, Expression*>& remap, int* varcount, int varmax);
Expression* transform(Partition* equivs, Expression* exp, vector<int> path, int* varcount, int varmax);

Expression* rebuildExp(Partition* equivs, Expression* exp, map<int, Expression*>& remap, int* varcount, int varmax)
{
    switch(exp->getType())
    {
    case TRUE:
    case FALSE:
        return new Expression(exp);
    case VAR:
        if(remap.find(exp->getVar()) == remap.end())
        {
            if(*varcount < varmax && rand() % 100 < 75)
            {
                int var = (*varcount)++;
                remap[exp->getVar()] = Expression::varExp(var);
                return remap[exp->getVar()];
            }
            else
            {
                int var = rand() % (*varcount);
                remap[exp->getVar()] = Expression::varExp(var);
                return remap[exp->getVar()];
            }
        }
        else
            return new Expression(remap[exp->getVar()]);
    case NOT:
    case AND:
    case OR:
    case IMPLIES:
    case EQUIV:
    {
        Expression* l = rebuildExp(equivs, exp->getExp0(), remap, varcount, varmax);
        Expression* r = 0;
        if(exp->getExp1())
            r = rebuildExp(equivs, exp->getExp1(), remap, varcount, varmax);
        return Expression::opExp(exp->getType(), l, r);
    }
    }
    return 0;
}

Expression* transform(Partition* equivs, Expression* exp, vector<int> path, int* varcount, int varmax)
{
    if(path.size() == 0)
    {
        vector<EquivClass*> classes = equivs->getEquiv(exp);
        while(classes.size())
        {
            int i_class = rand() % classes.size();
            EquivClass* cls = classes[i_class];

            vector<Expression*> exps = cls->getExpressions();
            Expression* matched = 0;
            map<int, Expression*> remap;
            for(int i = 0; i < exps.size(); i++)
            {
                map<int, Expression*> remap2;
                if(exps[i]->match(exp, &remap2))
                    if(matched == 0 || remap2.size() < remap.size())
                    {
                        matched = exps[i];
                        remap = remap2;
                    }
            }

            vector<Expression*> exps_big;
            for(int i = 0; i < exps.size(); i++)
                if(exps[i]->getSize() >= exp->getSize())
                    exps_big.push_back(exps[i]);
            if(exps_big.size())
                exps = exps_big;

            while(exps.size())
            {
                int i_exp = rand() % exps.size(); //TODO: BETTER RANDOM
                Expression* exp2 = exps[i_exp];
                Expression* res = rebuildExp(equivs, exp2, remap, varcount, varmax);
                if(res->equals(exp))
                {
                    delete res;
                    exps.erase(exps.begin() + i_exp);
                }
                else
                {
                    /*exp->printExp(cerr);
                    cerr << endl << "    by ";
                    exp2->printExp(cerr);
                    cerr << endl << "    => ";
                    res->printExp(cerr);
                    cerr << endl;*/
                    return res;
                }
            }

            classes.erase(classes.begin() + i_class);
        }
        return new Expression(exp);
    }
    else
    {
        if(path[0] == 0)
        {
            //Expression* l = rebuildExp(equivs, exp->getExp0(), remap, varcount, varmax);
            path.erase(path.begin());
            Expression* l = transform(equivs, exp->getExp0(), path, varcount, varmax);
            Expression* r = 0;
            if(exp->getExp1())
                r = new Expression(exp->getExp1());
            return Expression::opExp(exp->getType(), l, r);
        }
        else
        {
            path.erase(path.begin());
            Expression* l = new Expression(exp->getExp0());
            Expression* r = transform(equivs, exp->getExp1(), path, varcount, varmax);
            return Expression::opExp(exp->getType(), l, r);
        }
    }
}

void getPaths(vector<vector<int> >& exps, vector<vector<int> >& leaves, Expression* exp, vector<int> path = vector<int>())
{
    exps.push_back(path);
    if(!exp->getExp0() && !exp->getExp1())
        leaves.push_back(path);

    path.push_back(0);
    if(exp->getExp0())
    {
        path[path.size() - 1] = 0;
        getPaths(exps, leaves, exp->getExp0(), path);
    }
    if(exp->getExp1())
    {
        path[path.size() - 1] = 1;
        getPaths(exps, leaves, exp->getExp1(), path);
    }
}

Expression* cleanExp(Expression* exp, int varcount)
{
    switch(exp->getType())
    {
    case TRUE:
        return EOR(EVAR(0), ENOT(EVAR(0)));
    case FALSE:
        return EAND(EVAR(0), ENOT(EVAR(0)));
    case VAR:
        return new Expression(exp);
    case NOT:
    case AND:
    case OR:
    case IMPLIES:
    case EQUIV:
    {
        Expression* l = cleanExp(exp->getExp0(), varcount);
        Expression* r = 0;
        if(exp->getExp1())
            r = cleanExp(exp->getExp1(), varcount);
        return Expression::opExp(exp->getType(), l, r);
    }
    }
}

int main(int argc, char** argv)
{
    timeval tv;
    gettimeofday(&tv, 0);
    srand(tv.tv_sec * 1000000 + tv.tv_usec);

    Partition* equivs = new Partition();

    PADDEQUIV(equivs, EVAR(0), EAND(EVAR(0), ETRUE));
    PADDEQUIV(equivs, EVAR(0), EOR(EVAR(0), EFALSE));
    PADDEQUIV(equivs, EVAR(0), EAND(EVAR(0), EVAR(0)));
    PADDEQUIV(equivs, EVAR(0), EOR(EVAR(0), EVAR(0)));
    PADDEQUIV(equivs, EVAR(0), EAND(EVAR(0), EOR(EVAR(0), EVAR(1))));
    PADDEQUIV(equivs, EVAR(0), EOR(EVAR(0), EAND(EVAR(0), EVAR(1))));
    
    PADDEQUIV(equivs, EAND(EVAR(0), EVAR(1)), EAND(EVAR(1), EVAR(0)));
    PADDEQUIV(equivs, EOR(EVAR(0), EVAR(1)), EOR(EVAR(1), EVAR(0)));
    PADDEQUIV(equivs, EEQUIV(EVAR(0), EVAR(1)), EEQUIV(EVAR(1), EVAR(0)));

    PADDEQUIV(equivs, EAND(EAND(EVAR(0), EVAR(1)), EVAR(2)), EAND(EVAR(0), EAND(EVAR(1), EVAR(2))));
    PADDEQUIV(equivs, EOR(EOR(EVAR(0), EVAR(1)), EVAR(2)), EOR(EVAR(0), EOR(EVAR(1), EVAR(2))));
    PADDEQUIV(equivs, EEQUIV(EEQUIV(EVAR(0), EVAR(1)), EVAR(2)), EEQUIV(EVAR(0), EEQUIV(EVAR(1), EVAR(2))));

    PADDEQUIV(equivs, EAND(EVAR(0), EOR(EVAR(1), EVAR(2))), EOR(EAND(EVAR(0), EVAR(1)), EAND(EVAR(0), EVAR(2))));
    PADDEQUIV(equivs, EOR(EVAR(0), EAND(EVAR(1), EVAR(2))), EAND(EOR(EVAR(0), EVAR(1)), EOR(EVAR(0), EVAR(2))));

    PADDEQUIV(equivs, ENOT(EVAR(0)), EIMPLIES(EVAR(0), EFALSE));
    PADDEQUIV(equivs, ENOT(ENOT(EVAR(0))), EVAR(0));
    PADDEQUIV(equivs, ENOT(EAND(EVAR(0), EVAR(1))), EOR(ENOT(EVAR(0)), ENOT(EVAR(1))));
    PADDEQUIV(equivs, ENOT(EOR(EVAR(0), EVAR(1))), EAND(ENOT(EVAR(0)), ENOT(EVAR(1))));

    PADDEQUIV(equivs, EIMPLIES(EVAR(0), EVAR(1)), EOR(ENOT(EVAR(0)), EVAR(1)));
    PADDEQUIV(equivs, EIMPLIES(EVAR(0), EVAR(1)), EIMPLIES(ENOT(EVAR(1)), ENOT(EVAR(0))));

    PADDEQUIV(equivs, EEQUIV(EVAR(0), EVAR(1)), EAND(EIMPLIES(EVAR(0), EVAR(1)), EIMPLIES(EVAR(1), EVAR(0))));
    PADDEQUIV(equivs, EEQUIV(EVAR(0), EVAR(1)), EOR(EAND(EVAR(0), EVAR(1)), EAND(ENOT(EVAR(0)), ENOT(EVAR(1)))));
    PADDEQUIV(equivs, EEQUIV(EVAR(0), EVAR(1)), EAND(EOR(ENOT(EVAR(0)), EVAR(1)), EOR(EVAR(0), ENOT(EVAR(1)))));

    PADDEQUIV(equivs, ETRUE, EOR(EVAR(0), ENOT(EVAR(0))));
    PADDEQUIV(equivs, ETRUE, EIMPLIES(EVAR(0), EVAR(0)));
    PADDEQUIV(equivs, ETRUE, EIMPLIES(EFALSE, EVAR(0)));
    PADDEQUIV(equivs, ETRUE, EEQUIV(EVAR(0), EVAR(0)));
    PADDEQUIV(equivs, ETRUE, EOR(EVAR(0), ETRUE));
    PADDEQUIV(equivs, ETRUE, EAND(ETRUE, ETRUE));

    PADDEQUIV(equivs, EFALSE, EAND(EVAR(0), ENOT(EVAR(0))));
    PADDEQUIV(equivs, EFALSE, EIMPLIES(ETRUE, EFALSE));
    PADDEQUIV(equivs, EFALSE, EEQUIV(EVAR(0), ENOT(EVAR(0))));
    PADDEQUIV(equivs, EFALSE, EAND(EVAR(0), EFALSE));

    /*vector<EquivClass*> classes = equivs->getClasses();
    for(int i = 0; i < classes.size(); i++)
    {
        cerr << i << ":" << endl;
        vector<Expression*> exps = classes[i]->getExpressions();
        for(int j = 0; j < exps.size(); j++)
        {
            cerr << "    ";
            exps[j]->printExp(cerr);
            cerr << endl;
        }
    }*/
    int max_var = 16;
    int num_transform = 16;
    int type = -1;               //1 = TRUE, 0 = SAT, -1 = UNSAT

    for(int i = 1; i < argc; i++)
    {
        string next = argv[i];
        if(next == "-v" || next == "--var")
        {
            if(i + 1 >= argc)
                { cerr << "Missing argument!" << endl; return 1; }
            string arg1 = argv[++i];
            stringstream ss;
            ss << arg1;
            ss >> max_var;
        }
        else if(next == "-t" || next == "--transform")
        {
            if(i + 1 >= argc)
                { cerr << "Missing argument!" << endl; return 1; }
            string arg1 = argv[++i];
            stringstream ss;
            ss << arg1;
            ss >> num_transform;
        }
        else if(next == "-T" || next == "--true")
            type = 1;
        else if(next == "-S" || next == "--sat")
            type = 0;
        else if(next == "-U" || next == "--unsat")
            type = -1;
        else if(next == "-h" || next == "--help")
        {
            cout <<
                "usage: " << argv[0] << " [Options]" << endl <<
                "Options:" << endl <<
                "    -v <num>, --var <num>" << endl <<
                "        Sets the maximum number of variables to <num>." << endl <<
                "        (Default: 16)" << endl <<
                "    -t <num>, --transform <num>" << endl <<
                "        Sets the number of random transformations to <num>." << endl <<
                "        (Default: 16)" << endl <<
                "    -T, --true" << endl <<
                "        Output a tautology" << endl <<
                "    -S, --sat" << endl <<
                "        Output a satisfiable expression" << endl <<
                "    -U, --unsat" << endl <<
                "        Output an unsatisfiable expression" << endl <<
                "        (Default)" << endl <<
                "    -h, --help" << endl <<
                "        Prints this help message." << endl;
            return 0;
        }
        else
            { cerr << "Unknown option: " << next << endl; return 1; }
    }

    int varcount = 1;
    Expression* exp = 0;

    if(type == -1)
        exp = EFALSE;
    else if(type == 0)
    {
        exp = EVAR(varcount++);
        if(rand() % 2)
            exp = ENOT(exp);

        for(int i = 1; i < 4; i++)
        {
            Expression* e = EVAR(varcount++);
            if(rand() % 2)
                e = ENOT(e);
            if(rand() % 2)
                exp = EAND(exp, e);
            else
                exp = EOR(exp, e);
        }
    }
    else if(type == 1)
        exp = ETRUE;

    for(int i = 0; i < num_transform; i++)
    {
        vector<vector<int> > paths;
        vector<vector<int> > leaves;
        getPaths(paths, leaves, exp);
        /*for(int i2 = 0; i2 < paths.size(); i2++)
        {
            for(int i3 = 0; i3 < paths[i2].size(); i3++)
                cerr << paths[i2][i3] << " ";
            cerr << endl;
        }*/
        vector<int> path;
        if(rand() % 100 < 50)
            path = leaves[rand() % leaves.size()];
        else
            path = paths[rand() % paths.size()];

        Expression* exp2 = transform(equivs, exp, path, &varcount, max_var);
        delete exp;
        exp = exp2;
        /*exp->printExp(cerr);
        cerr << endl;
        {
            exp2 = cleanExp(exp, varcount);
            map<int, string> varnames;
            for(int i = 0; i < varcount; i++)
            {
                stringstream ss;
                ss << "x" << i;
                varnames[i] = ss.str();
            }
            exp2->print(cerr, varnames);
            cerr << endl;
            delete exp2;
        }
        cerr << endl;*/
    }


    Expression* exp2 = cleanExp(exp, varcount);
    map<int, string> varnames;
    for(int i = 0; i < varcount; i++)
    {
        stringstream ss;
        ss << "x" << i;
        varnames[i] = ss.str();
    }
    exp2->print(cout, varnames);
    cout << endl;

    delete exp2;
    delete exp;
    return 0;
}
