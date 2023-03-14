/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include <iostream>
#include "errors.h"


Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
    // Construct program's scope
    this->InitScope(decls);

    // Checking children
    for (int i = 0; i < decls->NumElements(); ++i){
      decls->Nth(i)->Check();
    }
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::Check() {
   // initialize scope with parameters
   for (int i = 0; i < decls->NumElements(); ++i){
      Decl* currentDecl = decls->Nth(i);
      const char* name = currentDecl->id->name;
      // report if this var already exists yo
      if (scope.find(name) != scope.end()){
        Decl* oldDecl = scope[name];
        ReportError::DeclConflict(currentDecl, oldDecl);
      } else{
        scope[name] = currentDecl;
      }
   }
   // Call check on VarDecls
   for (int i = 0; i < decls->NumElements(); ++i){
      VarDecl* currentDecl = decls->Nth(i);
      currentDecl->Check();
   }
   // Call check on Statements
   for (int i = 0; i < stmts->NumElements(); ++i){
      Stmt* stmt = stmts->Nth(i);
      stmt->Check();
   }
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}


ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

void ReturnStmt::Check() {
    
}
  
PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}


