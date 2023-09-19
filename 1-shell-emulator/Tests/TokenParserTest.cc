#include <fstream>

#include "TokenParser.h"
#include "gtest/gtest.h"

static const char* DataPath = "../../1-shell-emulator/Tests/Data/TokenParser/";

struct TestResult {
  enum class TestError { Success, Failure, FailedToOpen, FailedToRead } Error;
  ShellStatus Result;
};

class TokenParserTestEnvironment {
  std::string TestData;
  TokenParser Tp;
  const char* Delimeters;

  bool HasRun = false;

 public:
  explicit TokenParserTestEnvironment(const char* delim = " ")
      : Delimeters{delim} {}

  TestResult::TestError LoadString(const std::string& data) {
    TestData = data;
    return TestResult::TestError::Success;
  }

  TestResult Run() {
    ShellStatus status;

    status = TokenParserInit(&Tp);
    if (status != SH_SUCCESS) return {TestResult::TestError::Failure, status};

    status = ParseTokens(&Tp, &TestData[0], Delimeters);
    if (status != SH_SUCCESS) return {TestResult::TestError::Failure, status};

    HasRun = true;
    return {TestResult::TestError::Success, SH_SUCCESS};
  }

  const TokenParser& Get() const {
    assert(HasRun);
    return Tp;
  }

  size_t GetSize() const {
    assert(HasRun);
    int i = 0;
    for (; Tp.Tokens[i] != NULL; ++i)
      ;

    return i + 1;
  }

  std::vector<std::string> ToVec() const {
    assert(HasRun);

    std::vector<std::string> tokens;
    for (int i = 0; Tp.Tokens[i] != NULL; ++i) tokens.push_back(Tp.Tokens[i]);

    return tokens;
  }

  ~TokenParserTestEnvironment() {
    if (HasRun) {
      TokenParserDestroy(&Tp);
    }
  }
};

using TPTE = TokenParserTestEnvironment;

void RunStringTest(const std::string& input,
                   const std::vector<std::string>& expected) {
  auto tpte = TPTE{};

  tpte.LoadString(input);

  TestResult res = tpte.Run();

  ASSERT_EQ(res.Error, TestResult::TestError::Success);
  ASSERT_EQ(res.Result, SH_SUCCESS);

  EXPECT_EQ(tpte.GetSize(), expected.size() + 1);
  ASSERT_EQ(tpte.ToVec(), expected);
}

void RunFileTest(const std::string& inputName,
                 const std::string& expectedName) {
  std::string inputData;
  std::vector<std::string> expectedData;

  std::ifstream inputStream{DataPath + inputName};
  ASSERT_TRUE(inputStream.good());

  std::getline(inputStream, inputData);

  std::ifstream expectedStream{DataPath + expectedName};

  std::string buf;
  while (std::getline(expectedStream, buf, ' ')) expectedData.push_back(buf);

  RunStringTest(inputData, expectedData);
}

TEST(TokenParser, EmptyString) {
  auto tpte = TPTE{};

  tpte.LoadString("");
  tpte.Run();

  ASSERT_EQ(tpte.GetSize(), 1);
}

TEST(TokenParser, EmptyStringWithSpaces) {
  auto tpte = TPTE{};

  tpte.LoadString("                 ");
  tpte.Run();

  ASSERT_EQ(tpte.GetSize(), 1);
}

TEST(TokenParser, EasyString) {
  std::string input = "lorem ipsum sir dolor amet";
  std::vector<std::string> expected = {"lorem", "ipsum", "sir", "dolor",
                                       "amet"};

  RunStringTest(input, expected);
}

TEST(TokenParser, EasyStringWithSpaces) {
  std::string input = "    lorem      ipsum       sir dolor  amet ";
  std::vector<std::string> expected = {"lorem", "ipsum", "sir", "dolor",
                                       "amet"};

  RunStringTest(input, expected);
}

TEST(TokenParser, EasyFile) {
  std::string inputName = "Easy.input";
  std::string expectedName = "Easy.expect";

  RunFileTest(inputName, expectedName);
}

TEST(TokenParser, EasyFileWithSpaces) {
  std::string inputName = "EasyWithSpaces.input";
  std::string expectedName = "Easy.expect";

  RunFileTest(inputName, expectedName);
}

TEST(TokenParser, 4KBOfSpaces) {
  std::string inputName = "4KBOfSpaces.input";
  std::string expectedName = "4KBOfSpaces.expect";

  RunFileTest(inputName, expectedName);
}

TEST(TokenParser, LargeFile) {
  std::string inputName = "LargeFile.input";
  std::string expectedName = "LargeFile.expect";

  RunFileTest(inputName, expectedName);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}