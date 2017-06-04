#ifndef FORMULA_H_INCLUDED
#define FORMULA_H_INCLUDED

#include <iostream>
#include <map>
#include <vector>

enum ExpType
{
    TRUE, FALSE, VAR, NOT, AND, OR, IMPLIES, EQUIV
};

class Expression
{
private:
    Expression(ExpType type, int var, Expression* a, Expression* b);

    ExpType type;
    int var;
    Expression* subexp[2];
    bool clean;
    int size;
public:
    Expression(Expression* exp);
    ~Expression();

    static Expression* trueExp();
    static Expression* falseExp();
    static Expression* varExp(int var);
    static Expression* opExp(ExpType op, Expression* a, Expression* b = 0);
    
    ExpType getType();
    int getVar();
    Expression* getExp0();
    Expression* getExp1();
    bool isClean();
    int getSize();

    //returns true if expressions are equal 
    bool equals(Expression* exp);

    //returns true if exp has the form of this expression
    bool match(Expression* exp, std::map<int, Expression*>* remap = 0);

    //prints the formula in RPN
    void print(std::ostream& output, std::map<int, std::string>& names);

    //prints the formula in infix
    void printExp(std::ostream& output);
};

#endif //FORMULA_H_INCLUDED
