#include <unistd.h>

#include <fstream>

#include "Executor.h"
#include "TokenParser.h"
#include "gtest/gtest.h"

static const char* DataPath = "../../1-shell-emulator/Tests/Data/Executor/";

class ExecutorTestEnvironment {
  ExecutorContext Ctx;
  TokenParser Tp;
  bool IsInit = false;

  FILE* InputFile = NULL;
  FILE* OutputFile = NULL;

  char* Buf;
  size_t Length;

 public:
  ExecutorTestEnvironment() = default;

  bool SetInputFile(const std::string& inputName) {
    FILE* f = fopen(inputName.c_str(), "r");
    if (!f) {
      perror("Failed to open input file");
      return false;
    }

    InputFile = f;
    return true;
  }

  bool SetOutputFile(const std::string& outputName) {
    FILE* f = fopen(outputName.c_str(), "w");
    if (!f) {
      perror("Failed to open output file");
      return false;
    }

    OutputFile = f;
    return true;
  }

  ShellStatus Init(char** env) {
    assert(InputFile);
    assert(OutputFile);
    assert(TokenParserInit(&Tp) == SH_SUCCESS);

    IsInit = true;
    return ExecutorContextInit(&Ctx, env, fileno(InputFile),
                               fileno(OutputFile));
  }

  ShellStatus ExecuteLine(std::string line) {
    ShellStatus status;

    status = ParseTokens(&Tp, &line[0], " ");
    if (status != SH_SUCCESS) return status;

    return Execute(&Ctx, Tp.Tokens);
  }

  const ExecutorContext& Get() const { return Ctx; }

  void Stop() {
    fclose(InputFile);
    fclose(OutputFile);
    if (IsInit) {
      ExecutorDestroy(&Ctx);
    }
  }
};

using ETE = ExecutorTestEnvironment;

void SimpleInit(ETE& ete) {
  ete.SetInputFile("/dev/null");
  ete.SetOutputFile("/dev/null");
  ete.Init(environ);
}

void RunFullTest(const std::string& commandsName, const std::string& inputName,
                 const std::string& expectedName) {
  static const char* outputName = "tmp";

  std::ifstream commandsStream{DataPath + commandsName};
  ASSERT_TRUE(commandsStream.good());

  std::ifstream expectedStream{DataPath + expectedName};
  ASSERT_TRUE(commandsStream.good());

  ETE ete{};
  ASSERT_TRUE(ete.SetInputFile(DataPath + inputName));
  ASSERT_TRUE(ete.SetOutputFile(outputName));

  std::string line;
  while (std::getline(commandsStream, line)) {
    ShellStatus status = ete.ExecuteLine(line);
    ASSERT_EQ(status, SH_SUCCESS);
  }

  ete.Stop();

  std::ifstream outputStream{outputName};
  assert(outputStream.good());

  std::stringstream output;
  output << outputStream.rdbuf();

  std::stringstream expected;
  expected << expectedStream.rdbuf();

  ASSERT_EQ(output.str(), expected.str());
}

TEST(Executor, Exit) {
  ETE ete{};
  SimpleInit(ete);

  ASSERT_TRUE(ete.Get().Active);

  ShellStatus status = ete.ExecuteLine("exit");
  EXPECT_EQ(status, SH_SUCCESS);

  ASSERT_FALSE(ete.Get().Active);

  ete.Stop();
}

TEST(Executor, Empty) {
  ETE ete{};
  SimpleInit(ete);

  EXPECT_EQ(ete.ExecuteLine("    "), SH_SUCCESS);

  ete.Stop();
}

TEST(Executor, ToFile) {
  std::string content = "ABC";
  std::string file = "tmp";

  ETE ete{};
  SimpleInit(ete);

  EXPECT_EQ(ete.ExecuteLine("echo " + content + " > " + file), SH_SUCCESS);

  std::ifstream outputStream{file};
  assert(outputStream.good());

  std::stringstream output;
  output << outputStream.rdbuf();

  ASSERT_EQ(content + "\n", output.str());

  ete.Stop();
}

TEST(Executor, FromFile) {
  std::string content = "Hello DEF";
  std::string fileIn = "Text.input";
  std::string fileOut = "tmp";

  ETE ete{};
  SimpleInit(ete);

  ShellStatus status =
      ete.ExecuteLine("grep DEF < " + std::string(DataPath) + fileIn + " > tmp");
  EXPECT_EQ(status, SH_SUCCESS);

  std::ifstream outputStream{fileOut};
  assert(outputStream.good());

  std::stringstream output;
  output << outputStream.rdbuf();

  ASSERT_EQ(content + "\n", output.str());

  ete.Stop();
}

TEST(Executor, InputOverride) {
  ETE ete{};
  SimpleInit(ete);

  ShellStatus status = ete.ExecuteLine("ls | ls < tmp");
  ASSERT_EQ(status, SH_INPUT_OVERRIDE);

  ete.Stop();
}

TEST(Executor, OutputOverride) {
  ETE ete{};
  SimpleInit(ete);

  ShellStatus status = ete.ExecuteLine("ls > tmp | grep abc");
  ASSERT_EQ(status, SH_OUTPUT_OVERRIDE);

  ete.Stop();
}

TEST(Executor, FileNotFound) {
  ETE ete{};
  SimpleInit(ete);

  ShellStatus status =
      ete.ExecuteLine("ls < SomeFileThatDoesNotExist | grep abd");
  ASSERT_EQ(status, SH_ERRNO_ERROR);
  ASSERT_EQ(errno, ENOENT);

  ete.Stop();
}

TEST(Executor, SyntaxError) {
  ETE ete{};
  SimpleInit(ete);

  ShellStatus status =
      ete.ExecuteLine("ls | | |");
  ASSERT_EQ(status, SH_SYNTAX_ERROR);

  ete.Stop();
}