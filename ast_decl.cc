/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"
#include <string>
        
         
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
    NamedType* type = dynamic_cast<NamedType*>(this->type);
    if (type){
      Node *program = this->parent;
      // Look for parent
      while (program->parent != NULL) {
        program = program->parent;
      }
      Identifier* type_id = type->id;
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
}
  

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
}


InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
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



