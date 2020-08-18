// StructFieldUser.cc
// Oct. 25, 2016
// (c) Copyright 2016 LANSLLC, all rights reserved

/* Find which functions use which fields. */

#include "dump_things.h"
#include "make_replacement.h"
#include "struct_field_user.h"
#include "summarize_command_line.h"
#include "utilities.h"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>

using namespace clang::tooling;
using namespace llvm;

const char * addl_help =
    "For specified structs, find which functions read/write which fields";

static llvm::cl::OptionCategory SFUOpts("Common options for struct-field-use");

static llvm::cl::opt<bool> verbose_compiler(
    "vc",
    llvm::cl::desc("pass -v to compiler instance (default false)"),
    llvm::cl::cat(SFUOpts),
    llvm::cl::init(false));

static cl::opt<std::string> target_struct_string(
    "ts",
    cl::desc("target struct(s), separated by commas if nec. E.g. "
             "-ts=\"cell_t,bas_t,region_t\""),
    cl::value_desc("target-struct-string"),
    cl::cat(SFUOpts));

static cl::opt<bool> export_opts("xp",
                                 cl::desc("export command line options"),
                                 cl::value_desc("bool"),
                                 cl::cat(SFUOpts),
                                 cl::init(false));

/** Print out in various possibly useful ways. */
template <typename MapOMapOSet>
void
print_fields(MapOMapOSet const & m);

void
add_include_paths(ClangTool & tool);

int
main(int argc, const char ** argv)
{
  using namespace corct;
  CommonOptionsParser opt_prs(argc, argv, SFUOpts, addl_help);
  if(export_opts) {
    summarize_command_line("struct-field-use", addl_help);
    return 0;
  }
  RefactoringTool Tool(opt_prs.getCompilations(), opt_prs.getSourcePathList());
  add_include_paths(Tool);
  vec_str targ_fns(split(target_struct_string, ','));
  struct_field_user s_finder(targ_fns);
  struct_field_user::matchers_t field_matchers = s_finder.matchers();
  finder_t finder;
  for(auto m : field_matchers) { finder.addMatcher(m, &s_finder); }
  Tool.run(newFrontendActionFactory(&finder).get());
  std::cout << "Fields written:\n";
  print_fields(s_finder.lhs_uses_);
  std::cout << "Fields accessed, but not written:\n";
  print_fields(s_finder.non_lhs_uses_);
  return 0;
}  // main

void
add_include_paths(ClangTool & tool)
{
  // add header search paths to compiler
  ArgumentsAdjuster ardj1 =
      getInsertArgumentAdjuster(corct::clang_inc_dir1.c_str());
  ArgumentsAdjuster ardj2 =
      getInsertArgumentAdjuster(corct::clang_inc_dir2.c_str());
  tool.appendArgumentsAdjuster(ardj1);
  tool.appendArgumentsAdjuster(ardj2);
  if(verbose_compiler) {
    ArgumentsAdjuster ardj3 = getInsertArgumentAdjuster("-v");
    tool.appendArgumentsAdjuster(ardj3);
  }
  return;
}  // add_include_paths

template <typename MapOMapOSet>
void
print_fields(MapOMapOSet const & m)
{
  using corct::string_t;
  corct::string_t tabs("");
  for(auto map_it : m) {
    string_t const s_name = (map_it.first);
    for(auto mm_it : (map_it.second)) {
      string_t const f_name = mm_it.first;
      // tabs = corct::add_tab(tabs);
      for(auto membr : mm_it.second) {
        std::cout << tabs << f_name << " " << s_name << " " << membr << "\n";
      }
      // tabs = corct::remove_tab(tabs);
    }
  }
  return;
}  // print_fields

// End of file
