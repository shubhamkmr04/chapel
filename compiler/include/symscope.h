#ifndef _SYMSCOPE_H_
#define _SYMSCOPE_H_

#include "chplalloc.h"
#include "map.h"
#include "symbol.h"

enum scopeType {
  // builtins at the global level
  SCOPE_INTRINSIC = -4,
  SCOPE_INTERNAL_PRELUDE = -3,
  SCOPE_PRELUDE = -2,
  SCOPE_POSTPARSE = -1,
  SCOPE_MODULE = 0,           // less is all modules
  SCOPE_PARAM,
  SCOPE_FUNCTION,
  SCOPE_LOCAL,
  SCOPE_FORLOOP,
  SCOPE_FORALLEXPR,
  SCOPE_LETEXPR,
  SCOPE_CLASS,
};

//class SymLink;
class SymtabTraversal;
class ScopeLookupCache;

class SymScope : public gc {
 public:
  scopeType type;
  
  ScopeLookupCache *lookupCache;

  Stmt* stmtContext;  // statement context
  Symbol* symContext; // symbol context
  Expr* exprContext;  // expression context

  SymScope* parent;
  SymScope* child;
  SymScope* sibling;

  Vec<Symbol*> symbols;
  //  AList<SymLink>* syms;

  Map<char*,Vec<FnSymbol*>*> visibleFunctions;

  ChainHashMap<char*, StringHashFns, Symbol*> table;

  SymScope(scopeType init_type);
  void setContext(Stmt* stmt, Symbol* sym = NULL, Expr* expr = NULL);

  void traverse(SymtabTraversal* traversal);

  bool isEmpty(void);
  bool isInternal(void);

  void insert(Symbol* sym);
  void remove(Symbol* sym);

  Symbol* findEnclosingSymContext();
  Stmt* findEnclosingStmtContext();
  Expr* findEnclosingExprContext();

  void print(FILE* outfile = stdout, bool tableOrder = false);

  int parentLength(void);

  // these are "private"
  char* indentStr(void);
  void printHeader(FILE* outfile);
  void printSymbols(FILE* outfile, bool tableOrder);
  void printFooter(FILE* outfile);

  void codegen(FILE* outfile, char* separator);

  bool commonModuleIsFirst();
  ModuleSymbol* SymScope::getModule();
  void setVisibleFunctions(Vec<FnSymbol*>* moreVisibleFunctions);
  void printVisibleFunctions();
};

#endif
