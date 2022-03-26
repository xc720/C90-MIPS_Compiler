#ifndef ast_statements_hpp
#define ast_statements_hpp

#include "ast_expressions.hpp"
#include "ast_primitives.hpp"

class Stat;

typedef const Stat *StatPtr;

class Stat
{
public:
    virtual ~Stat() {}
    virtual void pretty_print(std::ostream &dst) const = 0;
    virtual void Translate2MIPS(std::string destReg) const = 0;
};

class Stat_list;

typedef const Stat_list *Stat_listPtr;

class Stat_list
    : public Stat
{
private:
    StatPtr stat;
    Stat_listPtr stat_list;

public:
    Stat_list(StatPtr _stat, Stat_listPtr _stat_list = nullptr)
        : stat(_stat), stat_list(_stat_list)
    {
    }
    virtual ~Stat_list()
    {
        delete stat;
        delete stat_list;
    }
    StatPtr return_stat() const
    {
        return stat;
    }
    Stat_listPtr return_stat_list() const
    {
        return stat_list;
    }
    virtual void pretty_print(std::ostream &dst) const override
    {
        stat->pretty_print(dst);
        if (stat_list != nullptr)
        {
            stat_list->pretty_print(dst);
        }
    }

    virtual void Translate2MIPS(std::string destReg) const override
    {
        return_stat()->Translate2MIPS(destReg);
        if (stat_list != nullptr)
        {
            return_stat_list()->Translate2MIPS(destReg);
        }
    }
};

class Select_Stat
    : public Stat
{
private:
    ExpressionPtr condition;
    StatPtr stat;

public:
    Select_Stat(ExpressionPtr _condition, StatPtr _stat = nullptr)
        : condition(_condition), stat(_stat)
    {
    }
    virtual ~Select_Stat()
    {
        delete condition;
        delete stat;
    }
    ExpressionPtr return_cond() const
    {
        return condition;
    }
    StatPtr return_stat() const
    {
        return stat;
    }
};

class If_Stat
    : public Select_Stat
{
private:
    StatPtr else_branch;

public:
    If_Stat(ExpressionPtr _condition, StatPtr _if_branch = nullptr, StatPtr _else_branch = nullptr)
        : Select_Stat(_condition, _if_branch), else_branch(_else_branch)
    {
    }
    ~If_Stat()
    {
        delete else_branch;
    }
    StatPtr getElse() const
    {
        return else_branch;
    }

    virtual void pretty_print(std::ostream &dst) const override
    {
        dst << "if ( ";
        return_cond()->pretty_print(dst);
        dst << " ) ";
        return_stat()->pretty_print(dst);
        if (else_branch != nullptr)
        {
            dst << "else ";
            else_branch->pretty_print(dst);
        }
        dst << '\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override
    {
        std::string condition = makeName("condition");
        return_cond()->Translate2MIPS(condition);
        std::string exit = makeName("exit");
        if (else_branch != nullptr)
        {
            std::string else_stat = makeName("else_stat");
            std::cout << "beq " << condition << " $0 " << else_stat << std::endl;
            return_stat()->Translate2MIPS(destReg);
            std::string exit = makeName("exit");
            std::cout << "j " << exit << std::endl;
            std::cout << else_stat << ":" << std::endl;
            getElse()->Translate2MIPS(destReg);
            std::cout << exit << ":" << std::endl;
        }
        else
        {
            std::string exit = makeName("exit");
            std::cout << "beq " << condition << " $0 " << exit << std::endl;
            return_stat()->Translate2MIPS(destReg);
            std::cout << exit << ":" << std::endl;
        }
    }
};

class Loop_Stat
    : public Stat
{
private:
    ExpressionPtr condition;
    StatPtr stat;

public:
    Loop_Stat() {}
    Loop_Stat(ExpressionPtr _condition, StatPtr _stat = nullptr)
        : condition(_condition), stat(_stat)
    {
    }
    ~Loop_Stat()
    {
        delete condition;
        delete stat;
    }
    ExpressionPtr return_cond() const
    {
        return condition;
    }
    StatPtr return_stat() const
    {
        return stat;
    }
};

class While_Stat
    : public Loop_Stat
{
public:
    While_Stat(ExpressionPtr _condition, StatPtr _stat = nullptr)
        : Loop_Stat(_condition, _stat)
    {
    }

    virtual void pretty_print(std::ostream &dst) const override
    {
        dst << "while ( ";
        return_cond()->pretty_print(dst);
        dst << " ) ";
        return_stat()->pretty_print(dst);
        dst << '\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override
    {
         std::string condition = makeName("while_condition");
        return_cond()->Translate2MIPS(condition);
        std::string exit = makeName("exit");
        std::cout << "beq " << condition << " $0 " << exit << std::endl;
        std::string start = makeName("start");
        std::cout << start <<  ":" << std::endl;
        return_stat()->Translate2MIPS(destReg);
        return_cond()->Translate2MIPS(condition);
        std::cout << "bne " << condition << " $0 " << start << std::endl;
        std::cout << exit << ":" << std::endl;
        std::cout << "add " << destReg << " $0 $0" << std::endl;
        }
    };

class For_Stat
    : public Loop_Stat
{
private:
    Variable *initVar = nullptr;
    ExpressionPtr initExpr = nullptr;
    ExpressionPtr checkExpr;
    ExpressionPtr updateExpr;
    StatPtr stat;

public:
    For_Stat(ExpressionPtr _initExpr, ExpressionPtr _checkExpr, ExpressionPtr _updateExpr, StatPtr _stat)
        : initExpr(_initExpr), checkExpr(_checkExpr), updateExpr(_updateExpr), stat(_stat)
    {
    }
    For_Stat(Variable *_initVar, ExpressionPtr _checkExpr, ExpressionPtr _updateExpr, StatPtr _stat)
        : initVar(_initVar), checkExpr(_checkExpr), updateExpr(_updateExpr), stat(_stat)
    {
    }
    ~For_Stat()
    {
        delete initVar;
        delete initExpr;
        delete checkExpr;
        delete updateExpr;
        delete stat;
    }
    virtual void pretty_print(std::ostream &dst) const override
    {
        dst << "for ( ";
        if (initVar != nullptr && initExpr == nullptr)
        {
            initVar->pretty_print(dst);
        }
        else if (initVar == nullptr && initExpr != nullptr)
        {
            initExpr->pretty_print(dst);
        }
        dst << " ; ";
        checkExpr->pretty_print(dst);
        dst << " ; ";
        updateExpr->pretty_print(dst);
        dst << " ) \n";
        return_stat()->pretty_print(dst);
    }

    virtual void Translate2MIPS(std::string destReg) const override
    {
        std::string condition = makeName("while_condition");
        return_cond()->Translate2MIPS(condition);
        std::string unique_exit = makeName("exit");
        std::cout << "beq " << condition << " $0 " << unique_exit << std::endl;
        std::string unique_start = makeName("start");
        std::cout << unique_start << ":" << std::endl;
        return_stat()->Translate2MIPS(destReg);
        return_cond()->Translate2MIPS(condition);
        std::cout << "bne " << condition << " $0 " << unique_start << std::endl;
        std::cout << unique_exit << ":" << std::endl;
        std::cout << "add " << destReg << " $0 $0" << std::endl;
    }
};

class Expression_Stat
    : public Stat
{
private:
    ExpressionPtr expression;

public:
    Expression_Stat(ExpressionPtr _expression = nullptr)
        : expression(_expression)
    {
    }
    ~Expression_Stat()
    {
        delete expression;
    }
    ExpressionPtr return_Exp() const
    {
        return expression;
    }
    virtual void pretty_print(std::ostream &dst) const override
    {
        if (expression != nullptr)
        {
            expression->pretty_print(dst);
        }
        dst << ";";
        dst << '\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override
    {
        return_Exp()->Translate2MIPS(destReg);
    }
};

class Jump_Stat
    : public Stat
{
private:
    ExpressionPtr expression;

public:
    Jump_Stat(ExpressionPtr _expression = nullptr)
        : expression(_expression)
    {
    }
    ~Jump_Stat()
    {
        delete expression;
    }
    ExpressionPtr return_Exp() const
    {
        return expression;
    }
    virtual void pretty_print(std::ostream &dst) const override
    {
        dst << "return ";
        if (expression != nullptr)
        {
            expression->pretty_print(dst);
        }

        dst << ";";
        dst << '\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override
    {
    std::string exp = makeName("exp");
    return_Exp()->Translate2MIPS(exp);
    std::cout << "add $2 $0 " << exp << std::endl;
    }
};

class Comp_Stat
    : public Stat
{
private:
    Stat_listPtr stat_List;
    Decl_listPtr decl_List;

public:
    Comp_Stat()
    {
    }
    Comp_Stat(Stat_listPtr _stat_List)
        : stat_List(_stat_List)
    {
    }
    Comp_Stat(Decl_listPtr _decl_List)
        : decl_List(_decl_List)
    {
    }
    Comp_Stat(Decl_listPtr _decl_List, Stat_listPtr _stat_List)
        : stat_List(_stat_List), decl_List(_decl_List)
    {
    }
    ~Comp_Stat()
    {
        delete stat_List;
        delete decl_List;
    }

    Stat_listPtr getstatlist() const
    {
        return stat_List;
    }

    Decl_listPtr getdecllist() const
    {
        return decl_List;
    }
    virtual void pretty_print(std::ostream &dst) const override
    {
        dst << "{\n";
        if (decl_List != nullptr)
        {
            decl_List->pretty_print(dst);
        }
        if (stat_List != nullptr)
        {
            stat_List->pretty_print(dst);
        }
        dst << "}";
        dst << '\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override
    {
        if (decl_List != nullptr)
        {
            decl_List->Translate2MIPS(destReg);
        }
        if (stat_List != nullptr)
        {
            stat_List->Translate2MIPS(destReg);
        }
    }
};

class Continue_Stat
    : public Jump_Stat
{
public:
    Continue_Stat()
    {}
    ~Continue_Stat() 
    {}

    virtual void pretty_print(std::ostream &dst) const override
    {
        dst<<"continue;";
        dst<<'\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override{
        std::cout << "j "   << std::endl;
    }
};

class Break_Stat
    : public Jump_Stat
{
public:
    Break_Stat()
    {}
    ~Break_Stat() 
    {}

    virtual void pretty_print(std::ostream &dst) const override
    {
        dst<<"break;";
        dst<<'\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override{
        std::cout << "j "   << std::endl;
    }
};

class Label_Stat
    : public Stat
{
private:
    ExpressionPtr expression;
    StatPtr statement;
public:
    Label_Stat(ExpressionPtr _expression = nullptr, StatPtr _statement = nullptr)
        : expression(_expression)
        , statement(_statement)
    {}
    ~Label_Stat() {
        delete expression;
        delete statement;
    }
    ExpressionPtr return_Exp() const
    { return expression; }
    StatPtr getStat() const
    { return statement; }
    virtual void pretty_print(std::ostream &dst) const override
    {
        dst<<"case ";
        if(expression!=nullptr){
            expression->pretty_print(dst);
        }
        dst<<": ";
        if(statement!=nullptr){
            statement->pretty_print(dst);
        }
        dst<<'\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override {
        
    }
};

class Switch_Stat
    : public Select_Stat
{
public:
    Switch_Stat(ExpressionPtr _condition, StatPtr _stat)
        : Select_Stat(_condition, _stat)
    {}
    ~Switch_Stat() {
    }

    virtual void pretty_print(std::ostream &dst) const override
    {
        dst<<"switch ( ";
        return_cond()->pretty_print(dst);
        dst<<" ) ";
        return_stat()->pretty_print(dst);
        dst<<'\n';
    }

    virtual void Translate2MIPS(std::string destReg) const override {

    }
};

#endif