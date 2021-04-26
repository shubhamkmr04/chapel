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

#include "chpl/Queries/Context.h"

#include "chpl/Queries/QueryImpl.h"

#include <cstdlib>
#include <cassert>

namespace chpl {

using namespace chpl::querydetail;

Context::Context()
  : uniqueStringsTable(), queryDB(), queryDeps(),
    currentRevisionNumber(1), lastPrepareToGCRevisionNumber(0), gcCounter(1) {
}

owned<Context> Context::build() {
  return toOwned(new Context());
}

Context::~Context() {
  // free all of the unique'd strings
  for (auto& item: this->uniqueStringsTable) {
    const char* s = item.second;
    free((void*)s);
  }
}

#define ALIGN_DN(i, size)  ((i) & ~((size) - 1))
#define ALIGN_UP(i, size)  ALIGN_DN((i) + (size) - 1, size)

static char* allocateEvenAligned(size_t amt) {
  char* buf = (char*) malloc(amt);
  // Normally, malloc returns something that is aligned to 16 bytes,
  // but it's technically possible that a platform library
  // could not do so. So, here we check.
  // We only need even alignment here.
  if ((((uintptr_t)buf) & 1) != 0) {
    free(buf);
    // try again with an aligned allocation
    size_t alignment = sizeof(void *);
    size_t align_up_len = ALIGN_UP(amt, sizeof(void*));
    buf = (char*) aligned_alloc(alignment, align_up_len);
  }
  assert(buf);
  assert((((uintptr_t)buf) & 1) == 0);
  return buf;
}

const char* Context::getOrCreateUniqueString(const char* str) {
  char gcMark = this->gcCounter & 0xff;
  auto search = this->uniqueStringsTable.find(str);
  if (search != this->uniqueStringsTable.end()) {
    char* buf = search->second;
    // update the GC mark
    // Performance: Would it be better to do this store unconditionally?
    if (this->currentRevisionNumber == this->lastPrepareToGCRevisionNumber) {
      buf[0] = gcMark;
    }
    const char* key = buf+2; // pass the 2 bytes of metadata
    return key;
  }
  size_t strLen = strlen(str);
  size_t allocLen = strLen+3; // 2 bytes of metadata, str data, '\0'
  char* buf = allocateEvenAligned(allocLen);
  // set the GC mark
  buf[0] = gcMark;
  // set the unused metadata (need to still have even alignment)
  buf[1] = 0x02;
  // copy the string data, including the null terminator
  memcpy(buf+2, str, strLen+1);
  const char* key = buf+2; // pass the 2 bytes of metadata
  // Add it to the table
  this->uniqueStringsTable.insert(search, {key, buf});
  return key;
}

const char* Context::uniqueCString(const char* s) {
  if (s == nullptr) s = "";
  return this->getOrCreateUniqueString(s);
}

UniqueString Context::moduleNameForID(ID id) {
  // If the symbol path is empty, this ID doesn't have a module.
  if (id.symbolPath().isEmpty()) {
    UniqueString empty;
    return empty;
  }

  // Otherwise, the module name is everything up to the first '.'
  size_t len = 0;
  const char* s = id.symbolPath().c_str();
  while (true) {
    if (s[len] == '\0' || s[len] == '.') break;
    len++;
  }

  return UniqueString::build(this, s, len);
}

UniqueString Context::filePathForID(ID id) {
  UniqueString modName = this->moduleNameForID(id);
  return this->filePathForModuleName(modName);
}

UniqueString Context::filePathForModuleName(UniqueString modName) {
  QUERY_BEGIN_NAMED(this, UniqueString, "filePathForModuleName", modName);
  if (QUERY_USE_SAVED()) {
    return QUERY_GET_SAVED();
  }
  assert(false && "This query should always use a saved result");
  auto result = UniqueString::build(this, "<unknown file path>");
  return QUERY_END(result);
}

void Context::advanceToNextRevision(bool prepareToGC) {
  this->currentRevisionNumber++;
  if (prepareToGC) {
    this->lastPrepareToGCRevisionNumber = this->currentRevisionNumber;
    gcCounter++;
  }
  printf("CURRENT REVISION NUMBER IS NOW %i\n",
         (int) this->currentRevisionNumber);
}

void Context::collectGarbage() {
  // if there are no parent queries, we can clear out the saved oldResults
  if (queryDeps.size() == 0 &&
      this->lastPrepareToGCRevisionNumber == this->currentRevisionNumber) {
    // warning: these loops proceeds in a nondeterministic order
    for (auto& dbEntry: queryDB) {
      QueryMapBase* queryMapBase = dbEntry.second.get();
      queryMapBase->clearOldResults(this->currentRevisionNumber);
    }
    // Performance: Would it be better to modify the table in-place
    // rather than creating a new table as is done here?
    char gcMark = this->gcCounter & 0xff;
    UniqueStringsTableType newTable;
    std::vector<char*> toFree;
    for (auto& e: uniqueStringsTable) {
      const char* key = e.first;
      char* buf = e.second;
      if (buf[0] == gcMark) {
        newTable.insert(std::make_pair(key, buf));
      } else {
        toFree.push_back(buf);
      }
    }
    for (char* buf: toFree) {
      free(buf);
    }
    uniqueStringsTable.swap(newTable);
  }
}

bool Context::setFilePathForModuleName(UniqueString modName, UniqueString path) {
  UniqueString queryName = UniqueString::build(this, "filePathForModuleName");
  auto tupleOfArgs = std::make_tuple(modName);
  bool changed = false;
  auto queryMapResult = updateResultForQuery(tupleOfArgs, path,
                                             changed, queryName);
  return changed;
}

bool Context::setFileText(UniqueString path, std::string data) {
  UniqueString queryName = UniqueString::build(this, "fileText");
  auto tupleOfArgs = std::make_tuple(path);
  bool changed = false;
  auto queryMapResult = updateResultForQuery(tupleOfArgs, std::move(data),
                                             changed, queryName);
  return changed;
}

bool Context::queryCanUseSavedResult(QueryMapResultBase* resultEntry) {
  // If there was no result, we can't reuse it
  if (resultEntry == nullptr) {
    return false;
  }

  // If we already checked this query in this revision,
  // we can use this result
  if (resultEntry->lastComputed == this->currentRevisionNumber ||
      resultEntry->lastCheckedAndReused == this->currentRevisionNumber) {
    return true;
  }

  // Otherwise, check the dependencies. Have any of them
  // changed since the last revision in which we computed this?
  if (resultEntry->dependencies.size() > 0) {
    for (QueryMapResultBase* dependency : resultEntry->dependencies) {
      if (dependency->lastChanged > resultEntry->lastComputed) {
        return false;
      }
      // check the dependencies, transitively
      if (!queryCanUseSavedResult(dependency)) {
        return false;
      }
    }
  } else {
    // If there are no dependencies, assume it is some external
    // input that is managed by the currentRevisionNumber.
    // So, recompute it if the current revision number has changed.
    if (this->currentRevisionNumber > resultEntry->lastComputed) {
      return false;
    }
  }

  // Otherwise, all of the inputs have not changed
  // since this result was last computed.
  resultEntry->lastCheckedAndReused = this->currentRevisionNumber;
  return true;
}

bool Context::queryCanUseSavedResultAndPushIfNot(UniqueString queryName, const char* queryFunc, QueryMapResultBase* resultEntry) {
  bool ret = this->queryCanUseSavedResult(resultEntry);
  if (ret == false) {
    printf("QUERY COMPUTING %s (...)\n", queryFunc);
    // since the result cannot be used, the query will be evaluated
    // so push something to queryDeps
    queryDeps.push_back(QueryDepsEntry(queryName));
  } else {
    printf("QUERY END       %s (...) REUSING\n", queryFunc);
  }
  return ret;
}

void Context::saveDependenciesAndErrorsInParent(QueryMapResultBase* resultEntry) {
  // Record that the parent query depends upon this one.
  //
  // we haven't pushed the query beginning yet, so the
  // parent query is at queryDeps.back()
  if (queryDeps.size() > 0) {
    queryDeps.back().dependencies.push_back(resultEntry);
    if (resultEntry->errors.size() > 0) {
      for (const ErrorMessage& e : resultEntry->errors) {
        queryDeps.back().errors.push_back(e);
      }
    }
  }
}
void Context::endQueryHandleDependency(QueryMapResultBase* result) {
  // queryDeps.back() is the dependency vector for this query
  // which is now ending. So, replace result->dependencies with it.
  result->dependencies.swap(queryDeps.back().dependencies);
  result->errors.swap(queryDeps.back().errors);
  queryDeps.pop_back();
  // additionally, we've run a query and there might well be
  // a parent query. In that event, we should update the dependency
  // vector for the parent query.
  saveDependenciesAndErrorsInParent(result);
}

void Context::queryNoteError(ErrorMessage error) {
  assert(queryDeps.size() > 0);
  queryDeps.back().errors.push_back(std::move(error));
}


namespace querydetail {


template<>
std::size_t queryArgsHash<>(const std::tuple<>& tuple) {
  return 0;
}

template<>
bool queryArgsEquals<>(const std::tuple<>& lhs,
                       const std::tuple<>& rhs) {
  return true;
}

template<>
void queryArgsPrint<>(const std::tuple<>& tuple) {
}

void queryArgsPrintSep() {
  printf(", ");
}

void queryArgsPrintUnknown() {
  printf("?");
}

void queryArgsPrintOne(const ID& v) {
  printf("ID(%s@%i)", v.symbolPath().c_str(), v.postOrderId());
}
void queryArgsPrintOne(const UniqueString& v) {
  printf("\"%s\"", v.c_str());
}

QueryMapResultBase::~QueryMapResultBase() {
}

QueryMapBase::~QueryMapBase() {
}


} // end namespace querydetail
} // end namespace chpl
