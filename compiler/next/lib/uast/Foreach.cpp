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

#include "chpl/uast/Foreach.h"

#include "chpl/uast/Builder.h"

namespace chpl {
namespace uast {


bool Foreach::contentsMatchInner(const ASTNode* other) const {
  const Foreach* lhs = this;
  const Foreach* rhs = (const Foreach*) other;

  if (!lhs->indexableLoopContentsMatchInner(rhs))
    return false;

  if (lhs->withClauseChildNum_ != rhs->withClauseChildNum_)
    return false;

  return true;
}

void Foreach::markUniqueStringsInner(Context* context) const {
  indexableLoopMarkUniqueStringsInner(context);
}

owned<Foreach> Foreach::build(Builder* builder,
                              Location loc,
                              owned<Decl> indexVariable,
                              owned<Expression> iterand,
                              owned<WithClause> withClause,
                              ASTList stmts,
                              bool usesDo) {
  assert(iterand.get() != nullptr);

  ASTList lst;
  int8_t indexVariableChildNum = -1;
  int8_t iterandChildNum = -1;
  int8_t withClauseChildNum = -1;

  if (indexVariable.get() != nullptr) {
    indexVariableChildNum = lst.size();
    lst.push_back(std::move(indexVariable));
  }

  if (iterand.get() != nullptr) {
    iterandChildNum = lst.size();
    lst.push_back(std::move(iterand));
  }

  if (withClause.get() != nullptr) {
    withClauseChildNum = lst.size();
    lst.push_back(std::move(withClause));
  }

  for (auto& stmt : stmts) {
    lst.push_back(std::move(stmt));
  }

  Foreach* ret = new Foreach(std::move(lst), indexVariableChildNum,
                           iterandChildNum,
                           withClauseChildNum,
                           usesDo);
  builder->noteLocation(ret, loc);
  return toOwned(ret);
}

owned<Foreach> Foreach::build(Builder* builder, Location loc,
                              owned<Decl> indexVariable,
                              owned<Expression> iterand,
                              ASTList stmts,
                              bool usesDo) {
  return Foreach::build(builder, loc, std::move(indexVariable),
                        std::move(iterand),
                        /*withClause*/ nullptr,
                        std::move(stmts),
                        usesDo);
}

owned<Foreach> Foreach::build(Builder* builder, Location loc,
                              owned<Expression> iterand,
                              owned<WithClause> withClause,
                              ASTList stmts,
                              bool usesDo) {
  return Foreach::build(builder, loc, /*indexVariable*/ nullptr,
                        std::move(iterand),
                        std::move(withClause),
                        std::move(stmts),
                        usesDo);
}

owned<Foreach> Foreach::build(Builder* builder, Location loc,
                              owned<Expression> iterand,
                              ASTList stmts,
                              bool usesDo) {
  return Foreach::build(builder, loc, /*indexVariable*/ nullptr,
                        std::move(iterand),
                        /*withClause*/ nullptr,
                        std::move(stmts),
                        usesDo);
}


} // namespace uast
} // namespace chpl
