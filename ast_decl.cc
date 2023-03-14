/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"
#include <iostream>
#include <string>
#include <vector>
        
std::string Decl::GetName(){
    return this->id->name;
}


Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}


VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}

void VarDecl::Check() {
    // If type is named
    this->type->Check();
    // NamedType* type = dynamic_cast<NamedType*>(this->type);
    // if (type){
    //   Node *program = this->parent;
    //   // Look for parent
    //   while (program->parent != NULL) {
    //     program = program->parent;
    //   }
    //   Identifier* type_id = type->id;
    //   std::string type_name = type_id->name;
    //   // Throw error if namedtype is not in program's scope
    //   if (program->scope.find(type_name) == program->scope.end()){
    //     ReportError::IdentifierNotDeclared(type_id, reasonT::LookingForType);
    //   } 
    //   // We found the namedtype, must ensure it is a Class/Interface
    //   else {
    //     Decl* type_decl = program->scope[type_name];
    //     ClassDecl* class_check = dynamic_cast<ClassDecl*>(type_decl);
    //     InterfaceDecl* interface_check = dynamic_cast<InterfaceDecl*>(type_decl);
    //     if (!(class_check || interface_check)) {
    //       ReportError::IdentifierNotDeclared(type_id, reasonT::LookingForType);
    //     }
    //   }
    // }
}
  

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
    extendedClass = NULL;
}

void ClassDecl::InitClassScope(ClassDecl* base_class){
  // construct scope of class when extends base class
   if (base_class) {
      for (int i = 0; i < this->members->NumElements(); i++) {
         Decl* member = this->members->Nth(i);
         std::string member_name = member->GetName();
         //check if name is in base class scope
         if (base_class->scope.find(member_name) != base_class->scope.end()) {
           Decl* base_decl = base_class->scope[member_name];
           //check if decl is func
           FnDecl *base_function_decl = dynamic_cast<FnDecl*>(base_decl);
           if (base_function_decl) {
             FnDecl* member_function_decl = dynamic_cast<FnDecl*>(member);
             // if this member is also a function
             if (member_function_decl){
              // assign matching extends to member
              member_function_decl->extends = base_function_decl;
             } else {
              ReportError::DeclConflict(member, base_decl);
             }
           } else {
            //else it is a varDecl -> error
            ReportError::DeclConflict(member, base_decl);
           }
         }
         // check if name is in current scope
         else {
           if (this->scope.find(member_name) != this->scope.end()) {
              ReportError::DeclConflict(member, this->scope[member_name]);
           } else {
              this->scope[member_name] = member;
           }
         }
      }
    // Construct scope with no base class
   } else {
     InitScope(this->members);
   }
}
void ClassDecl::Check() {
    Node *program = this->parent;
    while (program->parent != NULL) {
      program = program->parent;
    }
    //check if extends base class exists
    ClassDecl* base_class = NULL;
    if (this->extends) {
      NamedType* base_type = this->extends;
      string base_type_name = base_type->GetName();
      base_class = dynamic_cast<ClassDecl*>(FindDecl(base_type_name));
      // If it's not defined, throw an error
      if (base_class == NULL){
        ReportError::IdentifierNotDeclared(base_type->id, reasonT::LookingForClass);
      } else {
        this->extendedClass = base_class;
      }
    }
   //check if implements interfaces exist
   for (int i = 0; i < this->implements->NumElements(); i++) {
      NamedType* base_type = this->implements->Nth(i);
      string base_type_name = base_type->GetName();
      // If it's not defined, throw an error
      InterfaceDecl *interface = dynamic_cast<InterfaceDecl*>(FindDecl(base_type_name));
      if (!interface){
        ReportError::IdentifierNotDeclared(base_type->id, reasonT::LookingForInterface);
      } else {
        this->interfaces[base_type] = interface;
      }
   }
   InitClassScope(base_class);
   /* 
   If function is inherited from a class, we may want to store this information.
   
   */
   // If this class implements other jawns
   for (int i = 0; i < this->implements->NumElements(); ++i){
    NamedType* interface_type = this->implements->Nth(i);
    InterfaceDecl* interface = this->interfaces[interface_type];
    // Validate Inferfaces
    if (!this->ValidateInterface(interface)) {
      ReportError::InterfaceNotImplemented(this, interface_type);
    }
   }
    // Check the members
   for (int i = 0; i < this->members->NumElements(); ++i){
    Decl* member = this->members->Nth(i);
    member->Check();
   }
}

bool ClassDecl::ValidateInterface(InterfaceDecl* interface){
  // Look through interface members for this class
  for (int i = 0; i < interface->members->NumElements(); ++i){
    Decl* member = interface->members->Nth(i);
    FnDecl* impl_function = dynamic_cast<FnDecl*>(member);
    // If this this thing is legit interface
    if (impl_function) {
      std::string function_name = impl_function->GetName();
      // we don't have jawn
      if (this->scope.find(function_name) == this->scope.end()){
        return false;
      }
      FnDecl* class_func = dynamic_cast<FnDecl*>(this->scope[function_name]);
      // we have it but shit's not a function
      if (!class_func){
        return false;
      }
      // Function found with same name
      class_func->implements = impl_function;
      // shit's not the same function
      if (!impl_function->isSameSignature(class_func)){
        return false;
      }
    }
  }
  return true;
}

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}


void InterfaceDecl::Check(){
  InitScope(members);
  // for (int i = 0; i < members->NumElements(); ++i){
  //   Decl* member = members->Nth(i);

  // }
}
	
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

bool isSameType(Type* first, Type* second){
  NamedType* other_type = dynamic_cast<NamedType*>(second);
  NamedType* our_type = dynamic_cast<NamedType*>(first);
  // If one is a named type and the other is not
  if ((!our_type && other_type) || (our_type && other_type)){
    return false;
  }
  else if (our_type){
    if (our_type->GetName() != other_type->GetName()){
      return false;
    }
  }
  else {
    if (!first->IsEquivalentTo(second)){
      return false;
    } 
  }
  return true;
}

bool FnDecl::isSameSignature(FnDecl *other){
  if (!isSameType(this->returnType, other->returnType)){
    return false;
  }
  if (this->formals->NumElements() != other->formals->NumElements()) {
    return false;
  }
  // Validate formals types
  for (int i = 0; i < this->formals->NumElements(); i++) {
    Type* our_type = this->formals->Nth(i)->type;
    Type* other_type = other->formals->Nth(i)->type;
    if (!isSameType(our_type, other_type)) {
      return false;
    }
  }
  return true;
}

void FnDecl::Check(){
    // Check return type
    returnType->Check();
    // Check formals
    for (int i = 0; i < formals->NumElements(); ++i){
      Decl* formal = formals->Nth(i);
      formal->Check();
    }
     // check inheritance for matching signature
    // overrides and implements
   if (this->extends && this->implements) {
     if (!extends->isSameSignature(implements)) {
       ReportError::OverrideMismatch(this);
     }
     //overrides
   } else if (this->extends) {
     if (!this->isSameSignature(extends)) {
       ReportError::OverrideMismatch(this);
     }
     //implements
   } else if (this->implements) {
     if (!this->isSameSignature(implements)) {
       ReportError::OverrideMismatch(this);
     }
   }
   // initialize scope with parameters
   for (int i = 0; i < formals->NumElements(); ++i){
      Decl* currentDecl = formals->Nth(i);
      const char* name = currentDecl->id->name;
      // report if this var already exists yo
      if (scope.find(name) != scope.end()){
        Decl* oldDecl = scope[name];
        ReportError::DeclConflict(currentDecl, oldDecl);
      } else{
        scope[name] = currentDecl;
      }
    }
    //Check Stmt Body
    if (body) body->Check();
}


