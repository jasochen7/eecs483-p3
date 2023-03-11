/* File: ast.cc
 * ------------
 */

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "errors.h"
#include <string.h> // strdup
#include <stdio.h>  // printf

Node::Node(yyltype loc) {
    location = new yyltype(loc);
    parent = NULL;
}

Node::Node() {
    location = NULL;
    parent = NULL;
}

Decl* Node::FindDecl(std::string id_name) {
    Node *current = this;
    // Look for parent
    while (current != NULL) {
      if (current->scope.find(id_name) != current->scope.end()){
        return current->scope[id_name];
      }
      current = current->parent;
    }
    return NULL;
}

void Node::InitScope(List<Decl*> *decl_list) {
    for (int i = 0; i < decl_list->NumElements(); ++i){
      Decl* currentDecl = decl_list->Nth(i);
      const char* name = currentDecl->id->name;
      // report if this decl already exists yo
      if (scope.find(name) != scope.end()){
        Decl* oldDecl = scope[name];
        ReportError::DeclConflict(currentDecl, oldDecl);
      } else{
        scope[name] = currentDecl;
      }
    }
}
	 
Identifier::Identifier(yyltype loc, const char *n) : Node(loc) {
    name = strdup(n);
} 

