/*
 *  Copyright (c) 2014, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <osquery/filesystem.h>

#include <boost/filesystem.hpp>

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "osquery/core/conversions.h"
#include "osquery/core/test_util.h"

using namespace osquery::core;
namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

char* argv0;

namespace osquery {

class PlistTests : public testing::Test {};

TEST_F(PlistTests, test_parse_plist_content) {
  // Isolate plist parsing errors to the plist parser, instead of file reader.
  std::string content;
  readFile(kTestDataPath + "test.plist", content);

  pt::ptree tree;
  auto s = parsePlistContent(content, tree);

  EXPECT_TRUE(s.ok());
  EXPECT_EQ(s.toString(), "OK");

  // Check the specifics of the PLIST.
  EXPECT_EQ(tree.get<bool>("Disabled"), true);
  EXPECT_THROW(tree.get<bool>("foobar"), pt::ptree_bad_path);
  EXPECT_EQ(tree.get<std::string>("Label"), "com.apple.FileSyncAgent.sshd");

  std::vector<std::string> program_arguments = {
      "/System/Library/CoreServices/FileSyncAgent.app/Contents/Resources/"
      "FileSyncAgent_sshd-keygen-wrapper",
      "-i",
      "-f",
      "/System/Library/CoreServices/FileSyncAgent.app/Contents/Resources/"
      "FileSyncAgent_sshd_config",
  };
  pt::ptree program_arguments_tree = tree.get_child("ProgramArguments");
  std::vector<std::string> program_arguments_parsed;
  for (const auto& argument : program_arguments_tree) {
    program_arguments_parsed.push_back(argument.second.get<std::string>(""));
  }
  EXPECT_EQ(program_arguments_parsed, program_arguments);
}

TEST_F(PlistTests, test_parse_plist_from_file) {
  // Now read the plist from a file and parse.
  boost::property_tree::ptree tree;
  auto s = parsePlist(kTestDataPath + "test.plist", tree);

  EXPECT_TRUE(s.ok());
  EXPECT_EQ(s.toString(), "OK");
}

TEST_F(PlistTests, test_parse_plist_array) {
  // Now read the plist from a file and parse.
  boost::property_tree::ptree tree;
  auto s = parsePlist(kTestDataPath + "test_array.plist", tree);

  EXPECT_TRUE(s.ok());
  EXPECT_EQ(s.toString(), "OK");
}

TEST_F(PlistTests, test_parse_plist_content_with_blobs) {
  pt::ptree tree;
  fs::path bin_path(argv0);

  auto s = parsePlist((bin_path.parent_path() /
                       "../../../../tools/tests/test_binary.plist").string(),
                      tree);
  EXPECT_TRUE(s.ok());
  EXPECT_EQ(s.toString(), "OK");
  EXPECT_THROW(tree.get<bool>("foobar"), pt::ptree_bad_path);
  EXPECT_EQ(tree.get<std::string>("SessionItems.Controller"),
            "CustomListItems");
  auto first_element =
      tree.get_child("SessionItems.CustomListItems").begin()->second;
  EXPECT_EQ(first_element.get<std::string>("Name"), "Flux");
  std::string alias = base64Decode(first_element.get<std::string>("Alias"));

  // Verify we parsed the binary blob correctly
  EXPECT_NE(alias.find("Applications/Flux.app"), std::string::npos);
}
}

int main(int argc, char* argv[]) {
  argv0 = argv[0];
  testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
