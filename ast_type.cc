/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "errors.h"
 
/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double, etc.)
 * They can be accessed with the syntax Type::intType. This allows you to
 * directly access them and share the built-in types where needed rather that
 * creates lots of copies.
 */

Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::nullType   = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType  = new Type("error"); 

Type::Type(const char *n) {
    Assert(n);
    typeName = strdup(n);
}

bool isCompatible(Type* lhs, Type* rhs){
  ArrayType* lhs_array = dynamic_cast<ArrayType*>(lhs);
  ArrayType* rhs_array = dynamic_cast<ArrayType*>(rhs); 
  NamedType* lhs_named = dynamic_cast<NamedType*>(lhs);
  NamedType* rhs_named = dynamic_cast<NamedType*>(rhs);
  if (lhs_array){
    // If the rhs is also an array
    if (rhs_array) {
      ArrayType* lhs_elem_array = dynamic_cast<ArrayType*>(lhs_array->elemType);
      ArrayType* rhs_elem_array = dynamic_cast<ArrayType*>(rhs_array->elemType);
      // Arrays of arrays
      if (lhs_elem_array && rhs_elem_array) {
        return isCompatible(rhs_array->elemType, lhs_array->elemType);
      }
      NamedType* lhs_elem_named = dynamic_cast<NamedType*>(lhs_array->elemType);
      NamedType* rhs_elem_named = dynamic_cast<NamedType*>(rhs_array->elemType);
      if (lhs_elem_named && rhs_elem_named) {
        return lhs_elem_named->GetName() == rhs_elem_named->GetName(); 
      } else {
        return lhs_array->elemType == rhs_array->elemType;
      }
    } else if (lhs == Type::nullType){
      return true;
    } else {
      return false;
    }
  }
  //
  if (lhs_named) {
    if (rhs_named){
      return lhs_named->Compatible(rhs_named);
    } else if (lhs == Type::nullType){
      return true;
    } else {
      return false;
    }
  }
  return lhs == rhs;
}
void Type::Check(){
}

ClassDecl* NamedType::GetClassDecl(){
  std::string name = this->id->name;
  Node* program = this;
  // Keep bubbling up while you have parents
  while (program->parent) {
    program = program->parent;
  }
  if (program->scope.find(name) != program->scope.end()) {
    ClassDecl* class_decl = dynamic_cast<ClassDecl*>(program->scope[name]);
    if (class_decl) {
      return class_decl;
    }
  }
  return NULL;
}

NamedType::NamedType(Identifier *i) : Type(*i->GetLocation()) {
    Assert(i != NULL);
    (id=i)->SetParent(this);
} 

bool NamedType::Compatible(NamedType* other) {
  ClassDecl* my_class = this->GetClassDecl();
  // If the two classes have the same name
  while (my_class) {
    if (my_class->GetName() == other->GetName()) {
      return true;
    }
    for (int i = 0; i < my_class->implements->NumElements(); i++) {
      NamedType* implType = my_class->implements->Nth(i);
      if (implType->GetName() == other->GetName()) {
        return true;
      }
    }
    my_class = my_class->extendedClass;
  }
  return false;
}

bool NamedType::isEquivalentTo(Type *o) {
  std::cout << "Chcking equivlanecy" << std::endl;
  if (o == Type::nullType) {
    std::cout << "NULL";
    return true;
  }
  NamedType* other = dynamic_cast<NamedType*>(o);
  ClassDecl* my_class = this->GetClassDecl();
  // If the two classes have the same name
  while (my_class) {
    if (my_class->GetName() == other->GetName()) {
      return true;
    }
    for (int i = 0; i < my_class->implements->NumElements(); i++) {
      NamedType* implType = my_class->implements->Nth(i);
      if (implType->GetName() == other->GetName()) {
        return true;
      }
    }
    my_class = my_class->extendedClass;
  }
  return false;
}

void NamedType::Check(){
  Node *program = this->parent;
  // Look for parent
  while (program->parent != NULL) {
    program = program->parent;
  }
  Identifier* type_id = this->id;
  std::string type_name = type_id->name;
  // Throw error if namedtype is not in program's scope
  if (program->scope.find(type_name) == program->scope.end()){
    ReportError::IdentifierNotDeclared(type_id, reasonT::LookingForType);
  } 
  // We found the namedtype, must ensure it is a Class/Interface
  else {
    Decl* type_decl = program->scope[type_name];
    ClassDecl* class_check = dynamic_cast<ClassDecl*>(type_decl);
    InterfaceDecl* interface_check = dynamic_cast<InterfaceDecl*>(type_decl);
    if (!(class_check || interface_check)) {
      ReportError::IdentifierNotDeclared(type_id, reasonT::LookingForType);
    }
  }
}

ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc) {
    Assert(et != NULL);
    (elemType=et)->SetParent(this);
}

bool ArrayType::isEquivalentTo(Type *o) {
  ArrayType* other = dynamic_cast<ArrayType*>(o);
  if (!other) {
    return o == Type::nullType;
  }
  return elemType->IsEquivalentTo(other->elemType);
}

void ArrayType::Check(){
  elemType->Check();
}
