/**
 * \file TypedBaseFilter.hxx
 */

#include <ATK/Core/TypedBaseFilter.h>
#include <ATK/Core/Utilities.h>

#include <complex>
#include <cstdint>
#include <iostream>
#include <type_traits>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#if USE_SIMD
#include <simdpp/simd.h>
#endif

namespace
{
  typedef boost::mpl::vector<std::int16_t, std::int32_t, int64_t, float, double, std::complex<float>, std::complex<double> > ConversionTypes;

  template<typename Vector, typename DataType>
  typename boost::enable_if<typename boost::mpl::empty<Vector>::type, void>::type
    convert_scalar_array(ATK::BaseFilter* filter, unsigned int port, DataType* converted_input, std::size_t size, int type)
  {
    throw std::runtime_error("Cannot convert types for these filters");
  }

  template<typename Vector, typename DataType>
  typename boost::disable_if<typename boost::is_arithmetic<typename boost::mpl::front<Vector>::type>::type, typename boost::disable_if<typename boost::mpl::empty<Vector>::type, void>::type>::type
  convert_scalar_array(ATK::BaseFilter* filter, unsigned int port, DataType* converted_input, std::size_t size, int type)
  {
    convert_scalar_array<typename boost::mpl::pop_front<Vector>::type, DataType>(filter, port, converted_input, size, type - 1);
  }

  template<typename Vector, typename DataType>
  typename boost::enable_if<typename boost::is_arithmetic<typename boost::mpl::front<Vector>::type>::type, typename boost::disable_if<typename boost::mpl::empty<Vector>::type, void>::type>::type
    convert_scalar_array(ATK::BaseFilter* filter, unsigned int port, DataType* converted_input, std::size_t size, int type)
  {
    if(type != 0)
    {
      convert_scalar_array<typename boost::mpl::pop_front<Vector>::type, DataType>(filter, port, converted_input, size, type - 1);
    }
    else
    {
      typedef typename boost::mpl::front<Vector>::type InputOriginalType;
      InputOriginalType* original_input_array = static_cast<ATK::TypedBaseFilter<InputOriginalType>*>(filter)->get_output_array(port);
      ATK::ConversionUtilities<InputOriginalType, DataType>::convert_array(original_input_array, converted_input, size);
    }
  }

  template<typename Vector, typename DataType>
  typename boost::enable_if<typename boost::mpl::empty<Vector>::type, void>::type
    convert_complex_array(ATK::BaseFilter* filter, unsigned int port, DataType* converted_input, std::size_t size, int type)
  {
    throw std::runtime_error("Can't convert types");
  }

  template<typename Vector, typename DataType>
  typename boost::disable_if<typename boost::mpl::empty<Vector>::type, void>::type
    convert_complex_array(ATK::BaseFilter* filter, unsigned int port, DataType* converted_input, std::size_t size, int type)
  {
    assert(type >= 0);
    if (type != 0)
    {
      convert_complex_array<typename boost::mpl::pop_front<Vector>::type, DataType>(filter, port, converted_input, size, type - 1);
    }
    else
    {
      typedef typename boost::mpl::front<Vector>::type InputOriginalType;
      InputOriginalType* original_input_array = static_cast<ATK::TypedBaseFilter<InputOriginalType>*>(filter)->get_output_array(port);
      ATK::ConversionUtilities<InputOriginalType, DataType>::convert_array(original_input_array, converted_input, size);
    }
  }

  /// Conversion function for arithmetic types
  template<typename Vector, typename DataType>
  typename boost::enable_if<typename boost::is_arithmetic<DataType>::type, void>::type convert_array(ATK::BaseFilter* filter, unsigned int port, DataType* converted_input, std::size_t size, int type)
  {
    convert_scalar_array<Vector, DataType>(filter, port, converted_input, size, type);
  }

  /// Conversion function for other types not contained in ConversionTypes (no conversion in that case, just copy)
  template<typename Vector, typename DataType>
  typename boost::disable_if<typename boost::is_arithmetic<DataType>::type, typename boost::disable_if<typename boost::mpl::contains<Vector, DataType>::type, void>::type>::type convert_array(ATK::BaseFilter* filter, unsigned int port, DataType* converted_input, std::size_t size, int type)
  {
    assert(dynamic_cast<ATK::OutputArrayInterface<DataType>*>(filter));
     // For SIMD, you shouldn't call this, but adapt input/output delays so that there is no copy from one filter to another.
    DataType* original_input_array = dynamic_cast<ATK::TypedBaseFilter<DataType>*>(filter)->get_output_array(port);
    ATK::ConversionUtilities<DataType, DataType>::convert_array(original_input_array, converted_input, size);
  }

  /// Conversion function for std::complex contained in ConversionTypes
  template<typename Vector, typename DataType>
  typename boost::disable_if<typename boost::is_arithmetic<DataType>::type, typename boost::enable_if<typename boost::mpl::contains<Vector, DataType>::type, void>::type>::type convert_array(ATK::BaseFilter* filter, unsigned int port, DataType* converted_input, std::size_t size, int type)
  {
    convert_complex_array<Vector, DataType>(filter, port, converted_input, size, type);
  }

  template<typename Vector, typename DataType>
  typename boost::enable_if<typename boost::mpl::contains<Vector, DataType>::type, int>::type get_type()
  {
    return boost::mpl::distance<boost::mpl::begin<ConversionTypes>::type, typename boost::mpl::find<ConversionTypes, DataType>::type >::value;
  }

  template<typename Vector, typename DataType>
  typename boost::disable_if<typename boost::mpl::contains<Vector, DataType>::type, int>::type get_type()
  {
    return -1;
  }
}

namespace ATK
{
  template<typename DataType>
  OutputArrayInterface<DataType>::~OutputArrayInterface()
  {
  }

  template<typename DataType_, typename DataType__>
  TypedBaseFilter<DataType_, DataType__>::TypedBaseFilter(std::size_t nb_input_ports, std::size_t nb_output_ports)
  :Parent(nb_input_ports, nb_output_ports), converted_inputs_delay(nb_input_ports), converted_inputs(nb_input_ports, nullptr), converted_inputs_size(nb_input_ports, 0), direct_filters(nb_input_ports, nullptr), outputs_delay(nb_output_ports), outputs(nb_output_ports, nullptr), outputs_size(nb_output_ports, 0), default_input(nb_input_ports, TypeTraits<DataType_>::Zero()), default_output(nb_output_ports, TypeTraits<DataType__>::Zero())
  {
  }

  template<typename DataType_, typename DataType__>
  TypedBaseFilter<DataType_, DataType__>::TypedBaseFilter(TypedBaseFilter&& other)
  : Parent(std::move(other)), converted_inputs_delay(std::move(other.converted_inputs_delay)), converted_inputs(std::move(other.converted_inputs)), converted_inputs_size(std::move(other.converted_inputs_size)), direct_filters(std::move(other.direct_filters)), outputs_delay(std::move(other.outputs_delay)), outputs(std::move(other.outputs)), outputs_size(std::move(other.outputs_size)), default_input(std::move(other.default_input)), default_output(std::move(other.default_output))
  {
  }

  template<typename DataType_, typename DataType__>
  TypedBaseFilter<DataType_, DataType__>::~TypedBaseFilter()
  {
  }

  template<typename DataType_, typename DataType__>
  void TypedBaseFilter<DataType_, DataType__>::set_nb_input_ports(std::size_t nb_ports)
  {
    if(nb_ports == nb_input_ports)
      return;
    Parent::set_nb_input_ports(nb_ports);
    converted_inputs_delay = std::vector<AlignedVector>(nb_ports);
    converted_inputs.assign(nb_ports, nullptr);
    converted_inputs_size.assign(nb_ports, 0);
    direct_filters.assign(nb_ports, nullptr);
    default_input.assign(nb_ports, TypeTraits<DataTypeInput>::Zero());
  }

  template<typename DataType_, typename DataType__>
  void TypedBaseFilter<DataType_, DataType__>::set_nb_output_ports(std::size_t nb_ports)
  {
    if(nb_ports == nb_output_ports)
      return;
    Parent::set_nb_output_ports(nb_ports);
    outputs_delay = std::vector<AlignedOutVector>(nb_ports);
    outputs.assign(nb_ports, nullptr);
    outputs_size.assign(nb_ports, 0);
    default_output.assign(nb_ports, TypeTraits<DataTypeOutput>::Zero());
  }

  template<typename DataType_, typename DataType__>
  void TypedBaseFilter<DataType_, DataType__>::process_impl(std::size_t size) const
  {
  }

  template<typename DataType_, typename DataType__>
  void TypedBaseFilter<DataType_, DataType__>::prepare_process(std::size_t size)
  {
    convert_inputs(size);
  }

  template<typename DataType_, typename DataType__>
  int TypedBaseFilter<DataType_, DataType__>::get_type() const
  {
    return ::get_type<ConversionTypes, DataType__>();
  }

  template<typename DataType_, typename DataType__>
  DataType__* TypedBaseFilter<DataType_, DataType__>::get_output_array(std::size_t port) const
  {
    return outputs[port];
  }

  template<typename DataType_, typename DataType__>
  std::size_t TypedBaseFilter<DataType_, DataType__>::get_output_array_size() const
  {
    return outputs_size.front();
  }

  template<typename DataType_, typename DataType__>
  void TypedBaseFilter<DataType_, DataType__>::convert_inputs(std::size_t size)
  {
    for(unsigned int i = 0; i < nb_input_ports; ++i)
    {
      // if the input delay is smaller than the preceding filter output delay, we may have overlap
      // if the types are identical and if the type is not -1 (an unknown type)
      // if we have overlap, don't copy anything at all
      if((input_delay <= connections[i].second->get_output_delay()) && (direct_filters[i] != nullptr))
      {
        converted_inputs[i] = direct_filters[i]->get_output_array(connections[i].first);
        converted_inputs_size[i] = size;
        continue;
      }
      auto input_size = converted_inputs_size[i];
      if(input_size < size)
      {
        AlignedVector temp(input_delay + size);
        if(input_size == 0)
        {
          for(unsigned int j = 0; j < input_delay; ++j)
          {
            temp[j] = default_input[i];
          }
        }
        else
        {
          const auto input_ptr = converted_inputs[i];
          for(int j = 0; j < static_cast<int>(input_delay); ++j)
          {
            temp[j] = input_ptr[last_size + j - input_delay];
          }
        }

        converted_inputs_delay[i] = std::move(temp);
        converted_inputs[i] = converted_inputs_delay[i].data() + input_delay;
        converted_inputs_size[i] = size;
      }
      else
      {
        auto my_last_size = static_cast<int64_t>(last_size) * input_sampling_rate / output_sampling_rate;
        const auto input_ptr = converted_inputs[i];
        for(int j = 0; j < static_cast<int>(input_delay); ++j)
        {
          input_ptr[j - input_delay] = input_ptr[my_last_size + j - input_delay];
        }
      }
      convert_array<ConversionTypes, DataTypeInput>(connections[i].second, connections[i].first, converted_inputs[i], size, connections[i].second->get_type());
    }
  }

  template<typename DataType_, typename DataType__>
  void TypedBaseFilter<DataType_, DataType__>::prepare_outputs(std::size_t size)
  {
    for(unsigned int i = 0; i < nb_output_ports; ++i)
    {
      auto output_size = outputs_size[i];
      if(output_size < size)
      {
        AlignedOutVector temp(output_delay + size);
        if(output_size == 0)
        {
          for(unsigned int j = 0; j < output_delay; ++j)
          {
            temp[j] = default_output[i];
          }
        }
        else
        {
          const auto output_ptr = outputs[i];
          for(int j = 0; j < static_cast<int>(output_delay); ++j)
          {
            temp[j] = output_ptr[last_size + j - output_delay];
          }
        }

        outputs_delay[i] = std::move(temp);
        outputs[i] = outputs_delay[i].data() + output_delay;
        outputs_size[i] = size;
      }
      else
      {
        const auto output_ptr = outputs[i];
        for(int j = 0; j < static_cast<int>(output_delay); ++j)
        {
          output_ptr[j - output_delay] = output_ptr[last_size + j - output_delay];
        }
      }
    }
  }

  template<typename DataType_, typename DataType__>
  void TypedBaseFilter<DataType_, DataType__>::full_setup()
  {
    // Reset input arrays
    converted_inputs_delay = std::vector<AlignedVector>(nb_input_ports);
    converted_inputs.assign(nb_input_ports, nullptr);
    converted_inputs_size.assign(nb_input_ports, 0);

    // Reset output arrays
    outputs_delay = std::vector<AlignedOutVector>(nb_output_ports);
    outputs.assign(nb_output_ports, nullptr);
    outputs_size.assign(nb_output_ports, 0);

    Parent::full_setup();
  }

  template<typename DataType_, typename DataType__>
  void TypedBaseFilter<DataType_, DataType__>::set_input_port(std::size_t input_port, BaseFilter* filter, std::size_t output_port)
  {
    Parent::set_input_port(input_port, filter, output_port);
    converted_inputs_size[input_port] = 0;
    direct_filters[input_port] = dynamic_cast<OutputArrayInterface<DataType_>*>(filter);
  }
}
