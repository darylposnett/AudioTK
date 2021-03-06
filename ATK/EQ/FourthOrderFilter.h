/**
 * \file FourthOrderFilter.h
 */

#ifndef ATK_EQ_FOURTHORDERFILTER_H
#define ATK_EQ_FOURTHORDERFILTER_H

#include <ATK/Core/TypedBaseFilter.h>
#include <ATK/EQ/EQInterface.h>

namespace ATK
{
  /// Base class for all IIR second order filters
  template<typename DataType_>
  class FourthOrderBaseCoefficients: public TypedBaseFilter<DataType_>, public SingleCutFrequencyInterface<typename TypeTraits<DataType_>::Scalar>
  {
  public:
    /// Simplify parent calls
    typedef TypedBaseFilter<DataType_> Parent;
    using typename Parent::AlignedScalarVector;
    using typename Parent::DataType;
    typedef typename TypeTraits<DataType>::Scalar CoeffDataType;
    using Parent::setup;
  protected:
    CoeffDataType cut_frequency;

    const static int in_order=4;
    const static int out_order=4;
    AlignedScalarVector coefficients_in;
    AlignedScalarVector coefficients_out;

    void setup() override;
  public:
    /*!
     * @brief Constructor
     * @param nb_channels is the number of input and output channels
     */
    FourthOrderBaseCoefficients(std::size_t nb_channels = 1);
    /// Sets the cut or central frequency of the filter
    void set_cut_frequency(CoeffDataType cut_frequency) final;
    /// Returns the cut or central frequency
    CoeffDataType get_cut_frequency() const final;
  };
}

#endif
