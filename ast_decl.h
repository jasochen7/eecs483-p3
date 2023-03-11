/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp3: You will need to extend the Decl classes to implement 
 * semantic processing including detection of declaration conflicts 
 * and managing scoping issues.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "ast_type.h"
#include "list.h"
#include <vector>

class Identifier;
class Stmt;

class Decl : public Node 
{
  public:
    Identifier *id;
    std::string GetName();
    Decl(Identifier *name);
    friend std::ostream& operator<<(std::ostream& out, Decl *d) { return out << d->id; }
};

class VarDecl : public Decl 
{
  public:
    Type *type;
    VarDecl(Identifier *name, Type *type);
    void Check();
};

class InterfaceDecl : public Decl 
{
  public:
    List<Decl*> *members;
    InterfaceDecl(Identifier *name, List<Decl*> *members);
};

class ClassDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    NamedType *extends;
    ClassDecl* extendedClass;
    List<NamedType*> *implements;
    std::unordered_map<NamedType*, InterfaceDecl*> interfaces;

  public:
    bool ValidateInterface(InterfaceDecl* interface);
    ClassDecl(Identifier *name, NamedType *extends, 
              List<NamedType*> *implements, List<Decl*> *members);
    void Check();
};

class FnDecl : public Decl 
{
  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
    
  public:
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    void SetFunctionBody(Stmt *b);
    bool isSameSignature(FnDecl* other);
};

#endif
