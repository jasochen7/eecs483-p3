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
  // Binary
  if (left) {
    Type* lhs_type = left->GetType();
    Type* rhs_type = right->GetType();
    if (rhs_type == Type::errorType || lhs_type == Type::errorType) return Type::errorType;
    // Types are not the same
    if (!isCompatible(rhs_type, lhs_type)){
      // std::cout << "equivalency issue" << std::endl;
      ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      return Type::errorType;
    }
    // Type is not double or int
    if (lhs_type != Type::intType && lhs_type != Type::doubleType){
      ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      return Type::errorType;
    }
    return lhs_type;
  }
  // Unary minus
  else{
    Type* rhs_type = right->GetType();
    if (rhs_type == Type::errorType) return Type::errorType;
    // Type is not double or int
    if (rhs_type != Type::intType && rhs_type != Type::doubleType) {
      ReportError::IncompatibleOperand(op, rhs_type);
      return Type::errorType;
    }
    return rhs_type;
  }
}

Type* RelationalExpr::GetType(){
  // <, >, <=, >=
  Type* lhs_type = left->GetType();
  Type* rhs_type = right->GetType();
  // Return errortype if either is error
  if (rhs_type == Type::errorType || lhs_type == Type::errorType){
    return Type::boolType;
  }
  // If the types aren't compatible
  if (!isCompatible(lhs_type, rhs_type)) {
    ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
    return Type::boolType;
  } else{
    // Not Double and not int
    if (rhs_type != Type::doubleType && rhs_type != Type::intType){
      ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
      return Type::boolType;
    }
  }
  return Type::boolType;
}

Type* EqualityExpr::GetType() {
  Type* lhs_type = left->GetType();
  Type* rhs_type = right->GetType();
  // return errortype if either is errortype
  if (rhs_type == Type::errorType || lhs_type == Type::errorType){
    return Type::boolType;
  }
  if (!isCompatible(lhs_type, rhs_type) && !isCompatible(rhs_type, lhs_type)){
    ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
  }
  return Type::boolType;
  // NamedType* named_rhs = dynamic_cast<NamedType*>(rhs_type);
  // NamedType* named_lhs = dynamic_cast<NamedType*>(lhs_type);
  // // If both are named types
  // if (named_lhs && named_rhs) {
  //   // If the named types are compatible
  //   if (named || named_rhs->Compatible(named_lhs)) {
  //     return Type::boolType;
  //   // If the names don't match
  //   } else {
  //     ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
  //     return Type::boolType;
  //   }
  // }
  // // If one is a named type and the other is not 
  // else if (named_lhs || named_rhs){
  //   // comparision between object and nullType
  //   if (lhs_type == Type::nullType || rhs_type == Type::nullType) {
  //     return Type::boolType;
  //   } else {
  //     ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
  //     return Type::boolType;
  //   }
  // }
  // // both are basic types
  // else{
  //   // The types match
  //   if (rhs_type == lhs_type){
  //     return Type::boolType;
  //   } else {
  //     ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
  //     return Type::boolType;
  //   }
  // }
}

Type* LogicalExpr::GetType(){
  // && , ||, unary !
  Type* rhs_type = right->GetType();
  if (rhs_type == Type::errorType) {
    return Type::boolType;
  }
  if (left) {
    Type* lhs_type = left->GetType();
    if (lhs_type == Type::errorType) {
      return Type::boolType;
    }
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
  if (lhs_type == Type::errorType || rhs_type == Type::errorType) {
    return Type::errorType;
  }
  // If lhs not compatible with rhs
  if (!isCompatible(rhs_type, lhs_type)){
    ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
    return Type::errorType;
  } else{
    return lhs_type;
  }
  // NamedType* named_lhs = dynamic_cast<NamedType*>(lhs_type);
  // NamedType* named_rhs = dynamic_cast<NamedType*>(rhs_type);
  // // Both types named
  // if (named_lhs && named_rhs) {
  //   // If they are compatible
  //   if (named_rhs->Compatible(named_lhs)) {
  //     return named_lhs;
  //   }
  //   // If they're not
  //   else {
  //     ReportError::IncompatibleOperands(op, named_lhs, named_rhs);
  //     return Type::errorType;
  //   }
  // }
  // // If lhs not named but rhs is - this is bs
  // else if (!named_lhs && named_rhs){
  //   ReportError::IncompatibleOperands(op, lhs_type, named_rhs);
  //   return Type::errorType;
  // }
  // // If rhs not named - must be null
  // else if (!named_rhs && named_lhs){
  //   // If rhs is null
  //   if (rhs_type == Type::nullType){
  //     return named_lhs;
  //   }
  //   // If not
  //   else {
  //     ReportError::IncompatibleOperands(op, named_lhs, rhs_type);
  //     return Type::errorType;
  //   }
  // } 
  // // Neither are named
  // else if (!named_lhs && !named_rhs){
  //   if (lhs_type == rhs_type){
  //     return lhs_type;
  //   } else {
  //     ReportError::IncompatibleOperands(op, lhs_type, rhs_type);
  //     return Type::errorType;
  //   }
  // }
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
    Type* base_type = base->GetType();
    // if subscript or base is bullshit
    if (subscript_type == Type::errorType || base_type == Type::errorType){
      return Type::errorType;
    } 
    // subscript not an int
    if (subscript_type != Type::intType) {
      ReportError::SubscriptNotInteger(subscript);
      return Type::errorType;
    }
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
    // Look for the variable up your scope ladder
    while (parent){
      // If we have found the variable of question
      if (parent->scope.find(name) != parent->scope.end()){
        VarDecl* var = dynamic_cast<VarDecl*>(parent->scope[name]);
        // Var has been found, return
        if (var) {
          return var->type;
        }
      }
      parent = parent->parent;
    }
    // Var hasn't been found, return error
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
    // Case base to named_type
    NamedType* named_type = dynamic_cast<NamedType*>(base_type);
    // If base is valid named_type
    if (named_type) {
      // Look for class name in parent scopes
      std::string class_name = named_type->GetName();
      Node *current = this;
      while (current->parent) {
        current = current->parent;
      }
      // Procure class Decl of this class name
      Decl* lookup = current->scope[class_name];
      ClassDecl* class_decl= dynamic_cast<ClassDecl*>(lookup);
      // If this class decl is valid, validate the var within the class
      if (class_decl) {
        std::string var_name = field->name;
        VarDecl* found_var_decl = NULL;
        ClassDecl* found_class = NULL;
        //variable not found in current class scope
        if (class_decl->scope.find(var_name) == class_decl->scope.end()) {
          // check in extends
          ClassDecl* extended_class = class_decl->extendedClass;
          while (extended_class) {
            //found var name in a base class
            if (extended_class->scope.find(var_name) != extended_class->scope.end()) {
              found_var_decl = dynamic_cast<VarDecl*>(extended_class->scope[var_name]);
              // Update the class to the one you found it in
              if (found_var_decl) {
                found_class = extended_class;
                break;
              }
            }
            extended_class = extended_class->extendedClass;
          }
          // If you never find it, this var is bogus
          if (!found_var_decl) {
            ReportError::IdentifierNotDeclared(field, LookingForVariable);
            return Type::errorType;
          }
        }
        // variable found in current class scope
        else {
          found_var_decl = dynamic_cast<VarDecl*>(class_decl->scope[var_name]);
          found_class = class_decl;
        }

        //check for if var_decl was found within class scope
        if (found_var_decl) {
          Node *current = this;
          while (current) {
            ClassDecl* current_class = dynamic_cast<ClassDecl*>(current);
            while (current_class) {
              if (current_class->GetName() == found_class->GetName()) {
                return found_var_decl->type;
              }
              current_class = current_class->extendedClass;
            }
            current = current->parent;
          }
          ReportError::InaccessibleField(field, base_type);
          return Type::errorType;
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
    // not a named type
    else {
      ReportError::FieldNotFoundInBase(field, base_type);
      return Type::errorType;
    }
  }
  std::cout << "reached end" << std::endl;
  return Type::errorType;
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

void Call::CheckActuals() {
  for (int i = 0; i < actuals->NumElements(); ++i){
    //std::cout << "checking actual: " << i << std::endl;
    Expr* actual = actuals->Nth(i);
    Type* actual_type = actual->GetType();
  }
}

Type* Call::GetType() {
    // for function of unspecified base
    FnDecl* found_func = NULL;
    if (!base) {
      std::string name = field->name;
      Node* parent = this;
      Type* result = NULL;
      while (parent){
        // If we have found the function of question
        if (parent->scope.find(name) != parent->scope.end()){
          FnDecl* func = dynamic_cast<FnDecl*>(parent->scope[name]);
          if (func) {
            found_func = func;
            break;
          }
        }
        parent = parent->parent;
      }
    }
    // there is a base
    else {
      Type* base_type = base->GetType();
      if (base_type == Type::errorType) {
        CheckActuals();
        return Type::errorType;
      }
      ArrayType* array_base = dynamic_cast<ArrayType*>(base_type);
      // If the base type is an array, length or bust
      if (array_base) {
        if (strcmp(field->name, "length")){
          ReportError::FieldNotFoundInBase(field, array_base);
          CheckActuals();
          return Type::errorType;
        }
        // calling length on ArrayType
        else {
          //verify args is 0 for length()
          if (actuals->NumElements() != 0){
            ReportError::NumArgsMismatch(field, 0, actuals->NumElements());
            CheckActuals();
            return Type::errorType;
          }
          // Return int
          else {
            return Type::intType;
          }
        }
      }
      // Else type is named
      NamedType* named_base = dynamic_cast<NamedType*>(base_type);
      if (named_base){
        std::string class_name = named_base->GetName();
        Node *current = this;
        while (current->parent) {
          current = current->parent;
        }
        Decl* lookup = current->scope[class_name];
        ClassDecl* class_decl= dynamic_cast<ClassDecl*>(lookup);
        InterfaceDecl* intf_decl = dynamic_cast<InterfaceDecl*>(lookup);
        //base is a class
        if (class_decl) {
          ClassDecl* current = class_decl;
          while (current){
          // If we have found the function of question
            if (current->scope.find(field->name) != current->scope.end()){
              FnDecl* func = dynamic_cast<FnDecl*>(current->scope[field->name]);
              if (func) {
                found_func = func;
                break;
              }
            }
            if (!current->extends) {
              break;
            }
            current = current->extends->GetClassDecl();
          }
        }
        // base is an interface
        else if (intf_decl) {
          // If this field exists within the interface
          if (intf_decl->scope.find(field->name) != intf_decl->scope.end()) {
            FnDecl* func = dynamic_cast<FnDecl*>(current->scope[field->name]);
            if (func) {
              if (func->implementedBy.size() > 0) {
                found_func = func;
              }
            }
          }
        }
      }
      // If base is not namedtype
      else {
          ReportError::FieldNotFoundInBase(field, base_type);
          CheckActuals();
          return Type::errorType;
      }
    }
    if (found_func){
      int expected_args = found_func->formals->NumElements();
      int given_args = actuals->NumElements();
      // If num args not the same
      if (expected_args != given_args){
        ReportError::NumArgsMismatch(field, expected_args, given_args);
        CheckActuals();
        return Type::errorType;
      }
      // Look through args and match them
      for (int i = 0; i < actuals->NumElements(); ++i){
        //std::cout << "checking actual: " << i << std::endl;
        Expr* actual = actuals->Nth(i);
        Type* actual_type = actual->GetType();
        Type* formal_type = found_func->formals->Nth(i)->type;
        if (!isCompatible(actual_type, formal_type)){
          ReportError::ArgMismatch(actual, i + 1, actual_type, formal_type);
          return Type::errorType;
        }
      }
      return found_func->returnType;
    // This field WAS NOT FOUND
    } else {
      // If there is a base
      if (base) {
        ReportError::FieldNotFoundInBase(field, base->GetType());
        CheckActuals();
        return Type::errorType;
      } else {
        ReportError::IdentifierNotDeclared(field, reasonT::LookingForFunction);
        CheckActuals();
        return Type::errorType;
      }
    }
}
 

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}

Type* NewExpr::GetType() {
  ClassDecl* class_decl = cType->GetClassDecl();
  if (class_decl == NULL) {
    ReportError::IdentifierNotDeclared(cType->id, reasonT::LookingForClass);
    return Type::errorType;
  }
  return cType;
}

NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this);
    (elemType=et)->SetParent(this);
}

Type* NewArrayExpr::GetType() {
    Type* size_type = size->GetType();
    bool errored = false;
    if (size_type == Type::errorType){
    }
    else if (size_type != Type::intType) {
      ReportError::NewArraySizeNotInteger(size);
      // errored = true;
    }
    else if (elemType == Type::errorType) {
      errored = true;
    }
    elemType->Check();
    // NamedType *obj_type = dynamic_cast<NamedType*>(elemType);
    // if (obj_type) {
    //   ClassDecl* class_decl = obj_type->GetClassDecl();
    //   if (class_decl == NULL) {
    //     ReportError::IdentifierNotDeclared(obj_type->id, reasonT::LookingForClass);
    //     return Type::errorType;
    //   }
    // }
    // // If the element is an array
    // ArrayType *arr_type = dynamic_cast<ArrayType*>(elemType);
    // if (arr_type) {
    //   NamedType* named_arr = dynamic_cast<NamedType*>(arr_type->elemType);
    //   named_arr->Check();
    // }
    //std::cout << errored << std::endl;
    return errored ? Type::errorType : new ArrayType(*this->location, elemType);
}

       
