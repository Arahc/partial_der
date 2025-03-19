#include<iostream>
#include<memory>
#include<string>
#include<typeinfo>
#ifdef DEBUG
    #define debug(...) fprintf(stderr, __VA_ARGS__)
#else
    #define debug
#endif
#define error(...) fprintf(stderr, __VA_ARGS__),exit(-1)

using int32 = int;
using uint32 = unsigned int;
using int64 = long long;
using uint64 = unsigned long long;

template<typename T=int64> T gcd(T x, T y){
    while(x^=y^=x^=y%=x);
    return y;
}
template<typename T=int64> T lcm(T x, T y){
    return x/gcd(x, y)*y;
}
template<typename T1=int64,typename T2=int64> struct fraction{
    T1 a;T2 b; // a/b
    fraction(T1 _a=T1(0),T2 _b=T2(1)):a(_a),b(_b){
        if(std::is_same_v<T1,T2> && std::is_same_v<T1,int64>){
            if(b<0) a=-a,b=-b;
            if(a==0) b=1;
            else{
                T1 g=gcd(a,T1(b));
                a/=g,b/=T2(g);
            }
        }
    }
    fraction operator + (fraction x){
        return fraction(a*x.b+b*x.a,b*x.b);
    }
    fraction operator - (fraction x){
        return fraction(a*x.b-b*x.a,b*x.b);
    }
    fraction operator - (){
        return fraction(-a,b);
    }
    fraction operator * (fraction x){
        return fraction(a*x.a,b*x.b);
    }
    fraction operator / (fraction x){
        return fraction(a*x.b,b*x.a);
    }
    void operator += (fraction x){
        *this=*this+x;
    }
    void operator -= (fraction x){
        *this=*this-x;
    }
    void operator *= (fraction x){
        *this=*this*x;
    }
    void operator /= (fraction x){
        *this=*this/x;
    }
    bool operator == (fraction x){
        return (a==x.a && b==x.b);
    }
    bool operator < (fraction x){
        return a*x.b<x.a*b;
    }
    bool operator > (fraction x){
        return a*x.b>x.a*b;
    }
    bool operator <= (fraction x){
        return a*x.b<=x.a*b;
    }
    bool operator >= (fraction x){
        return a*x.b>=x.a*b;
    }
    bool operator != (fraction x){
        return (a!=x.a || b!=x.b);
    }
};
template<typename T1=int64,typename T2=int64> struct exponent{
    T1 a;T2 p; // a^p
    exponent(T1 _a=T1(0),T2 _p=T2(1)):a(_a),p(_p){}
};

class Function{ public:
    virtual std::unique_ptr<Function> get_der() const = 0;
    virtual std::string to_str() const = 0;
    virtual std::unique_ptr<Function> clone() const = 0;
    virtual uint32 get_type() const { return 0; }
};

/* Basic unit
 *  - Constant 1
 *  - Variable 2
 *  - Parameter 3
 */

template<typename T1=int64, typename T2=int64> class Constant: public Function{
    fraction<T1,T2> val;
public:
    Constant(T1 _a=T1(0), T2 _b=T2(1)):val(_a,_b){}
    Constant(fraction<T1,T2> _val):val(_val){}
    std::unique_ptr<Function> get_der() const override{
        return std::make_unique< Constant<> >(0);
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique< Constant<> >(val.a, val.b);
    }
    std::string to_str() const override{
        if(val.a==T1(0)) return "0";
        if(val.b==T2(1)) return std::to_string(val.a);
        return std::to_string(val.a)+"/"+std::to_string(val.b);
    }
    uint32 get_type() const override {return 1;}
};
class Variable: public Function{
    std::string name;
public:
    Variable(std::string _name="x"):name(_name){}
    std::unique_ptr<Function> get_der() const override{
        return std::make_unique< Constant<> >(1);
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique<Variable>(name);
    }
    std::string to_str() const override{
        return name;
    }
    uint32 get_type() const override {return 2;}
};
class Parameter: public Function{
    std::string name;
public:
    Parameter(std::string _name="a"):name(_name){}
    std::unique_ptr<Function> get_der() const override{
        return std::make_unique< Constant<> >(0);
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique<Parameter>(name);
    }
    std::string to_str() const override{
        return name;
    }
    uint32 get_type() const override {return 3;}
};

/* Basic operation
 *  - Add 4
 *  - Sub 5
 *  - Mul 6
 *  - Div 7
 */

class Add: public Function{
    std::unique_ptr<Function> f,g;
public:
    Add(std::unique_ptr<Function> _f, std::unique_ptr<Function> _g):f(std::move(_f)),g(std::move(_g)){}
    std::unique_ptr<Function> get_der() const override{
        // (f+g)' = f' + g'
        return std::make_unique<Add>(f->get_der(), g->get_der());
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique<Add>(f->clone(), g->clone());
    }
    std::string to_str() const override{
        if(g->to_str()=="0") return f->to_str();
        if(f->to_str()=="0") return g->to_str();
        return "("+f->to_str()+"+"+g->to_str()+")";
    }
    uint32 get_type() const override {return 4;}
};
class Sub: public Function{
    std::unique_ptr<Function> f,g;
public:
    Sub(std::unique_ptr<Function> _f, std::unique_ptr<Function> _g):f(std::move(_f)),g(std::move(_g)){}
    std::unique_ptr<Function> get_der() const override{
        // (f-g)' = f' - g'
        return std::make_unique<Sub>(f->get_der(), g->get_der());
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique<Sub>(f->clone(), g->clone());
    }
    std::string to_str() const override{
        if(g->to_str()=="0") return f->to_str();
        if(f->to_str()=="0") return "(-"+g->to_str()+")";
        return "("+f->to_str()+"-"+g->to_str()+")";
    }
    uint32 get_type() const override {return 5;}
};
class Mul: public Function{
    std::unique_ptr<Function> f,g;
public:
    Mul(std::unique_ptr<Function> _f, std::unique_ptr<Function> _g):f(std::move(_f)),g(std::move(_g)){}
    std::unique_ptr<Function> get_der() const override{
        // (f*g)' = f'*g + f*g'
        return std::make_unique<Add>(
            std::make_unique<Mul>(f->get_der(), g->clone()),
            std::make_unique<Mul>(f->clone(), g->get_der())
        );
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique<Mul>(f->clone(), g->clone());
    }
    std::string to_str() const override{
        if(g->to_str()=="0" || f->to_str()=="0") return "0";
        if(g->to_str()=="1") return f->to_str();
        if(f->to_str()=="1") return g->to_str();
        return "("+f->to_str()+"*"+g->to_str()+")";
    }
    uint32 get_type() const override {return 6;}
};
class Div: public Function{
    std::unique_ptr<Function> f,g;
public:
    Div(std::unique_ptr<Function> _f, std::unique_ptr<Function> _g):f(std::move(_f)),g(std::move(_g)){}
    std::unique_ptr<Function> get_der() const override{
        // (f/g)' = (f'*g - f*g')/g^2
        auto res = std::make_unique<Div>(
            std::make_unique<Sub>(
                std::make_unique<Mul>(f->get_der(), g->clone()),
                std::make_unique<Mul>(f->clone(), g->get_der())
            ),
            std::make_unique<Mul>(g->clone(), g->clone())
        );
        return res;
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique<Div>(f->clone(), g->clone());
    }
    std::string to_str() const override{
        if(g->to_str()=="1") return f->to_str();
        if(f->to_str()=="0") return "0";
        return "("+f->to_str()+"/"+g->to_str()+")";
    }
    uint32 get_type() const override {return 7;}
};

/* Elementary function
 *  - Ln 8
 *  - Exp 9
 *  - Pow 10
 */

class Ln: public Function{
    std::unique_ptr<Function> f;
public:
    Ln(std::unique_ptr<Function> _f):f(std::move(_f)){}
    std::unique_ptr<Function> get_der() const override{
        // (ln(f))' = f'/f
        return std::make_unique<Div>(f->get_der(), f->clone());
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique<Ln>(f->clone());
    }
    std::string to_str() const override{
        if(f->to_str()=="1") return "0";
        return "\\ln("+f->to_str()+")";
    }
    uint32 get_type() const override {return 8;}
};
class Exp: public Function{
    std::unique_ptr<Function> f;
public:
    Exp(std::unique_ptr<Function> _f):f(std::move(_f)){}
    std::unique_ptr<Function> get_der() const override{
        // (exp(f))' = f'*exp(f)
        return std::make_unique<Mul>(f->get_der(), std::make_unique<Exp>(f->clone()));
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique<Exp>(f->clone());
    }
    std::string to_str() const override{
        if(f->to_str()=="0") return "1";
        if(f->to_str()=="1") return "e";
        return "e^("+f->to_str()+")";
    }
    uint32 get_type() const override {return 9;}
};
class Pow: public Function{
    std::unique_ptr<Function> f,p;
public:
    template<typename T1=int64, typename T2=int64> Pow(std::unique_ptr<Function> _f, T1 _a, T2 _b):f(std::move(_f)),p(std::make_unique< Constant<T1,T2> >(_a,_b)){}
    template<typename T1=int64, typename T2=int64> Pow(std::unique_ptr<Function> _f, fraction<T1,T2> _p):f(std::move(_f)),p(std::make_unique< Constant<T1,T2> >(_p)){}
    Pow(std::unique_ptr<Function> _f, std::unique_ptr<Function> _p):f(std::move(_f)),p(std::move(_p)){}
    std::unique_ptr<Function> get_der() const override{
        // (f^p)' = f^p * (p'*ln(f) + p*f'/f)
        return std::make_unique<Mul>(
            std::make_unique<Pow>(f->clone(), p->clone()),
            std::make_unique<Add>(
                std::make_unique<Mul>(p->get_der(), std::make_unique<Ln>(f->clone())),
                std::make_unique<Mul>(
                    p->clone(),
                    std::make_unique<Div>(f->get_der(), f->clone())
                )
            )
        );
    }
    std::unique_ptr<Function> clone() const override{ 
        return std::make_unique<Pow>(f->clone(), p->clone());
    }
    std::string to_str() const override{
        if(p->to_str()=="0") return "1";
        if(p->to_str()=="1") return f->to_str();
        return "("+f->to_str()+"^"+p->to_str()+")";
    }
    uint32 get_type() const override {return 10;}
};

int main(int argc, char *argv[]){
    auto f=
    std::make_unique<Add>(
        std::make_unique<Add>(
            std::make_unique<Mul>(
                std::make_unique<Sub>(
                    std::make_unique<Ln>(
                        std::make_unique<Variable>("x")
                    ),
                    std::make_unique<Exp>(std::make_unique<Variable>("x"))
                ),
                std::make_unique<Variable>("x")
            ),
            std::make_unique<Div>(
                std::make_unique< Parameter >("a"),
                std::make_unique< Variable >("x")
            )
        ),
        std::make_unique<Sub>(
            std::make_unique<Pow>(
                std::make_unique<Variable>("x"),
                std::make_unique< Constant<> >(2)
            ),
            std::make_unique<Pow>(
                std::make_unique< Constant<> >(2),
                std::make_unique<Variable>("x")
            )
        )
    );
    printf("f : %s\n",f->to_str().c_str());
    auto df=f->get_der();
    printf("f': %s\n",df->to_str().c_str());

    return 0;
}