#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>

using matrix = std::vector<std::vector<std::uint64_t>>;
using result_t = std::pair<std::chrono::duration<double>, matrix>;

// helper function for inner most loop
int sub_mul(const matrix &m1, const matrix &m2, uint64_t row, uint64_t col,
            uint64_t depth) {
  std::uint64_t partial_result = 0;
  hpx::for_loop(0, depth,
                [&](auto &i) { partial_result += m1[row][i] * m2[i][col]; });
  return partial_result;
}


// sequential multiplication for calculating speed
result_t sequential(const matrix &m1, const matrix &m2, const uint64_t n) {
  matrix result(n, std::vector<uint64_t>(n));

  auto t1 = std::chrono::high_resolution_clock::now();
  for (uint64_t i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      result[i][j] = sub_mul(m1, m2, i, j, n);
    }
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = t2 - t1;

  return std::make_pair(elapsed, result);
}

// using parallel for_loop
result_t parallel(const matrix &m1, const matrix &m2, const uint64_t n) {
  matrix result(n, std::vector<uint64_t>(n));

  auto t1 = std::chrono::high_resolution_clock::now();

  for (uint64_t i = 0; i < n; i++) {
    hpx::for_loop(hpx::execution::par, 0, n,
                  [&](auto j) { result[i][j] = sub_mul(m1, m2, i, j, n); });
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = t2 - t1;

  return std::make_pair(elapsed, result);
}



int hpx_main(hpx::program_options::variables_map &vm) {
  std::uint64_t n = vm["n"].as<std::uint64_t>();

  matrix m1(n, std::vector<uint64_t>(n));
  matrix m2(n, std::vector<uint64_t>(n));

  for (std::uint64_t i = 0; i < n; i++) {
    for (std::uint64_t j = 0; j < n; j++) {
      m1[i][j] = rand() % 50;
      m2[i][j] = rand() % 50;
    }
  }

  auto seq_res = sequential(m1, m2, n);
  auto par_res = parallel(m1, m2, n);

  if (seq_res.second != par_res.second)
    std::cout << "Failed";
  else {
    std::cout << "Time Elapsed sequential : " << seq_res.first.count() << "s\n";
    std::cout << "Time Elapsed parallel : " << par_res.first.count() << "s\n";

  }
  return hpx::finalize();
}

int main(int argc, char *argv[]) {
  using namespace hpx::program_options;

  options_description desc_commandline;
  desc_commandline.add_options()(
      "n", value<std::uint64_t>()->default_value(128), "Matrix dimension");

  hpx::init_params init_args;
  init_args.desc_cmdline = desc_commandline;

  return hpx::init(argc, argv, init_args);
}


