/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.  
 *
 * pp3: You will need to extend the Type classes to implement
 * the type system and rules for type equivalency and compatibility.
 */
 
#ifndef _H_ast_type
#define _H_ast_type

#include "ast.h"
#include "list.h"
#include <iostream>
#include <string>
class ClassDecl;


class Type : public Node 
{
  protected:
    char *typeName;

  public :
    static Type *intType, *doubleType, *boolType, *voidType,
                *nullType, *stringType, *errorType;

    Type(yyltype loc) : Node(loc) {}
    Type(const char *str);
    
    virtual void PrintToStream(std::ostream& out) { out << typeName; }
    friend std::ostream& operator<<(std::ostream& out, Type *t) { t->PrintToStream(out); return out; }
    virtual bool IsEquivalentTo(Type *other) { return this == other; }
    virtual void Check();
};

class NamedType : public Type 
{
  public:
    Identifier *id;
    
  public:
    ClassDecl* GetClassDecl();
    NamedType(Identifier *i);
    void Check();
    bool Compatible(NamedType* other);
    bool isEquivalentTo(Type* other);
    void PrintToStream(std::ostream& out) { out << id; }
    std::string GetName() {
      return id->name;
    }
};

class ArrayType : public Type 
{
  public:
    Type *elemType;
    ArrayType(yyltype loc, Type *elemType);
    void Check();
    bool isEquivalentTo(Type* other);
    void PrintToStream(std::ostream& out) { out << elemType << "[]"; }
};

bool isCompatible(Type* lhs, Type* rhs);
#endif
