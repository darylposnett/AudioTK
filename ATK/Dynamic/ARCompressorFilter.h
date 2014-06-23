/**
 * \file ARCompressorFilter.h
 */

#ifndef ATK_DYNAMIC_ARCOMPRESSORFILTER_H
#define ATK_DYNAMIC_ARCOMPRESSORFILTER_H

#include <ATK/Core/TypedBaseFilter.h>
#include "config.h"

namespace ATK
{
  /**
   * Attack-Release Compressor. Has twice as many inputs channels as it has output channels
   * Even channels are signal, odd channels are gains
   */
  template<typename DataType_>
  class ATK_DYNAMIC_EXPORT ARCompressorFilter : public TypedBaseFilter<DataType_>
  {
  protected:
    typedef TypedBaseFilter<DataType_> Parent;
    using typename Parent::DataType;
    using Parent::converted_inputs_size;
    using Parent::outputs_size;
    using Parent::converted_inputs;
    using Parent::outputs;
    using Parent::nb_input_ports;
    using Parent::nb_output_ports;

  public:
    ARCompressorFilter(int nb_channels = 1);
    ~ARCompressorFilter();
    
  protected:
    virtual void process_impl(std::int64_t size);
    
  };
}

#endif
