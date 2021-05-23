/*
 * Copyright 2021 Hewlett Packard Enterprise Development LP
 * Other additional copyright holders may be indicated within.
 *
 * The entirety of this work is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CHPL_UAST_FOREACH_H
#define CHPL_UAST_FOREACH_H

#include "chpl/queries/Location.h"
#include "chpl/uast/IndexableLoop.h"
#include "chpl/uast/WithClause.h"

namespace chpl {
namespace uast {


/**
  This class represents a foreach loop. Foreach example:

  \rst
  .. code-block:: chapel

      // Example 1:
      var x: atomic int;
      foreach i in myRange with (ref x) {
        x.fetchAdd(i);
      }

  \endrst

 */
class Foreach final : public IndexableLoop {
 private:
  // Compute the start of the loop body.
  int8_t computeLoopBodyChildNum(int8_t indexVariableChildNum,
                                 int8_t iterandChildNum,
                                 int8_t withClauseChildNum) {
    return (indexVariableChildNum >= 0) + (iterandChildNum >= 0) +
           (withClauseChildNum >= 0);
  }

  Foreach(ASTList children, int8_t indexVariableChildNum,
          int8_t iterandChildNum,
          int8_t withClauseChildNum,
          bool usesDo)
    : IndexableLoop(asttags::Foreach, std::move(children),
                    indexVariableChildNum,
                    iterandChildNum,
                    computeLoopBodyChildNum(indexVariableChildNum,
                                            iterandChildNum,
                                            withClauseChildNum),
                    usesDo),
      withClauseChildNum_(withClauseChildNum) {

    assert(isExpressionASTList(children_));
  }

  bool contentsMatchInner(const ASTNode* other) const override;
  void markUniqueStringsInner(Context* context) const override;

  int8_t withClauseChildNum_;

 public:
  ~Foreach() override = default;

  /**
    Create and return a foreach loop. 
  */
  static owned<Foreach> build(Builder* builder, Location loc,
                              owned<Decl> indexVariable,
                              owned<Expression> iterand,
                              owned<WithClause> withClause,
                              ASTList stmts,
                              bool usesDo);

  /**
    Create and return a foreach loop. 
  */
  static owned<Foreach> build(Builder* builder, Location loc,
                              owned<Decl> indexVariable,
                              owned<Expression> iterand,
                              ASTList stmts,
                              bool usesDo);

  /**
    Create and return a foreach loop. 
  */
  static owned<Foreach> build(Builder* builder, Location loc,
                              owned<Expression> iterand,
                              owned<WithClause> withClause,
                              ASTList stmts,
                              bool usesDo);

  /**
    Create and return a foreach loop.
  */
  static owned<Foreach> build(Builder* builder, Location loc,
                              owned<Expression> iterand,
                              ASTList stmts,
                              bool usesDo);

  /**
    Returns the with clause of this foreach loop, or nullptr if it does
    not exist.
  */
  const WithClause* withClause() const {
    if (withClauseChildNum_ < 0) return nullptr;
    auto ret = child(withClauseChildNum_);
    assert(ret->isWithClause());
    return (const WithClause*)ret;
  }

};


} // end namespace uast
} // end namespace chpl

#endif
