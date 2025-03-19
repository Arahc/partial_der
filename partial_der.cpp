#include<stdio.h>
#include<memory>
#include<string>
#ifdef DEBUG
    #define debug(...) fprintf(stderr, __VA_ARGS__)
#else
    #define debug
#endif

bool LATEX_MODE=false;

template<typename T=int64_t> T gcd(T x, T y){
    while(x^=y^=x^=y%=x);
    return y;
}
template<typename T=int64_t> T lcm(T x, T y){
    return x/gcd(x, y)*y;
}
template<typename T=int64_t> struct fraction{
    T a,b; // a/b
    fraction(T _a=T(0),T _b=T(1)):a(_a),b(_b){
        if(b<T(0)) a=-a,b=-b;
        else if(b==T(0)) throw("Divide by 0");
        if(a==T(0)) b=1;
        else{
            T g=gcd(a,b);
            a/=g,b/=g;
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

class Function{ public:
    virtual std::unique_ptr<Function> get_der() const = 0;
    virtual std::string to_str() const = 0;
    virtual std::unique_ptr<Function> clone() const = 0;

    enum Type{
        CONST, VAR, PARAM, ADD, SUB, MUL, DIV, POW, LN, EXP, UNDEF
    }type;
    virtual Type get_type() const {return UNDEF;}
};

/* Basic unit
*  - Constant 1
*  - Variable 2
*  - Parameter 3
*/

template<typename T=int64_t> class Constant: public Function{
    fraction<T> val;
public:
    Constant(T _a=T(0), T _b=T(1)):val(_a,_b){}
    Constant(fraction<T> _val):val(_val){}
    std::unique_ptr<Function> get_der() const override{
        return std::make_unique< Constant<> >(0);
    }
    std::unique_ptr<Function> clone() const override{
        return std::make_unique< Constant<> >(val.a, val.b);
    }
    std::string to_str() const override{
        if(val.a==T(0)) return "0";
        if(val.b==T(1)) return std::to_string(val.a);
        if(LATEX_MODE) return "\\frac{"+std::to_string(val.a)+"}{"+std::to_string(val.b)+"}";
        return std::to_string(val.a)+"/"+std::to_string(val.b);
    }
    Type get_type() const override {return CONST;}
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
    Type get_type() const override {return VAR;}
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
    Type get_type() const override {return PARAM;}
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
        if(g->to_str()[0]=='-') return f->to_str()+g->to_str();
        return f->to_str()+"+"+g->to_str();
    }
    Type get_type() const override {return ADD;}
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
        if(f->to_str()=="0") return "-"+g->to_str();
        return f->to_str()+"-"+g->to_str();
    }
    Type get_type() const override {return SUB;}
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
        std::string f_str=f->to_str(),g_str=g->to_str();
        if(f->get_type()==ADD || f->get_type()==SUB) f_str="("+f_str+")";
        if(g->get_type()==ADD || g->get_type()==SUB) g_str="("+g_str+")";
        if(LATEX_MODE) return f_str+"\\cdot "+g_str;
        return f_str+"*"+g_str;
    }
    Type get_type() const override {return MUL;}
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
        std::string f_str=f->to_str(),g_str=g->to_str();
        if(LATEX_MODE) return "\\frac{"+f_str+"}{"+g_str+"}";
        if(f->get_type()==ADD || f->get_type()==SUB) f_str="("+f_str+")";
        if(g->get_type()==ADD || g->get_type()==SUB || g->get_type()==MUL || g->get_type()==DIV) g_str="("+g_str+")";
        return f_str+"/"+g_str;
    }
    Type get_type() const override {return DIV;}
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
    Type get_type() const override {return LN;}
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
        if(LATEX_MODE) return "e^{"+f->to_str()+"}";
        return "e^("+f->to_str()+")";
    }
    Type get_type() const override {return EXP;}
};
class Pow: public Function{
    std::unique_ptr<Function> f,p;
public:
    template<typename T=int64_t> Pow(std::unique_ptr<Function> _f, T _a, T _b):f(std::move(_f)),p(std::make_unique< Constant<T> >(_a,_b)){}
    template<typename T=int64_t> Pow(std::unique_ptr<Function> _f, fraction<T> _p):f(std::move(_f)),p(std::make_unique< Constant<T> >(_p)){}
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
        std::string f_str=f->to_str(),p_str=p->to_str();
        if(f->get_type()!=CONST && f->get_type()!=VAR && f->get_type()!=PARAM) f_str="("+f_str+")";
        if(LATEX_MODE) return f->to_str()+"^{"+p->to_str()+"}";
        return f->to_str()+"^("+p->to_str()+")";
    }
    Type get_type() const override {return POW;}
};

template<typename T=int64_t> struct Token{
    enum Type{
        NUMBER, WORD, ADD, SUB, MUL, DIV, POW, LPAR, RPAR, eof, error
    }type;
    std::string word;
    fraction<T> val;
};
template<typename T=int64_t> class Lexer{
    const std::string str;
    size_t pos,len;
    char nowChar;
    void next(){
        ++pos;
        nowChar=(pos<len?str[pos]:'\0');
    }
    void prev(){
        --pos;
        nowChar=(pos<len?str[pos]:'\0');
    }
    T get_num(){
        T x=0;
        if(!isdigit(nowChar)) throw("ReadType Error (number)");
        while(isdigit(nowChar)) x=x*10+(nowChar^48),next();
        return x;
    }
    std::string get_word(){
        std::string res="";
        if(!isalpha(nowChar)) throw("ReadType Error (parameter)");
        while(isalpha(nowChar)) res+=nowChar,next();
        return res;
    }
public:
    Lexer(const std::string &_str):str(_str),pos(0){
        len=str.size();
        nowChar=(pos<len?str[pos]:'\0');
    }
    Token<T> get_token(){
        if(nowChar=='\0') return {Token<T>::eof};
        if(isdigit(nowChar)){
            T a=get_num();
            if(nowChar=='/'){
                next();
                if(isdigit(nowChar)){
                    T b=get_num();
                    return {Token<T>::NUMBER, "", fraction<T>(a,b)};
                }
                prev();
            }
            return {Token<T>::NUMBER, "", fraction<T>(a)};
        }
        if(isalpha(nowChar)){
            std::string str=get_word();
            return {Token<T>::WORD, str, fraction<T>(0)};
        }
        char c=nowChar;next();
        switch(c){
            case '+': return {Token<T>::ADD};
            case '-': return {Token<T>::SUB};
            case '*': return {Token<T>::MUL};
            case '/': return {Token<T>::DIV};
            case '^': return {Token<T>::POW};
            case '(': return {Token<T>::LPAR};
            case ')': return {Token<T>::RPAR};
            default: return {Token<T>::error};
        }
    }
};
template<typename T=int64_t> class Parser{
    Lexer<T> &lex;
    Token<T> nowTok;
    std::string var;
    void next(){
        nowTok=lex.get_token();
    }
    void read(typename Token<T>::Type type){
        if(nowTok.type!=type) throw("Syntax Error (missing token)");
        next();
    }
    std::unique_ptr<Function> get_element(){
        if(nowTok.type==Token<T>::SUB){
            next(); // read '-'
            auto res=get_element();
            return std::make_unique<Mul>(std::make_unique< Constant<> >(-1), std::move(res));
        }
        if(nowTok.type==Token<T>::NUMBER){
            auto res=nowTok.val;
            next(); // read a number
            return std::make_unique< Constant<> >(res);
        }
        if(nowTok.type==Token<T>::WORD){
            std::string name=nowTok.word;
            next(); // read a word
            if(name==var) return std::make_unique<Variable>(name);
            return std::make_unique<Parameter>(name);
        }
        if(nowTok.type==Token<T>::LPAR){
            next(); // read '('
            auto res=parse();
            read(Token<T>::RPAR);
            return res;
        }
        throw("Syntax Error (element)");
        return nullptr;
    }
    std::unique_ptr<Function> get_pow(){
        auto f=get_element();
        while(nowTok.type==Token<T>::POW){
            next(); // read '^'
            if(nowTok.type==Token<T>::LPAR){
                next(); // read '('
                auto g=parse();
                read(Token<T>::RPAR);
                f=std::make_unique<Pow>(std::move(f), std::move(g));
            }
            else{
                auto g=get_element();
                f=std::make_unique<Pow>(std::move(f), std::move(g));
            }
        }
        return f;
    }
    std::unique_ptr<Function> get_mul(){
        auto f=get_pow();
        while(nowTok.type==Token<T>::MUL || nowTok.type==Token<T>::DIV){
            auto op=nowTok;
            next(); // read '*' or '/'
            auto g=get_pow();
            if(op.type==Token<T>::MUL) f=std::make_unique<Mul>(std::move(f), std::move(g));
            else f=std::make_unique<Div>(std::move(f), std::move(g));
        }
        return f;
    }
    std::unique_ptr<Function> get_add(){
        auto f=get_mul();
        while(nowTok.type==Token<T>::ADD || nowTok.type==Token<T>::SUB){
            auto op=nowTok;
            next(); // read '+' or '-'
            auto g=get_mul();
            if(op.type==Token<T>::ADD) f=std::make_unique<Add>(std::move(f), std::move(g));
            else f=std::make_unique<Sub>(std::move(f), std::move(g));
        }
        return f;
    }
public:
    Parser(Lexer<T> &_lex, std::string _var): lex(_lex), var(_var){
        nowTok=lex.get_token();
    }
    std::unique_ptr<Function> parse(){
        return get_add();
    }
};

#include<iostream>

int main(int argc, char *argv[]){
    for(int i=0;i<argc;++i){
        std::string arg=argv[i];
        if(arg=="-latex") LATEX_MODE=true;
    }
    std::string str, var;
    std::getline(std::cin, str);
    std::getline(std::cin, var);
    Lexer<> lex(str);
    Parser<> parser(lex, var);
    auto f=parser.parse();
    printf("f : %s\n",f->to_str().c_str());
    auto df=f->get_der();
    printf("f': %s\n",df->to_str().c_str());

    return 0;
}