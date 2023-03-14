/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>
#include "errors.h"


void Expr::Check(){
  Type* type = GetType();
}

Type* Expr::GetType(){
  return Type::errorType;
}


IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

Type* IntConstant::GetType() {
    return Type::intType;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}

Type* DoubleConstant::GetType() {
    return Type::doubleType;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}

Type* BoolConstant::GetType() {
  return Type::boolType;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
}

Type* StringConstant::GetType() {
  return Type::stringType;
}

Type* NullConstant::GetType() {
  return Type::nullType;
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

Type* CompoundExpr::GetType(){
  return Type::errorType;
}

Type* ArithmeticExpr::GetType(){
  Type* rhs_type = right->GetType();
  if (rhs_type == Type::errorType) return Type::errorType;
  // Binary
  if (left) {
    Type* lhs_type = left->GetType();
    // Lhs type is bogus
    if (lhs_type == Type::errorType){
      return Type::errorType;
    }
    // Types are not the same
    if (lhs_type != rhs_type){
      // Only report if BOTH TYPES are not error types
      if (lhs_type != Type::errorType && rhs_type != Type::errorType) {
        ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      }
      return Type::errorType;
    }
    // Type is not double or int
    if (lhs_type != Type::intType || lhs_type != Type::doubleType){
      // Only report if BOTH TYPES are not error types
      if (lhs_type != Type::errorType && rhs_type != Type::errorType) {
        ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      }
      return Type::errorType;
    }
    return lhs_type;
  }
  // Unary minus
  else{
    // Type is not double or int
    if (rhs_type != Type::intType || rhs_type != Type::doubleType) {
      // Only report if type is not error type
      if (rhs_type != Type::errorType) {
        ReportError::IncompatibleOperand(op, rhs_type);
      }
      return Type::errorType;
    }
    return rhs_type;
  }
}

Type* RelationalExpr::GetType(){
  // <, >, <=, >=
  Type* rhs_type = right->GetType();
  Type* lhs_type = left->GetType();
  if (rhs_type == Type::errorType || lhs_type == Type::errorType){
    return Type::boolType;
  }
  if (rhs_type != lhs_type) {
    // Only report if both types are not errors
    if (rhs_type != Type::errorType && lhs_type != Type::errorType){
       ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
    }
    return Type::boolType;
  } else{
    if (rhs_type != Type::doubleType || rhs_type != Type::intType){
      // Only report if both types are not errors
      if (rhs_type != Type::errorType && lhs_type != Type::errorType){
        ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      }
      return Type::boolType;
    }
  }
  return Type::boolType;
}

Type* EqualityExpr::GetType() {
  Type* rhs_type = right->GetType();
  Type* lhs_type = left->GetType();
  NamedType* named_rhs = dynamic_cast<NamedType*>(rhs_type);
  NamedType* named_lhs = dynamic_cast<NamedType*>(lhs_type);
  // If both are named types
  if (named_lhs && named_rhs) {
    // If the named tyoes are compatible
    if (named_lhs->Compatible(named_rhs) || named_rhs->Compatible(named_lhs)) {
      return Type::boolType;
    // If the names don't match
    } else {
      ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      return Type::errorType;
    }
  }
  // If one is a named type and the other is not 
  else if (named_lhs || named_rhs){
    // comparision between object and nullType
    if (lhs_type == Type::nullType || rhs_type == Type::nullType) {
      return Type::boolType;
    } else {
      if (lhs_type != Type::errorType && rhs_type != Type::errorType){
        ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      }
      return Type::boolType;
    }
  }
  // both are basic types
  else{
    // The types match
    if (rhs_type == lhs_type){
      return Type::boolType;
    } else {
      if (lhs_type != Type::errorType && rhs_type != Type::errorType){
         ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      }
      return Type::boolType;
    }
  }
}

Type* LogicalExpr::GetType(){
  // && , ||, unary !
  Type* rhs_type = right->GetType();
  if (left) {
    Type* lhs_type = left->GetType();
    // >= 1 of the two is not bool
    if (lhs_type != Type::boolType || rhs_type != Type::boolType) {
      ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      return Type::errorType;
    } else{
      return Type::boolType;
    }
  }
  // unary !
  else {
    // Jawn is not bool
    if (rhs_type != Type::boolType){
      ReportError::IncompatibleOperand(op, rhs_type);
    } else {
      return Type::boolType;
    }
  }
}

Type *AssignExpr::GetType() {
  Type* lhs_type = left->GetType();
  Type* rhs_type = right->GetType();
  NamedType* named_lhs = dynamic_cast<NamedType*>(lhs_type);
  NamedType* named_rhs = dynamic_cast<NamedType*>(rhs_type);
  // Both types named
  if (named_lhs && named_rhs) {
    // If they are compatible
    if (named_rhs->Compatible(named_lhs)) {
      return named_lhs;
    }
    // If they're not
    else {
      ReportError::IncompatibleOperands(op, named_lhs, named_rhs);
      return Type::errorType;
    }
  }
  // If lhs not named but rhs is - this is bs
  else if (!named_lhs && named_rhs){
    ReportError::IncompatibleOperands(op, lhs_type, named_rhs);
    return Type::errorType;
  }
  // If rhs not named - must be null
  else if (!named_rhs && named_lhs){
    // If rhs is null
    if (rhs_type == Type::nullType){
      return named_lhs;
    }
    // If not
    else {
      ReportError::IncompatibleOperands(op, named_lhs, rhs_type);
      return Type::errorType;
    }
  } 
  // Neither are named
  else if (!named_lhs && !named_rhs){
    if (lhs_type == rhs_type){
      return lhs_type;
    } else {
      ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      return Type::errorType;
    }
  }
}

// for polymorphism
Type* LValue::GetType(){
  return Type::errorType;
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}
   
Type* This::GetType(){
    Node* current_node = this->GetParent();
    ClassDecl* parent_class = NULL;
    while (current_node){
      ClassDecl* this_class = dynamic_cast<ClassDecl*>(current_node);
      if (this_class){
        parent_class = this_class;
        break;
      }
      current_node = current_node->GetParent();
    }
   
    if (!parent_class){
      ReportError::ThisOutsideClassScope(this);
      return Type::errorType;
    } else {
      return new NamedType(parent_class->id);
    }
}
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

Type* ArrayAccess::GetType() {
    Type* subscript_type = subscript->GetType();
    // subscript not an int
    if (subscript_type != Type::intType) {
      ReportError::SubscriptNotInteger(subscript);
      return Type::errorType;
    }
    Type* base_type = base->GetType();
    ArrayType* base_array_type = dynamic_cast<ArrayType*>(base_type);
    // Base expr is of array type;
    if (base_array_type) {
      return base_array_type->elemType;
    }
    // Base expr is not of array type;
    ReportError::BracketsOnNonArray(base);
    return Type::errorType;
}
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}

Type* FieldAccess::GetType(){
  if (!base){
    std::string name = field->name;
    Node* parent = this;
    Type* result = NULL;
    while (parent){
      // If we have found the variable of question
      if (parent->scope.find(name) != parent->scope.end()){
        VarDecl* var = dynamic_cast<VarDecl*>(parent->scope[name]);
        if (var) {
          return var->type;
        }
      }
      parent = parent->parent;
    }
    ReportError::IdentifierNotDeclared(field,reasonT::LookingForVariable);
    return Type::errorType;
  }
  // If we have a base class
  if (base){
    Type* base_type = base->GetType();
    // base type not found
    if (base_type == Type::errorType) {
      return Type::errorType;
    }
    NamedType* named_type = dynamic_cast<NamedType*>(base_type);
    if (named_type) {
      std::string class_name = named_type->GetName();
      Node *current = this;
      while (current->parent) {
        current = current->parent;
      }
      Decl* lookup = current->scope[class_name];
      ClassDecl* class_decl= dynamic_cast<ClassDecl*>(lookup);
      if (class_decl) {
        std::string var_name = field->name;
        VarDecl* found_var_decl = NULL;
        //variable not found in class scope
        if (class_decl->scope.find(var_name) == class_decl->scope.end()) {
          // check in extends
          ClassDecl* extended_class = class_decl->extendedClass;
          while (extended_class) {
            //found var name in a base class
            if (extended_class->scope.find(var_name) != extended_class->scope.end()) {
              found_var_decl = dynamic_cast<VarDecl*>(extended_class->scope[var_name]);
              if (found_var_decl) {
                break;
              }
            }
            extended_class = extended_class->extendedClass;
          }
          if (!found_var_decl) {
            ReportError::IdentifierNotDeclared(field, LookingForVariable);
            return Type::errorType;
          }
        }
        // variable found in class scope
        else {
          found_var_decl = dynamic_cast<VarDecl*>(class_decl->scope[var_name]);
        }

        //check for if var_decl was found
        if (found_var_decl) {
          return found_var_decl->type;
        } else {
          ReportError::IdentifierNotDeclared(field, LookingForVariable);
          return Type::errorType;
        }

      }
      else {
        ReportError::IdentifierNotDeclared(named_type->id, reasonT::LookingForClass);
        return Type::errorType;
      }
    }
    // not a named type: i have no clue how
    else {
      return Type::errorType;
    }
  }

  return Type::errorType;
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}
 

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}


NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this);
    (elemType=et)->SetParent(this);
}

       
