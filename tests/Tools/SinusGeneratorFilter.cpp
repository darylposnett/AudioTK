/**
 * \ file SinusGeneratorFilter.cpp
 */

#include <ATK/config.h>

#include <ATK/Mock/SimpleSinusGeneratorFilter.h>
#include <ATK/Mock/TriangleCheckerFilter.h>

#include <ATK/Tools/SumFilter.h>
#include <ATK/Tools/SinusGeneratorFilter.h>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

#define PROCESSSIZE (1024)

BOOST_AUTO_TEST_CASE( SinusGeneratorFilter_1k_test )
{
  ATK::SimpleSinusGeneratorFilter<float> generator;
  generator.set_output_sampling_rate(48000);
  generator.set_amplitude(-1);
  generator.set_frequency(1000);
  
  ATK::SinusGeneratorFilter<float> filter;
  filter.set_output_sampling_rate(48000);
  filter.set_frequency(1000);

  ATK::SumFilter<float> sumfilter;
  sumfilter.set_input_sampling_rate(48000);
  
  ATK::TriangleCheckerFilter<float> checker;
  checker.set_input_sampling_rate(48000);
  checker.set_amplitude(0);
  checker.set_frequency(1000);
  
  sumfilter.set_input_port(0, &generator, 0);
  sumfilter.set_input_port(1, &filter, 1);
  
  checker.set_input_port(0, &sumfilter, 0);
  
  checker.process(PROCESSSIZE);
}
