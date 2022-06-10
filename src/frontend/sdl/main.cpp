#include <Frontend.hpp>
#include <util.hpp>
#include <argparse/argparse.hpp>

int main(int argc, char* argv[]) {
  argparse::ArgumentParser program("natsukashii");
  program.add_argument("-c", "--core")
    .required()
    .help("Select the core");

  program.add_argument("rom")
    .required()
    .help("ROM to load");

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  natsukashii::frontend::App app(program.get<const std::string&>("rom"), program.get<const std::string&>("--core"));
  app.Run();

  return 0;
}
